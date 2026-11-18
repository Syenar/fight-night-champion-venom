#pragma once

#include "eye_resource_allocator.h"

#include <array>
#include <cstdint>
#include <limits>

namespace venom::sbs {

// Binds one eye's color/depth targets and brackets only the state transitions
// and viewport/scissor setup. It does not replay guest draws or submit work.
// The caller must keep the GPU idle before Initialize or destruction, and must
// provide exclusive command-list sequencing while a pass is active.
class EyePassBoundary {
 public:
  using ResourcePtr = Microsoft::WRL::ComPtr<ID3D12Resource>;

  EyePassBoundary() = default;
  EyePassBoundary(const EyePassBoundary&) = delete;
  EyePassBoundary& operator=(const EyePassBoundary&) = delete;

  HRESULT Initialize(ID3D12Device* device, const EyeResourceAllocator& allocator) noexcept {
    if (!device || active_eye_ >= 0) return E_INVALIDARG;

    std::array<ResourcePtr, 8> pending_resources{};
    for (std::size_t i = 0; i != 2; ++i) {
      for (std::size_t k = 0; k != 4; ++k) {
        const auto kind = static_cast<EyeResourceKind>(k);
        ID3D12Resource* resource = allocator.Get(static_cast<Eye>(i), kind);
        if (!resource) return E_INVALIDARG;
        pending_resources[i * 4 + k] = resource;
      }
    }

    D3D12_DESCRIPTOR_HEAP_DESC rtv_desc{};
    rtv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_desc.NumDescriptors = 2;
    rtv_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtv_desc.NodeMask = 0;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pending_rtv;
    HRESULT hr = device->CreateDescriptorHeap(&rtv_desc, IID_PPV_ARGS(pending_rtv.GetAddressOf()));
    if (FAILED(hr)) return hr;

    D3D12_DESCRIPTOR_HEAP_DESC dsv_desc{};
    dsv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsv_desc.NumDescriptors = 2;
    dsv_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsv_desc.NodeMask = 0;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pending_dsv;
    hr = device->CreateDescriptorHeap(&dsv_desc, IID_PPV_ARGS(pending_dsv.GetAddressOf()));
    if (FAILED(hr)) return hr;

    const UINT rtv_stride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const UINT dsv_stride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    auto rtv = pending_rtv->GetCPUDescriptorHandleForHeapStart();
    auto dsv = pending_dsv->GetCPUDescriptorHandleForHeapStart();
    D3D12_RENDER_TARGET_VIEW_DESC color_view{};
    color_view.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    color_view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    D3D12_DEPTH_STENCIL_VIEW_DESC depth_view{};
    depth_view.Format = DXGI_FORMAT_D32_FLOAT;
    depth_view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depth_view.Flags = D3D12_DSV_FLAG_NONE;
    for (std::size_t i = 0; i != 2; ++i) {
      device->CreateRenderTargetView(pending_resources[i * 4].Get(), &color_view, rtv);
      device->CreateDepthStencilView(pending_resources[i * 4 + 1].Get(), &depth_view, dsv);
      rtv.ptr += rtv_stride;
      dsv.ptr += dsv_stride;
    }

    rtv_heap_.Swap(pending_rtv);
    dsv_heap_.Swap(pending_dsv);
    resources_.swap(pending_resources);
    rtv_stride_ = rtv_stride;
    dsv_stride_ = dsv_stride;
    return S_OK;
  }

  HRESULT Begin(ID3D12GraphicsCommandList* list, Eye eye, UINT width, UINT height,
                const EyeResourceAllocator& current_allocator) noexcept {
    if (!list || width == 0 || height == 0 || !initialized() || active_eye_ >= 0 ||
        width > static_cast<UINT>((std::numeric_limits<LONG>::max)()) ||
        height > static_cast<UINT>((std::numeric_limits<LONG>::max)())) return E_INVALIDARG;
    const std::size_t index = static_cast<std::size_t>(eye);
    if (index >= 2) return E_INVALIDARG;
    for (std::size_t i = 0; i != 2; ++i) {
      for (std::size_t k = 0; k != 4; ++k) {
        const auto kind = static_cast<EyeResourceKind>(k);
        if (resources_[i * 4 + k].Get() != current_allocator.Get(static_cast<Eye>(i), kind))
          return E_INVALIDARG;
      }
    }
    const D3D12_RESOURCE_DESC color_desc = resources_[index * 4].Get()->GetDesc();
    const D3D12_RESOURCE_DESC depth_desc = resources_[index * 4 + 1].Get()->GetDesc();
    if (color_desc.Width != width || color_desc.Height != height ||
        depth_desc.Width != width || depth_desc.Height != height) return E_INVALIDARG;

    D3D12_RESOURCE_BARRIER color_barrier{};
    color_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    color_barrier.Transition.pResource = resources_[index * 4].Get();
    color_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    color_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    color_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    list->ResourceBarrier(1, &color_barrier);

    D3D12_RESOURCE_BARRIER depth_barrier{};
    depth_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    depth_barrier.Transition.pResource = resources_[index * 4 + 1].Get();
    depth_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    depth_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    depth_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    list->ResourceBarrier(1, &depth_barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = rtv_heap_->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = dsv_heap_->GetCPUDescriptorHandleForHeapStart();
    rtv.ptr += index * rtv_stride_;
    dsv.ptr += index * dsv_stride_;
    list->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    D3D12_VIEWPORT viewport{};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    list->RSSetViewports(1, &viewport);

    D3D12_RECT scissor{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    list->RSSetScissorRects(1, &scissor);
    active_list_ = list;
    active_eye_ = static_cast<int>(index);
    return S_OK;
  }

  HRESULT End(ID3D12GraphicsCommandList* list, Eye eye) noexcept {
    if (!list || !active_list_ || active_list_.Get() != list || active_eye_ < 0 ||
        static_cast<std::size_t>(active_eye_) != static_cast<std::size_t>(eye))
      return E_INVALIDARG;
    const std::size_t index = static_cast<std::size_t>(active_eye_);
    D3D12_RESOURCE_BARRIER barriers[2]{};
    barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[0].Transition.pResource = resources_[index * 4].Get();
    barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
    barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[1].Transition.pResource = resources_[index * 4 + 1].Get();
    barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
    list->ResourceBarrier(2, barriers);
    active_list_.Reset();
    active_eye_ = -1;
    return S_OK;
  }

  ID3D12Resource* Color(Eye eye) const noexcept {
    const auto index = static_cast<std::size_t>(eye);
    return index < 2 ? resources_[index * 4].Get() : nullptr;
  }

  D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView(Eye eye) const noexcept {
    D3D12_CPU_DESCRIPTOR_HANDLE handle{};
    const auto index = static_cast<std::size_t>(eye);
    if (index >= 2 || !rtv_heap_) return handle;
    handle = rtv_heap_->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += index * rtv_stride_;
    return handle;
  }

 private:
  bool initialized() const noexcept {
    if (!rtv_heap_ || !dsv_heap_) return false;
    for (const auto& resource : resources_) if (!resource) return false;
    return true;
  }

  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap_;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_heap_;
  std::array<ResourcePtr, 8> resources_{};
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> active_list_;
  UINT rtv_stride_ = 0;
  UINT dsv_stride_ = 0;
  int active_eye_ = -1;
};

}  // namespace venom::sbs
