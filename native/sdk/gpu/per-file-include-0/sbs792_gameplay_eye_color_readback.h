#pragma once

#include <rex/graphics/d3d12/deferred_command_list.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <new>
#include <utility>
#include <vector>
#include <wrl/client.h>

namespace venom::sbs {

// Copies tightly packed RGBA8 rows from a padded source footprint. Bounds and
// arithmetic are checked before any pointer arithmetic or row copy occurs.
inline bool CopyPackedRows792(const std::uint8_t* source, std::size_t source_size,
                              std::size_t source_offset, std::size_t source_row_pitch,
                              std::uint32_t width, std::uint32_t height,
                              std::uint8_t* destination,
                              std::size_t destination_size) noexcept {
  if (!source || !destination || width == 0 || height == 0) return false;
  constexpr std::size_t kBytesPerPixel = 4;
  if (static_cast<std::size_t>(width) >
      (std::numeric_limits<std::size_t>::max)() / kBytesPerPixel)
    return false;
  const std::size_t row_bytes = static_cast<std::size_t>(width) * kBytesPerPixel;
  if (source_row_pitch < row_bytes || source_offset > source_size) return false;
  const std::size_t rows_before_last = static_cast<std::size_t>(height - 1);
  const std::size_t max_size = (std::numeric_limits<std::size_t>::max)();
  if (rows_before_last != 0 &&
      source_row_pitch > (max_size - source_offset) / rows_before_last)
    return false;
  const std::size_t last_row = source_offset + rows_before_last * source_row_pitch;
  if (last_row > source_size || row_bytes > source_size - last_row) return false;
  if (static_cast<std::size_t>(height) > max_size / row_bytes) return false;
  const std::size_t packed_size = row_bytes * static_cast<std::size_t>(height);
  if (packed_size > destination_size) return false;

  for (std::size_t row = 0; row < static_cast<std::size_t>(height); ++row) {
    std::memcpy(destination + row * row_bytes,
                source + source_offset + row * source_row_pitch, row_bytes);
  }
  return true;
}

inline bool CalculateReadExtent792(std::uint64_t offset, std::uint64_t row_pitch,
                                   std::uint32_t width, std::uint32_t height,
                                   std::uint64_t buffer_size, std::size_t* out_offset,
                                   std::size_t* out_end) noexcept {
  if (!out_offset || !out_end || width == 0 || height == 0) return false;
  const std::uint64_t row_bytes = static_cast<std::uint64_t>(width) * UINT64_C(4);
  if (row_pitch < row_bytes) return false;
  if (offset > buffer_size) return false;
  const std::uint64_t rows_before_last = static_cast<std::uint64_t>(height - 1);
  const std::uint64_t max_u64 = (std::numeric_limits<std::uint64_t>::max)();
  if (rows_before_last != 0 && row_pitch > (max_u64 - offset) / rows_before_last)
    return false;
  const std::uint64_t last_row = offset + rows_before_last * row_pitch;
  if (row_bytes > max_u64 - last_row) return false;
  const std::uint64_t end = last_row + row_bytes;
  if (end > buffer_size || offset > (std::numeric_limits<std::size_t>::max)() ||
      end > (std::numeric_limits<std::size_t>::max)())
    return false;
  *out_offset = static_cast<std::size_t>(offset);
  *out_end = static_cast<std::size_t>(end);
  return true;
}

// Records readback copies for the two eye color textures, resolving matching
// four-sample inputs to retained single-sample textures first. This helper does not
// assign meaning to pixel values or make a rendering/stereo acceptance claim.
// The caller must keep this helper, its source colors, descriptors, and the
// deferred recorder alive and unmodified through GPU fence completion, and
// must not Reset the recorder before the recorded submission completes. Source
// colors are expected to be in COMMON state after DeferredEyePassBoundary::End.
class GameplayEyeColorReadback792 {
 public:
  using ResourcePtr = Microsoft::WRL::ComPtr<ID3D12Resource>;
  using PixelPair = std::array<std::array<std::uint8_t, 4>, 2>;

  struct FramePair {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::array<std::vector<std::uint8_t>, 2> rgba{};
  };

  GameplayEyeColorReadback792() = default;
  GameplayEyeColorReadback792(const GameplayEyeColorReadback792&) = delete;
  GameplayEyeColorReadback792& operator=(const GameplayEyeColorReadback792&) = delete;

  HRESULT Initialize(ID3D12Device* device, ID3D12Resource* left,
                     ID3D12Resource* right) noexcept {
    if (!device || !left || !right || left == right || initialized() || recorded_)
      return E_INVALIDARG;

    const std::array<ID3D12Resource*, 2> input{{left, right}};
    const std::array<D3D12_RESOURCE_DESC, 2> input_descs{{left->GetDesc(), right->GetDesc()}};
    const UINT sample_count = input_descs[0].SampleDesc.Count;
    if ((sample_count != 1 && sample_count != 4) ||
        input_descs[1].SampleDesc.Count != sample_count ||
        input_descs[0].Width != input_descs[1].Width ||
        input_descs[0].Height != input_descs[1].Height ||
        input_descs[0].SampleDesc.Quality != 0 || input_descs[1].SampleDesc.Quality != 0)
      return E_INVALIDARG;
    const bool multisampled = sample_count == 4;

    if (multisampled) {
      D3D12_FEATURE_DATA_FORMAT_SUPPORT format_support{};
      format_support.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      const HRESULT support_hr = device->CheckFeatureSupport(
          D3D12_FEATURE_FORMAT_SUPPORT, &format_support, sizeof(format_support));
      if (FAILED(support_hr)) return support_hr;
      if ((format_support.Support1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RESOLVE) == 0)
        return E_INVALIDARG;
    }

    std::array<ResourcePtr, 2> pending_colors{};
    std::array<ResourcePtr, 2> pending_msaa_colors{};
    std::array<ResourcePtr, 2> pending_readbacks{};
    std::array<D3D12_PLACED_SUBRESOURCE_FOOTPRINT, 2> pending_footprints{};
    std::array<UINT, 2> pending_num_rows{};
    std::array<UINT64, 2> pending_row_sizes{};
    std::array<UINT64, 2> pending_total_bytes{};

    for (std::size_t i = 0; i != input.size(); ++i) {
      const D3D12_RESOURCE_DESC& input_desc = input_descs[i];
      if (input_desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D ||
          input_desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM || input_desc.MipLevels != 1 ||
          input_desc.DepthOrArraySize != 1 || input_desc.SampleDesc.Count != sample_count ||
          input_desc.SampleDesc.Quality != 0 || input_desc.Width == 0 ||
          input_desc.Width > kMaximumDimension || input_desc.Height == 0 ||
          input_desc.Height > kMaximumDimension)
        return E_INVALIDARG;

      D3D12_RESOURCE_DESC readback_source_desc = input_desc;
      if (multisampled) {
        pending_msaa_colors[i] = input[i];
        D3D12_HEAP_PROPERTIES default_heap{};
        default_heap.Type = D3D12_HEAP_TYPE_DEFAULT;
        default_heap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        default_heap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        default_heap.CreationNodeMask = 1;
        default_heap.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC resolve_desc{};
        resolve_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resolve_desc.Alignment = 0;
        resolve_desc.Width = input_desc.Width;
        resolve_desc.Height = input_desc.Height;
        resolve_desc.DepthOrArraySize = 1;
        resolve_desc.MipLevels = 1;
        resolve_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        resolve_desc.SampleDesc.Count = 1;
        resolve_desc.SampleDesc.Quality = 0;
        resolve_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resolve_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        const HRESULT resolve_hr = device->CreateCommittedResource(
            &default_heap, D3D12_HEAP_FLAG_NONE, &resolve_desc, D3D12_RESOURCE_STATE_COMMON,
            nullptr, IID_PPV_ARGS(pending_colors[i].GetAddressOf()));
        if (FAILED(resolve_hr)) return resolve_hr;
        readback_source_desc = resolve_desc;
      } else {
        pending_colors[i] = input[i];
      }

      device->GetCopyableFootprints(&readback_source_desc, 0, 1, 0, &pending_footprints[i],
                                    &pending_num_rows[i], &pending_row_sizes[i],
                                    &pending_total_bytes[i]);
      const auto& footprint = pending_footprints[i];
      const UINT64 expected_row_size = readback_source_desc.Width * UINT64_C(4);
      if (footprint.Offset % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT != 0 ||
          footprint.Footprint.Format != DXGI_FORMAT_R8G8B8A8_UNORM ||
          footprint.Footprint.Width != readback_source_desc.Width ||
          footprint.Footprint.Height != readback_source_desc.Height ||
          footprint.Footprint.Depth != 1 ||
          footprint.Footprint.RowPitch == 0 ||
          footprint.Footprint.RowPitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT != 0 ||
          pending_num_rows[i] != readback_source_desc.Height ||
          pending_row_sizes[i] != expected_row_size ||
          pending_total_bytes[i] == 0 || pending_total_bytes[i] > kMaximumCombinedReadbackBytes ||
          footprint.Offset > pending_total_bytes[i] ||
          pending_total_bytes[i] - footprint.Offset < sizeof(std::uint32_t))
        return E_INVALIDARG;

      D3D12_HEAP_PROPERTIES heap{};
      heap.Type = D3D12_HEAP_TYPE_READBACK;
      heap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      heap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
      heap.CreationNodeMask = 1;
      heap.VisibleNodeMask = 1;
      D3D12_RESOURCE_DESC buffer{};
      buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      buffer.Alignment = 0;
      buffer.Width = pending_total_bytes[i];
      buffer.Height = 1;
      buffer.DepthOrArraySize = 1;
      buffer.MipLevels = 1;
      buffer.Format = DXGI_FORMAT_UNKNOWN;
      buffer.SampleDesc.Count = 1;
      buffer.SampleDesc.Quality = 0;
      buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      buffer.Flags = D3D12_RESOURCE_FLAG_NONE;
      HRESULT hr = device->CreateCommittedResource(
          &heap, D3D12_HEAP_FLAG_NONE, &buffer, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
          IID_PPV_ARGS(pending_readbacks[i].GetAddressOf()));
      if (FAILED(hr)) return hr;
    }

    if (pending_total_bytes[0] > kMaximumCombinedReadbackBytes ||
        pending_total_bytes[1] > kMaximumCombinedReadbackBytes - pending_total_bytes[0])
      return E_INVALIDARG;

    colors_.swap(pending_colors);
    msaa_colors_.swap(pending_msaa_colors);
    readbacks_.swap(pending_readbacks);
    footprints_ = pending_footprints;
    total_bytes_ = pending_total_bytes;
    multisampled_ = multisampled;
    recorded_submission_ = 0;
    recorded_ = false;
    return S_OK;
  }

  HRESULT Record(rex::graphics::d3d12::DeferredCommandList* list,
                 std::uint64_t submission) noexcept {
    if (!initialized() || !list || recorded_ || submission == 0) return E_INVALIDARG;

    if (multisampled_) {
      D3D12_RESOURCE_BARRIER to_resolve[4]{};
      for (std::size_t i = 0; i != colors_.size(); ++i) {
        to_resolve[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        to_resolve[i].Transition.pResource = msaa_colors_[i].Get();
        to_resolve[i].Transition.Subresource = 0;
        to_resolve[i].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        to_resolve[i].Transition.StateAfter = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
        to_resolve[i + colors_.size()].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        to_resolve[i + colors_.size()].Transition.pResource = colors_[i].Get();
        to_resolve[i + colors_.size()].Transition.Subresource = 0;
        to_resolve[i + colors_.size()].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        to_resolve[i + colors_.size()].Transition.StateAfter = D3D12_RESOURCE_STATE_RESOLVE_DEST;
      }
      list->D3DResourceBarrier(4, to_resolve);
      for (std::size_t i = 0; i != colors_.size(); ++i) {
        list->D3DResolveSubresource(colors_[i].Get(), 0, msaa_colors_[i].Get(), 0,
                                   DXGI_FORMAT_R8G8B8A8_UNORM);
      }

      D3D12_RESOURCE_BARRIER to_copy[4]{};
      for (std::size_t i = 0; i != colors_.size(); ++i) {
        to_copy[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        to_copy[i].Transition.pResource = msaa_colors_[i].Get();
        to_copy[i].Transition.Subresource = 0;
        to_copy[i].Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
        to_copy[i].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
        to_copy[i + colors_.size()].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        to_copy[i + colors_.size()].Transition.pResource = colors_[i].Get();
        to_copy[i + colors_.size()].Transition.Subresource = 0;
        to_copy[i + colors_.size()].Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_DEST;
        to_copy[i + colors_.size()].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
      }
      list->D3DResourceBarrier(4, to_copy);
    } else {
      D3D12_RESOURCE_BARRIER to_copy[2]{};
      for (std::size_t i = 0; i != colors_.size(); ++i) {
        to_copy[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        to_copy[i].Transition.pResource = colors_[i].Get();
        to_copy[i].Transition.Subresource = 0;
        to_copy[i].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        to_copy[i].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
      }
      list->D3DResourceBarrier(2, to_copy);
    }

    for (std::size_t i = 0; i != colors_.size(); ++i) {
      D3D12_TEXTURE_COPY_LOCATION destination{};
      destination.pResource = readbacks_[i].Get();
      destination.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
      destination.PlacedFootprint = footprints_[i];
      D3D12_TEXTURE_COPY_LOCATION source{};
      source.pResource = colors_[i].Get();
      source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
      source.SubresourceIndex = 0;
      list->D3DCopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
    }

    // MSAA sources were already restored immediately after resolving. Only
    // the single-sample copy sources need a transition after readback copies.
    D3D12_RESOURCE_BARRIER to_common[2]{};
    for (std::size_t i = 0; i != colors_.size(); ++i) {
      to_common[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      to_common[i].Transition.pResource = colors_[i].Get();
      to_common[i].Transition.Subresource = 0;
      to_common[i].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
      to_common[i].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
    }
    list->D3DResourceBarrier(2, to_common);
    recorded_submission_ = submission;
    recorded_ = true;
    return S_OK;
  }

  HRESULT ReadFirstPixels(std::uint64_t completed_submission, PixelPair* out) noexcept {
    if (!out) return E_INVALIDARG;
    if (!recorded_ || completed_submission < recorded_submission_) return E_PENDING;

    PixelPair pending{};
    std::array<void*, 2> mapped{};
    std::array<D3D12_RANGE, 2> read_ranges{};
    for (std::size_t i = 0; i != readbacks_.size(); ++i) {
      const UINT64 offset = footprints_[i].Offset;
      if (offset > total_bytes_[i] || total_bytes_[i] - offset < pending[i].size())
        return E_INVALIDARG;
      read_ranges[i] = D3D12_RANGE{static_cast<SIZE_T>(offset),
                                   static_cast<SIZE_T>(offset + pending[i].size())};
      HRESULT hr = readbacks_[i]->Map(0, &read_ranges[i], &mapped[i]);
      if (FAILED(hr)) {
        for (std::size_t previous = 0; previous < i; ++previous) {
          if (mapped[previous]) {
            const D3D12_RANGE no_writes{0, 0};
            readbacks_[previous]->Unmap(0, &no_writes);
          }
        }
        return hr;
      }
      if (!mapped[i]) {
        for (std::size_t previous = 0; previous <= i; ++previous) {
          if (mapped[previous]) {
            const D3D12_RANGE no_writes{0, 0};
            readbacks_[previous]->Unmap(0, &no_writes);
          }
        }
        return E_FAIL;
      }
    }

    for (std::size_t i = 0; i != readbacks_.size(); ++i) {
      const auto* bytes = static_cast<const std::uint8_t*>(mapped[i]);
      std::memcpy(pending[i].data(), bytes + footprints_[i].Offset, pending[i].size());
      const D3D12_RANGE no_writes{0, 0};
      readbacks_[i]->Unmap(0, &no_writes);
    }
    *out = pending;
    return S_OK;
  }

  HRESULT ReadFrames(std::uint64_t completed_submission, FramePair* out) noexcept {
    if (!out) return E_INVALIDARG;
    if (!recorded_ || completed_submission < recorded_submission_) return E_PENDING;
    if (!initialized()) return E_INVALIDARG;

    std::array<D3D12_RESOURCE_DESC, 2> descs{};
    for (std::size_t i = 0; i != colors_.size(); ++i) {
      descs[i] = colors_[i]->GetDesc();
      const auto& desc = descs[i];
      const auto& footprint = footprints_[i].Footprint;
      if (desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D ||
          desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM || desc.MipLevels != 1 ||
          desc.DepthOrArraySize != 1 || desc.SampleDesc.Count != 1 ||
          desc.SampleDesc.Quality != 0 ||
          desc.Width == 0 || desc.Width > kMaximumDimension || desc.Height == 0 ||
          desc.Height > kMaximumDimension ||
          footprint.Format != DXGI_FORMAT_R8G8B8A8_UNORM ||
          footprint.Width != desc.Width || footprint.Height != desc.Height ||
          footprint.Depth != 1)
        return E_INVALIDARG;
    }
    if (descs[0].Width != descs[1].Width || descs[0].Height != descs[1].Height)
      return E_INVALIDARG;
    if (total_bytes_[0] > kMaximumCombinedReadbackBytes ||
        total_bytes_[1] > kMaximumCombinedReadbackBytes - total_bytes_[0])
      return E_INVALIDARG;

    const std::uint32_t width = static_cast<std::uint32_t>(descs[0].Width);
    const std::uint32_t height = descs[0].Height;
    const std::uint64_t row_bytes_64 = static_cast<std::uint64_t>(width) * UINT64_C(4);
    if (row_bytes_64 > (std::numeric_limits<std::size_t>::max)() ||
        height > (std::numeric_limits<std::size_t>::max)() /
                     static_cast<std::size_t>(row_bytes_64))
      return E_INVALIDARG;
    const std::size_t packed_size = static_cast<std::size_t>(row_bytes_64) * height;

    FramePair pending{};
    pending.width = width;
    pending.height = height;
    try {
      pending.rgba[0].resize(packed_size);
      pending.rgba[1].resize(packed_size);
    } catch (const std::bad_alloc&) {
      return E_OUTOFMEMORY;
    } catch (...) {
      return E_FAIL;
    }

    std::array<void*, 2> mapped{};
    std::array<bool, 2> map_succeeded{};
    std::array<std::size_t, 2> read_offsets{};
    std::array<std::size_t, 2> read_ends{};
    for (std::size_t i = 0; i != readbacks_.size(); ++i) {
      const auto& footprint = footprints_[i];
      if (!CalculateReadExtent792(footprint.Offset, footprint.Footprint.RowPitch,
                                  width, height, total_bytes_[i], &read_offsets[i],
                                  &read_ends[i])) {
        for (std::size_t previous = 0; previous < i; ++previous) {
          if (map_succeeded[previous]) {
            const D3D12_RANGE no_writes{0, 0};
            readbacks_[previous]->Unmap(0, &no_writes);
          }
        }
        return E_INVALIDARG;
      }
      const D3D12_RANGE read_range{read_offsets[i], read_ends[i]};
      const HRESULT hr = readbacks_[i]->Map(0, &read_range, &mapped[i]);
      if (FAILED(hr)) {
        for (std::size_t previous = 0; previous < i; ++previous) {
          if (map_succeeded[previous]) {
            const D3D12_RANGE no_writes{0, 0};
            readbacks_[previous]->Unmap(0, &no_writes);
          }
        }
        return hr;
      }
      map_succeeded[i] = true;
      if (!mapped[i]) {
        for (std::size_t previous = 0; previous <= i; ++previous) {
          if (map_succeeded[previous]) {
            const D3D12_RANGE no_writes{0, 0};
            readbacks_[previous]->Unmap(0, &no_writes);
          }
        }
        return E_FAIL;
      }
    }

    bool copied = true;
    for (std::size_t i = 0; i != readbacks_.size(); ++i) {
      const auto& footprint = footprints_[i];
      copied = CopyPackedRows792(
          static_cast<const std::uint8_t*>(mapped[i]),
          read_ends[i], read_offsets[i],
          footprint.Footprint.RowPitch, width, height, pending.rgba[i].data(),
          pending.rgba[i].size()) && copied;
    }
    for (std::size_t i = 0; i != readbacks_.size(); ++i) {
      const D3D12_RANGE no_writes{0, 0};
      readbacks_[i]->Unmap(0, &no_writes);
    }
    if (!copied) return E_INVALIDARG;

    out->width = pending.width;
    out->height = pending.height;
    out->rgba.swap(pending.rgba);
    return S_OK;
  }

  std::uint64_t recorded_submission() const noexcept { return recorded_submission_; }
  bool recorded() const noexcept { return recorded_; }

 private:
  static constexpr UINT64 kMaximumCombinedReadbackBytes = UINT64_C(128) * 1024 * 1024;
  static constexpr UINT64 kMaximumDimension = 4096;

  bool initialized() const noexcept {
    return colors_[0] && colors_[1] && readbacks_[0] && readbacks_[1] &&
           (!multisampled_ || (msaa_colors_[0] && msaa_colors_[1]));
  }

  std::array<ResourcePtr, 2> colors_{};
  std::array<ResourcePtr, 2> msaa_colors_{};
  std::array<ResourcePtr, 2> readbacks_{};
  std::array<D3D12_PLACED_SUBRESOURCE_FOOTPRINT, 2> footprints_{};
  std::array<UINT64, 2> total_bytes_{};
  bool multisampled_ = false;
  std::uint64_t recorded_submission_ = 0;
  bool recorded_ = false;
};

}  // namespace venom::sbs
