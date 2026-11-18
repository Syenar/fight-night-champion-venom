#pragma once

#include <rex/graphics/d3d12/deferred_command_list.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <wrl/client.h>

namespace venom::sbs {

// Records readback copies for the two eye color textures. This helper does not
// assign meaning to pixel values or make a rendering/stereo acceptance claim.
// The caller must keep this helper, its source colors, descriptors, and the
// deferred recorder alive and unmodified through GPU fence completion, and
// must not Reset the recorder before the recorded submission completes. Source
// colors are expected to be in COMMON state after DeferredEyePassBoundary::End.
class DeferredEyeColorReadback {
 public:
  using ResourcePtr = Microsoft::WRL::ComPtr<ID3D12Resource>;
  using PixelPair = std::array<std::array<std::uint8_t, 4>, 2>;

  DeferredEyeColorReadback() = default;
  DeferredEyeColorReadback(const DeferredEyeColorReadback&) = delete;
  DeferredEyeColorReadback& operator=(const DeferredEyeColorReadback&) = delete;

  HRESULT Initialize(ID3D12Device* device, ID3D12Resource* left,
                     ID3D12Resource* right) noexcept {
    if (!device || !left || !right || left == right || initialized() || recorded_)
      return E_INVALIDARG;

    const std::array<ID3D12Resource*, 2> input{{left, right}};
    std::array<ResourcePtr, 2> pending_colors{};
    std::array<ResourcePtr, 2> pending_readbacks{};
    std::array<D3D12_PLACED_SUBRESOURCE_FOOTPRINT, 2> pending_footprints{};
    std::array<UINT, 2> pending_num_rows{};
    std::array<UINT64, 2> pending_row_sizes{};
    std::array<UINT64, 2> pending_total_bytes{};

    for (std::size_t i = 0; i != input.size(); ++i) {
      const D3D12_RESOURCE_DESC color_desc = input[i]->GetDesc();
      if (color_desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D ||
          color_desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM || color_desc.MipLevels != 1 ||
          color_desc.DepthOrArraySize != 1 || color_desc.SampleDesc.Count != 1 ||
          color_desc.Width == 0 || color_desc.Width > kMaximumDimension ||
          color_desc.Height == 0 || color_desc.Height > kMaximumDimension)
        return E_INVALIDARG;

      pending_colors[i] = input[i];
      device->GetCopyableFootprints(&color_desc, 0, 1, 0, &pending_footprints[i],
                                    &pending_num_rows[i], &pending_row_sizes[i],
                                    &pending_total_bytes[i]);
      const auto& footprint = pending_footprints[i];
      const UINT64 expected_row_size = color_desc.Width * UINT64_C(4);
      if (footprint.Offset % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT != 0 ||
          footprint.Footprint.Format != DXGI_FORMAT_R8G8B8A8_UNORM ||
          footprint.Footprint.Width != color_desc.Width ||
          footprint.Footprint.Height != color_desc.Height || footprint.Footprint.Depth != 1 ||
          footprint.Footprint.RowPitch == 0 ||
          footprint.Footprint.RowPitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT != 0 ||
          pending_num_rows[i] != color_desc.Height || pending_row_sizes[i] != expected_row_size ||
          pending_total_bytes[i] == 0 || pending_total_bytes[i] > kMaximumReadbackBytes ||
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

    colors_.swap(pending_colors);
    readbacks_.swap(pending_readbacks);
    footprints_ = pending_footprints;
    total_bytes_ = pending_total_bytes;
    recorded_submission_ = 0;
    recorded_ = false;
    return S_OK;
  }

  HRESULT Record(rex::graphics::d3d12::DeferredCommandList* list,
                 std::uint64_t submission) noexcept {
    if (!initialized() || !list || recorded_ || submission == 0) return E_INVALIDARG;

    D3D12_RESOURCE_BARRIER to_copy[2]{};
    for (std::size_t i = 0; i != colors_.size(); ++i) {
      to_copy[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      to_copy[i].Transition.pResource = colors_[i].Get();
      to_copy[i].Transition.Subresource = 0;
      to_copy[i].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
      to_copy[i].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
    }
    list->D3DResourceBarrier(2, to_copy);

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

  std::uint64_t recorded_submission() const noexcept { return recorded_submission_; }
  bool recorded() const noexcept { return recorded_; }

 private:
  static constexpr UINT64 kMaximumReadbackBytes = UINT64_C(8) * 1024 * 1024;
  static constexpr UINT64 kMaximumDimension = 1024;

  bool initialized() const noexcept {
    return colors_[0] && colors_[1] && readbacks_[0] && readbacks_[1];
  }

  std::array<ResourcePtr, 2> colors_{};
  std::array<ResourcePtr, 2> readbacks_{};
  std::array<D3D12_PLACED_SUBRESOURCE_FOOTPRINT, 2> footprints_{};
  std::array<UINT64, 2> total_bytes_{};
  std::uint64_t recorded_submission_ = 0;
  bool recorded_ = false;
};

}  // namespace venom::sbs
