#pragma once

#include <rex/graphics/d3d12/deferred_command_list.h>

#include "deferred_eye_pass_boundary.h"
#include "eye_resource_allocator.h"
#include "sbs736_matched_eye_resources.h"
#include "sbs737_matched_deferred_eye_boundary.h"

#include <array>
#include <memory>
#include <new>
#include <utility>
#include <wrl/client.h>

namespace venom::sbs {

// Opt-in diagnostic facade. The default path delegates to the qualified 712
// allocator-backed boundary. The matched path owns an independent pair of
// color/depth targets and descriptors. Keep the facade and its resources alive
// and unmodified through the matching GPU fence; initialize and destruction
// require GPU idle. This helper does not submit, clear, draw, or claim stereo.
class DiagnosticEyeBoundary {
 public:
  using ResourcePtr = Microsoft::WRL::ComPtr<ID3D12Resource>;

  DiagnosticEyeBoundary() = default;
  DiagnosticEyeBoundary(const DiagnosticEyeBoundary&) = delete;
  DiagnosticEyeBoundary& operator=(const DiagnosticEyeBoundary&) = delete;

  HRESULT Initialize(ID3D12Device* device, const EyeResourceAllocator& allocator,
                     bool use_matched) noexcept {
    if (!device || initialized_ || active_) return E_INVALIDARG;
    if (!use_matched) {
      const HRESULT hr = legacy_.Initialize(device, allocator);
      if (SUCCEEDED(hr)) {
        legacy_depths_[0] = allocator.Get(Eye::Left, EyeResourceKind::Depth);
        legacy_depths_[1] = allocator.Get(Eye::Right, EyeResourceKind::Depth);
        initialized_ = true;
      }
      return hr;
    }

    std::array<ResourcePtr, 4> pending_sources{};
    const std::array<std::pair<Eye, EyeResourceKind>, 4> source_kinds{{
        {Eye::Left, EyeResourceKind::Color}, {Eye::Left, EyeResourceKind::Depth},
        {Eye::Right, EyeResourceKind::Color}, {Eye::Right, EyeResourceKind::Depth}}};
    for (std::size_t i = 0; i < source_kinds.size(); ++i) {
      ID3D12Resource* resource = allocator.Get(source_kinds[i].first, source_kinds[i].second);
      if (!resource) return E_INVALIDARG;
      pending_sources[i] = resource;
    }

    const D3D12_RESOURCE_DESC color_desc = pending_sources[0]->GetDesc();
    const D3D12_RESOURCE_DESC depth_desc = pending_sources[1]->GetDesc();
    const D3D12_RESOURCE_DESC right_color_desc = pending_sources[2]->GetDesc();
    const D3D12_RESOURCE_DESC right_depth_desc = pending_sources[3]->GetDesc();
    if (!same_resource_shape(color_desc, right_color_desc) ||
        !same_resource_shape(depth_desc, right_depth_desc))
      return E_INVALIDARG;

    D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{};
    rtv_desc.Format = color_desc.Format;
    if (color_desc.SampleDesc.Count == 1) {
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
      rtv_desc.Texture2D.MipSlice = 0;
      rtv_desc.Texture2D.PlaneSlice = 0;
    } else if (color_desc.SampleDesc.Count == 2 || color_desc.SampleDesc.Count == 4) {
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
    } else {
      return E_INVALIDARG;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
    dsv_desc.Format = depth_desc.Format;
    dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
    if (depth_desc.SampleDesc.Count == 1) {
      dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      dsv_desc.Texture2D.MipSlice = 0;
    } else if (depth_desc.SampleDesc.Count == 2 || depth_desc.SampleDesc.Count == 4) {
      dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
    } else {
      return E_INVALIDARG;
    }

    std::shared_ptr<MatchedEyeResources> pending_matched;
    std::unique_ptr<MatchedDeferredEyeBoundary> pending_boundary;
    HRESULT hr = S_OK;
    try {
      pending_matched = std::make_shared<MatchedEyeResources>();
      hr = pending_matched->Initialize(device, color_desc, depth_desc, rtv_desc, dsv_desc);
      if (FAILED(hr)) return hr;
      pending_boundary = std::make_unique<MatchedDeferredEyeBoundary>();
    } catch (const std::bad_alloc&) {
      return E_OUTOFMEMORY;
    }
    hr = pending_boundary->Initialize(pending_matched);
    if (FAILED(hr)) return hr;

    sources_.swap(pending_sources);
    matched_owner_ = std::move(pending_matched);
    matched_boundary_ = std::move(pending_boundary);
    use_matched_ = true;
    initialized_ = true;
    return S_OK;
  }

  HRESULT Begin(rex::graphics::d3d12::DeferredCommandList* list, Eye eye,
                UINT width, UINT height,
                const EyeResourceAllocator& current_allocator) noexcept {
    if (!initialized_ || active_ || !valid_eye(eye)) return E_INVALIDARG;
    HRESULT hr = E_INVALIDARG;
    if (use_matched_) {
      if (!sources_match(current_allocator)) return E_INVALIDARG;
      hr = matched_boundary_->Begin(list, to_matched_eye(eye), width, height);
    } else {
      hr = legacy_.Begin(list, eye, width, height, current_allocator);
    }
    if (SUCCEEDED(hr)) active_ = true;
    return hr;
  }

  HRESULT End(rex::graphics::d3d12::DeferredCommandList* list, Eye eye) noexcept {
    if (!initialized_ || !active_ || !valid_eye(eye)) return E_INVALIDARG;
    const HRESULT hr = use_matched_ ? matched_boundary_->End(list, to_matched_eye(eye))
                                    : legacy_.End(list, eye);
    if (SUCCEEDED(hr)) active_ = false;
    return hr;
  }

  ID3D12Resource* Color(Eye eye) const noexcept {
    if (!initialized_ || !valid_eye(eye)) return nullptr;
    return use_matched_ ? matched_boundary_->Color(to_matched_eye(eye)) : legacy_.Color(eye);
  }

  ID3D12Resource* Depth(Eye eye) const noexcept {
    if (!initialized_ || !valid_eye(eye)) return nullptr;
    if (use_matched_) return matched_boundary_->Depth(to_matched_eye(eye));
    return legacy_depths_[static_cast<std::size_t>(eye)].Get();
  }

  D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView(Eye eye) const noexcept {
    D3D12_CPU_DESCRIPTOR_HANDLE empty{};
    if (!initialized_ || !valid_eye(eye)) return empty;
    return use_matched_ ? matched_boundary_->RenderTargetView(to_matched_eye(eye))
                        : legacy_.RenderTargetView(eye);
  }

  D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView(Eye eye) const noexcept {
    D3D12_CPU_DESCRIPTOR_HANDLE empty{};
    if (!initialized_ || !valid_eye(eye)) return empty;
    return use_matched_ ? matched_boundary_->DepthStencilView(to_matched_eye(eye))
                        : legacy_.DepthStencilView(eye);
  }

  bool initialized() const noexcept { return initialized_; }
  bool uses_matched_resources() const noexcept { return use_matched_; }

 private:
  static bool valid_eye(Eye eye) noexcept { return eye == Eye::Left || eye == Eye::Right; }

  static MatchedEyeResources::Eye to_matched_eye(Eye eye) noexcept {
    return eye == Eye::Left ? MatchedEyeResources::Eye::kLeft
                            : MatchedEyeResources::Eye::kRight;
  }

  static bool same_resource_shape(const D3D12_RESOURCE_DESC& left,
                                 const D3D12_RESOURCE_DESC& right) noexcept {
    return left.Dimension == right.Dimension && left.Alignment == right.Alignment &&
           left.Width == right.Width && left.Height == right.Height &&
           left.DepthOrArraySize == right.DepthOrArraySize && left.MipLevels == right.MipLevels &&
           left.Format == right.Format && left.SampleDesc.Count == right.SampleDesc.Count &&
           left.SampleDesc.Quality == right.SampleDesc.Quality && left.Layout == right.Layout &&
           left.Flags == right.Flags;
  }

  bool sources_match(const EyeResourceAllocator& allocator) const noexcept {
    if (!sources_[0] || !sources_[1] || !sources_[2] || !sources_[3]) return false;
    return sources_[0].Get() == allocator.Get(Eye::Left, EyeResourceKind::Color) &&
           sources_[1].Get() == allocator.Get(Eye::Left, EyeResourceKind::Depth) &&
           sources_[2].Get() == allocator.Get(Eye::Right, EyeResourceKind::Color) &&
           sources_[3].Get() == allocator.Get(Eye::Right, EyeResourceKind::Depth);
  }

  DeferredEyePassBoundary legacy_;
  std::shared_ptr<MatchedEyeResources> matched_owner_;
  std::unique_ptr<MatchedDeferredEyeBoundary> matched_boundary_;
  std::array<ResourcePtr, 4> sources_{};
  std::array<ResourcePtr, 2> legacy_depths_{};
  bool use_matched_ = false;
  bool initialized_ = false;
  bool active_ = false;
};

}  // namespace venom::sbs
