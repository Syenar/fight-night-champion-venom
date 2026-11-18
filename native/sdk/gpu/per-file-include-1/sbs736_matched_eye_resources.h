#pragma once

#include <d3d12.h>

#include <array>
#include <cstdint>
#include <limits>
#include <wrl/client.h>

namespace venom::sbs {

// Owns a matched pair of color/depth render resources and their private views.
// The caller supplies the exact resource and view descriptions. This helper
// makes no assumptions about rendering content, camera state, or stereo readiness.
class MatchedEyeResources {
 public:
  using ResourcePtr = Microsoft::WRL::ComPtr<ID3D12Resource>;
  using HeapPtr = Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>;

  enum class Eye : std::uint8_t { kLeft = 0, kRight = 1 };

  MatchedEyeResources() = default;
  MatchedEyeResources(const MatchedEyeResources&) = delete;
  MatchedEyeResources& operator=(const MatchedEyeResources&) = delete;

  HRESULT Initialize(ID3D12Device* device,
                     const D3D12_RESOURCE_DESC& color_desc,
                     const D3D12_RESOURCE_DESC& depth_desc,
                     const D3D12_RENDER_TARGET_VIEW_DESC& rtv_desc,
                     const D3D12_DEPTH_STENCIL_VIEW_DESC& dsv_desc) noexcept {
    if (!device || initialized() || !valid_descs(color_desc, depth_desc, rtv_desc, dsv_desc))
      return E_INVALIDARG;

    const D3D12_RESOURCE_ALLOCATION_INFO color_info =
        device->GetResourceAllocationInfo(0, 1, &color_desc);
    const D3D12_RESOURCE_ALLOCATION_INFO depth_info =
        device->GetResourceAllocationInfo(0, 1, &depth_desc);
    if (!valid_allocation_size(color_info.SizeInBytes) ||
        !valid_allocation_size(depth_info.SizeInBytes))
      return E_INVALIDARG;
    const UINT64 color_pair_bytes = color_info.SizeInBytes * UINT64_C(2);
    const UINT64 depth_pair_bytes = depth_info.SizeInBytes * UINT64_C(2);
    if (color_pair_bytes > kMaximumAllocationBytes ||
        depth_pair_bytes > kMaximumAllocationBytes - color_pair_bytes)
      return E_INVALIDARG;

    std::array<ResourcePtr, 2> pending_colors{};
    std::array<ResourcePtr, 2> pending_depths{};
    D3D12_HEAP_PROPERTIES heap{};
    heap.Type = D3D12_HEAP_TYPE_DEFAULT;
    heap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap.CreationNodeMask = 1;
    heap.VisibleNodeMask = 1;

    for (std::size_t i = 0; i != pending_colors.size(); ++i) {
      HRESULT hr = device->CreateCommittedResource(
          &heap, D3D12_HEAP_FLAG_NONE, &color_desc, D3D12_RESOURCE_STATE_COMMON,
          nullptr, IID_PPV_ARGS(pending_colors[i].GetAddressOf()));
      if (FAILED(hr)) return hr;
      hr = device->CreateCommittedResource(
          &heap, D3D12_HEAP_FLAG_NONE, &depth_desc, D3D12_RESOURCE_STATE_COMMON,
          nullptr, IID_PPV_ARGS(pending_depths[i].GetAddressOf()));
      if (FAILED(hr)) return hr;
    }

    HeapPtr pending_rtv_heap;
    HeapPtr pending_dsv_heap;
    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.NumDescriptors = 2;
    rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtv_heap_desc.NodeMask = 0;
    HRESULT hr = device->CreateDescriptorHeap(&rtv_heap_desc,
                                              IID_PPV_ARGS(pending_rtv_heap.GetAddressOf()));
    if (FAILED(hr)) return hr;

    D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc{};
    dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsv_heap_desc.NumDescriptors = 2;
    dsv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsv_heap_desc.NodeMask = 0;
    hr = device->CreateDescriptorHeap(&dsv_heap_desc,
                                      IID_PPV_ARGS(pending_dsv_heap.GetAddressOf()));
    if (FAILED(hr)) return hr;

    const UINT rtv_increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const UINT dsv_increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    if (rtv_increment == 0 || dsv_increment == 0) return E_FAIL;
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = pending_rtv_heap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = pending_dsv_heap->GetCPUDescriptorHandleForHeapStart();
    for (std::size_t i = 0; i != pending_colors.size(); ++i) {
      device->CreateRenderTargetView(pending_colors[i].Get(), &rtv_desc, rtv);
      device->CreateDepthStencilView(pending_depths[i].Get(), &dsv_desc, dsv);
      rtv.ptr += rtv_increment;
      dsv.ptr += dsv_increment;
    }

    if (!all_distinct(pending_colors, pending_depths)) return E_FAIL;

    colors_.swap(pending_colors);
    depths_.swap(pending_depths);
    rtv_heap_.Swap(pending_rtv_heap);
    dsv_heap_.Swap(pending_dsv_heap);
    rtv_increment_ = rtv_increment;
    dsv_increment_ = dsv_increment;
    return S_OK;
  }

  ID3D12Resource* ColorResource(Eye eye) const noexcept {
    const std::size_t index = index_of(eye);
    return index < colors_.size() ? colors_[index].Get() : nullptr;
  }

  ID3D12Resource* DepthResource(Eye eye) const noexcept {
    const std::size_t index = index_of(eye);
    return index < depths_.size() ? depths_[index].Get() : nullptr;
  }

  D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView(Eye eye) const noexcept {
    return descriptor(rtv_heap_.Get(), rtv_increment_, eye);
  }

  D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView(Eye eye) const noexcept {
    return descriptor(dsv_heap_.Get(), dsv_increment_, eye);
  }

  bool initialized() const noexcept {
    return colors_[0] && colors_[1] && depths_[0] && depths_[1] && rtv_heap_ && dsv_heap_;
  }

 private:
  static constexpr UINT64 kMaximumDimension = UINT64_C(4096);
  static constexpr UINT64 kMaximumAllocationBytes = UINT64_C(256) * 1024 * 1024;

  static std::size_t index_of(Eye eye) noexcept {
    return static_cast<std::size_t>(eye);
  }

  static bool valid_sample_count(UINT count) noexcept {
    return count == 1 || count == 2 || count == 4;
  }

  static bool valid_allocation_size(UINT64 bytes) noexcept {
    return bytes != 0 && bytes != std::numeric_limits<UINT64>::max() &&
           bytes <= kMaximumAllocationBytes;
  }

  static bool valid_descs(const D3D12_RESOURCE_DESC& color,
                          const D3D12_RESOURCE_DESC& depth,
                          const D3D12_RENDER_TARGET_VIEW_DESC& rtv,
                          const D3D12_DEPTH_STENCIL_VIEW_DESC& dsv) noexcept {
    if (color.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D ||
        depth.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D ||
        color.Width == 0 || color.Height == 0 || color.Width > kMaximumDimension ||
        color.Height > kMaximumDimension || depth.Width != color.Width ||
        depth.Height != color.Height || color.DepthOrArraySize != 1 ||
        depth.DepthOrArraySize != 1 || color.MipLevels != 1 || depth.MipLevels != 1 ||
        color.Format == DXGI_FORMAT_UNKNOWN || depth.Format == DXGI_FORMAT_UNKNOWN ||
        color.Format == depth.Format ||
        color.SampleDesc.Count != depth.SampleDesc.Count ||
        color.SampleDesc.Quality != depth.SampleDesc.Quality ||
        !valid_sample_count(color.SampleDesc.Count) ||
        (color.SampleDesc.Count == 1 && color.SampleDesc.Quality != 0) ||
        (color.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) == 0 ||
        (depth.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) == 0 ||
        rtv.Format != color.Format || dsv.Format != depth.Format ||
        dsv.Flags != D3D12_DSV_FLAG_NONE)
      return false;

    if (color.SampleDesc.Count == 1) {
      if (color.Layout != D3D12_TEXTURE_LAYOUT_UNKNOWN ||
          depth.Layout != D3D12_TEXTURE_LAYOUT_UNKNOWN ||
          rtv.ViewDimension != D3D12_RTV_DIMENSION_TEXTURE2D ||
          rtv.Texture2D.MipSlice != 0 || rtv.Texture2D.PlaneSlice != 0 ||
          dsv.ViewDimension != D3D12_DSV_DIMENSION_TEXTURE2D ||
          dsv.Texture2D.MipSlice != 0)
        return false;
    } else if (color.Layout != D3D12_TEXTURE_LAYOUT_UNKNOWN ||
               depth.Layout != D3D12_TEXTURE_LAYOUT_UNKNOWN ||
               rtv.ViewDimension != D3D12_RTV_DIMENSION_TEXTURE2DMS ||
               dsv.ViewDimension != D3D12_DSV_DIMENSION_TEXTURE2DMS) {
      return false;
    }
    return true;
  }

  static bool all_distinct(const std::array<ResourcePtr, 2>& colors,
                           const std::array<ResourcePtr, 2>& depths) noexcept {
    const std::array<ID3D12Resource*, 4> resources{{
        colors[0].Get(), colors[1].Get(), depths[0].Get(), depths[1].Get()}};
    for (std::size_t i = 0; i != resources.size(); ++i) {
      if (!resources[i]) return false;
      for (std::size_t j = i + 1; j != resources.size(); ++j) {
        if (resources[i] == resources[j]) return false;
      }
    }
    return true;
  }

  static D3D12_CPU_DESCRIPTOR_HANDLE descriptor(ID3D12DescriptorHeap* heap,
                                                UINT increment, Eye eye) noexcept {
    D3D12_CPU_DESCRIPTOR_HANDLE result{};
    const std::size_t index = index_of(eye);
    if (!heap || increment == 0 || index >= 2) return result;
    result = heap->GetCPUDescriptorHandleForHeapStart();
    result.ptr += static_cast<SIZE_T>(index) * increment;
    return result;
  }

  std::array<ResourcePtr, 2> colors_{};
  std::array<ResourcePtr, 2> depths_{};
  HeapPtr rtv_heap_;
  HeapPtr dsv_heap_;
  UINT rtv_increment_ = 0;
  UINT dsv_increment_ = 0;
};

}  // namespace venom::sbs
