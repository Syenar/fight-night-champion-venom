#pragma once

#include <rex/graphics/d3d12/deferred_command_list.h>
#include "sbs736_matched_eye_resources.h"

#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>

namespace venom::sbs {

// Records only per-eye resource state transitions, output binding, and viewport
// state. It neither clears nor draws nor submits. Keep this boundary, its shared
// resources, and the deferred recorder alive and unmodified through the matching
// GPU fence. Initialize and destruction require GPU idle; callers must serialize
// all uses of these resources so they begin/end in COMMON state.
class MatchedDeferredEyeBoundary {
 public:
  using Eye = MatchedEyeResources::Eye;

  MatchedDeferredEyeBoundary() = default;
  MatchedDeferredEyeBoundary(const MatchedDeferredEyeBoundary&) = delete;
  MatchedDeferredEyeBoundary& operator=(const MatchedDeferredEyeBoundary&) = delete;

  HRESULT Initialize(std::shared_ptr<const MatchedEyeResources> owner) noexcept {
    if (!owner || !owner->initialized() || owner_ || active_list_) return E_INVALIDARG;
    for (const Eye eye : std::array<Eye, 2>{{Eye::kLeft, Eye::kRight}}) {
      if (!owner->ColorResource(eye) || !owner->DepthResource(eye) ||
          owner->RenderTargetView(eye).ptr == 0 || owner->DepthStencilView(eye).ptr == 0)
        return E_INVALIDARG;
    }
    owner_ = std::move(owner);
    return S_OK;
  }

  HRESULT Begin(rex::graphics::d3d12::DeferredCommandList* list, Eye eye,
                UINT width, UINT height) noexcept {
    if (!list || !initialized() || active_list_ || !valid_eye(eye) || width == 0 || height == 0 ||
        width > kMaximumDimension || height > kMaximumDimension ||
        width > static_cast<UINT>((std::numeric_limits<LONG>::max)()) ||
        height > static_cast<UINT>((std::numeric_limits<LONG>::max)()))
      return E_INVALIDARG;

    ID3D12Resource* color = owner_->ColorResource(eye);
    ID3D12Resource* depth = owner_->DepthResource(eye);
    const D3D12_RESOURCE_DESC color_desc = color->GetDesc();
    const D3D12_RESOURCE_DESC depth_desc = depth->GetDesc();
    const D3D12_CPU_DESCRIPTOR_HANDLE rtv = owner_->RenderTargetView(eye);
    const D3D12_CPU_DESCRIPTOR_HANDLE dsv = owner_->DepthStencilView(eye);
    if (color_desc.Width != width || color_desc.Height != height ||
        depth_desc.Width != width || depth_desc.Height != height || rtv.ptr == 0 || dsv.ptr == 0)
      return E_INVALIDARG;

    D3D12_RESOURCE_BARRIER color_barrier{};
    color_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    color_barrier.Transition.pResource = color;
    color_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    color_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    color_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    list->D3DResourceBarrier(1, &color_barrier);

    D3D12_RESOURCE_BARRIER depth_barrier{};
    depth_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    depth_barrier.Transition.pResource = depth;
    depth_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    depth_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    depth_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    list->D3DResourceBarrier(1, &depth_barrier);
    list->D3DOMSetRenderTargets(1, &rtv, FALSE, &dsv);

    D3D12_VIEWPORT viewport{};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    list->RSSetViewport(viewport);
    const D3D12_RECT scissor{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    list->RSSetScissorRect(scissor);

    active_list_ = list;
    active_eye_ = static_cast<int>(eye);
    return S_OK;
  }

  HRESULT End(rex::graphics::d3d12::DeferredCommandList* list, Eye eye) noexcept {
    if (!list || !active_list_ || active_list_ != list || active_eye_ < 0 ||
        !valid_eye(eye) || active_eye_ != static_cast<int>(eye))
      return E_INVALIDARG;
    const Eye active = static_cast<Eye>(active_eye_);
    D3D12_RESOURCE_BARRIER barriers[2]{};
    barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[0].Transition.pResource = owner_->ColorResource(active);
    barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
    barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[1].Transition.pResource = owner_->DepthResource(active);
    barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
    list->D3DResourceBarrier(2, barriers);
    active_list_ = nullptr;
    active_eye_ = -1;
    return S_OK;
  }

  ID3D12Resource* Color(Eye eye) const noexcept {
    return initialized() && valid_eye(eye) ? owner_->ColorResource(eye) : nullptr;
  }

  ID3D12Resource* Depth(Eye eye) const noexcept {
    return initialized() && valid_eye(eye) ? owner_->DepthResource(eye) : nullptr;
  }

  D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView(Eye eye) const noexcept {
    D3D12_CPU_DESCRIPTOR_HANDLE empty{};
    return initialized() && valid_eye(eye) ? owner_->RenderTargetView(eye) : empty;
  }

  D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView(Eye eye) const noexcept {
    D3D12_CPU_DESCRIPTOR_HANDLE empty{};
    return initialized() && valid_eye(eye) ? owner_->DepthStencilView(eye) : empty;
  }

  bool initialized() const noexcept { return owner_ && owner_->initialized(); }
  bool active() const noexcept { return active_list_ != nullptr; }

 private:
  static constexpr UINT kMaximumDimension = 4096;

  static bool valid_eye(Eye eye) noexcept {
    return eye == Eye::kLeft || eye == Eye::kRight;
  }

  std::shared_ptr<const MatchedEyeResources> owner_;
  rex::graphics::d3d12::DeferredCommandList* active_list_ = nullptr;
  int active_eye_ = -1;
};

}  // namespace venom::sbs
