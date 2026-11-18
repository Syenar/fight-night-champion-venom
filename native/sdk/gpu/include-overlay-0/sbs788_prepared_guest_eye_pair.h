#pragma once

#include <rex/graphics/d3d12/deferred_command_list.h>
#include <rex/graphics/d3d12/render_target_cache.h>

#include "sbs736_matched_eye_resources.h"
#include "sbs737_matched_deferred_eye_boundary.h"

#include <array>
#include <cmath>
#include <cstdint>

namespace venom::sbs {

using PreparedGuestTargetsSnapshot788 =
    rex::graphics::d3d12::D3D12RenderTargetCache::BoundHostTargetsSnapshot778;

namespace detail {

inline bool SameResourceDesc788(const D3D12_RESOURCE_DESC& a,
                                const D3D12_RESOURCE_DESC& b) {
  return a.Dimension == b.Dimension && a.Alignment == b.Alignment &&
         a.Width == b.Width && a.Height == b.Height &&
         a.DepthOrArraySize == b.DepthOrArraySize && a.MipLevels == b.MipLevels &&
         a.Format == b.Format && a.SampleDesc.Count == b.SampleDesc.Count &&
         a.SampleDesc.Quality == b.SampleDesc.Quality && a.Layout == b.Layout &&
         a.Flags == b.Flags;
}

inline bool ValidPreparedTexture923(const D3D12_RESOURCE_DESC& desc,
                                    UINT width, UINT height) {
  return desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D &&
         width != 0 && height != 0 && width <= 4096 && height <= 4096 &&
         desc.Width == width && desc.Height == height &&
         desc.DepthOrArraySize == 1 && desc.MipLevels == 1 &&
         (desc.SampleDesc.Count == 1 || desc.SampleDesc.Count == 4) &&
         desc.SampleDesc.Quality == 0;
}

inline bool SamePreparedSampleDesc923(const D3D12_RESOURCE_DESC& a,
                                      const D3D12_RESOURCE_DESC& b) {
  return a.SampleDesc.Count == b.SampleDesc.Count &&
         a.SampleDesc.Quality == b.SampleDesc.Quality;
}

inline bool DistinctResources788(
    const std::array<ID3D12Resource*, 6>& resources) {
  for (std::size_t i = 0; i < resources.size(); ++i) {
    if (!resources[i]) return false;
    for (std::size_t j = i + 1; j < resources.size(); ++j) {
      if (resources[i] == resources[j]) return false;
    }
  }
  return true;
}

inline bool ValidSavedRasterState788(const D3D12_VIEWPORT& viewport,
                                     const D3D12_RECT& scissor,
                                     UINT width, UINT height) {
  constexpr float kViewportBoundsMin = -32768.0f;
  constexpr float kViewportBoundsMax = 32767.0f;
  if (width == 0 || height == 0 || width > 4096 || height > 4096 ||
      !std::isfinite(viewport.TopLeftX) || !std::isfinite(viewport.TopLeftY) ||
      !std::isfinite(viewport.Width) || !std::isfinite(viewport.Height) ||
      !std::isfinite(viewport.MinDepth) || !std::isfinite(viewport.MaxDepth) ||
      viewport.Width <= 0.0f || viewport.Height <= 0.0f ||
      viewport.TopLeftX < kViewportBoundsMin ||
      viewport.TopLeftX > kViewportBoundsMax ||
      viewport.TopLeftY < kViewportBoundsMin ||
      viewport.TopLeftY > kViewportBoundsMax ||
      viewport.MinDepth < 0.0f || viewport.MaxDepth > 1.0f ||
      viewport.MinDepth > viewport.MaxDepth) {
    return false;
  }
  const float right = viewport.TopLeftX + viewport.Width;
  const float bottom = viewport.TopLeftY + viewport.Height;
  if (!std::isfinite(right) || !std::isfinite(bottom) ||
      right < kViewportBoundsMin || right > kViewportBoundsMax ||
      bottom < kViewportBoundsMin || bottom > kViewportBoundsMax) {
    return false;
  }
  return scissor.left >= 0 && scissor.top >= 0 &&
         scissor.right > scissor.left && scissor.bottom > scissor.top &&
         static_cast<UINT>(scissor.right) <= width &&
         static_cast<UINT>(scissor.bottom) <= height;
}

class PreparedGuestEyeRestoreGuard788 {
 public:
  using Eye = MatchedEyeResources::Eye;
  using Recorder = rex::graphics::d3d12::DeferredCommandList;

  PreparedGuestEyeRestoreGuard788(
      Recorder& list, MatchedDeferredEyeBoundary& boundary,
      const PreparedGuestTargetsSnapshot788& original,
      const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor,
      UINT system_root_index, D3D12_GPU_VIRTUAL_ADDRESS original_cbv)
      : list_(list), boundary_(boundary), original_(original),
        viewport_(viewport), scissor_(scissor),
        system_root_index_(system_root_index), original_cbv_(original_cbv) {}

  PreparedGuestEyeRestoreGuard788(const PreparedGuestEyeRestoreGuard788&) = delete;
  PreparedGuestEyeRestoreGuard788& operator=(const PreparedGuestEyeRestoreGuard788&) = delete;

  ~PreparedGuestEyeRestoreGuard788() { (void)Restore(); }

  void BeforeBegin(Eye eye) { eye_ = eye; eye_pending_ = true; }
  void EyeEnded() { eye_pending_ = false; }

  HRESULT Restore() {
    if (restored_) return end_result_;
    if (eye_pending_ && boundary_.active()) {
      end_result_ = boundary_.End(&list_, eye_);
    }
    eye_pending_ = false;
    // Restore the exact one-RTV-plus-DSV snapshot, followed by the original
    // viewport, scissor, and system CBV using the same deferred recorder.
    list_.D3DOMSetRenderTargets(1, original_.rtv_handles.data(), FALSE,
                                &original_.dsv_handle);
    list_.RSSetViewport(viewport_);
    list_.RSSetScissorRect(scissor_);
    list_.D3DSetGraphicsRootConstantBufferView(system_root_index_, original_cbv_);
    restored_ = true;
    return end_result_;
  }

 private:
  Recorder& list_;
  MatchedDeferredEyeBoundary& boundary_;
  const PreparedGuestTargetsSnapshot788& original_;
  const D3D12_VIEWPORT& viewport_;
  const D3D12_RECT& scissor_;
  UINT system_root_index_;
  D3D12_GPU_VIRTUAL_ADDRESS original_cbv_;
  Eye eye_ = Eye::kLeft;
  HRESULT end_result_ = S_OK;
  bool eye_pending_ = false;
  bool restored_ = false;
};

}  // namespace detail

// Records a tightly gated diagnostic pair into the caller's existing deferred
// submission. The caller must establish `qualified` and `stencil_disabled`
// from authoritative pipeline/root-state checks and pass a fresh, authentic
// SnapshotBoundHostTargets778 result. That snapshot producer validates guest
// key depth/color roles; this helper treats guest keys as opaque metadata for
// caller-side PSO comparison. The caller must retain all referenced
// resources, views, boundary, and recorder unchanged until the submission fence
// completes. This helper performs no pipeline selection, shader classification,
// guest query/export work, CP cache mutation, or automatic guest-draw replay.
// DeferredCommandList recording may allocate internally; allocation failure in
// that recorder is fatal and cannot be made recoverable by this helper.
inline HRESULT RecordPreparedGuestEyePair788(
    rex::graphics::d3d12::DeferredCommandList& list,
    MatchedDeferredEyeBoundary& boundary,
    const PreparedGuestTargetsSnapshot788& snapshot,
    const D3D12_VIEWPORT& saved_viewport,
    const D3D12_RECT& saved_scissor,
    UINT system_root_index,
    D3D12_GPU_VIRTUAL_ADDRESS original_cbv,
    D3D12_GPU_VIRTUAL_ADDRESS left_cbv,
    D3D12_GPU_VIRTUAL_ADDRESS right_cbv,
    UINT draw_count, bool indexed,
    UINT width, UINT height, FLOAT depth_clear,
    bool qualified = false, bool stencil_disabled = false,
    ID3D12QueryHeap* pipeline_statistics_query_heap = nullptr) {
  using Eye = MatchedEyeResources::Eye;
  constexpr UINT kMaximumDimension = 4096;
  constexpr D3D12_GPU_VIRTUAL_ADDRESS kCbvAlignment =
      D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

  if (!qualified || !stencil_disabled || !boundary.initialized() || boundary.active() ||
      draw_count == 0 || system_root_index >= 32 || width == 0 || height == 0 ||
      width > kMaximumDimension || height > kMaximumDimension ||
      (depth_clear != 0.0f && depth_clear != 1.0f) ||
      original_cbv == 0 || left_cbv == 0 || right_cbv == 0 ||
      original_cbv % kCbvAlignment != 0 || left_cbv % kCbvAlignment != 0 ||
      right_cbv % kCbvAlignment != 0 || original_cbv == left_cbv ||
      original_cbv == right_cbv || left_cbv == right_cbv ||
      !detail::ValidSavedRasterState788(saved_viewport, saved_scissor, width, height)) {
    return E_INVALIDARG;
  }

  if (!snapshot.has_depth || snapshot.rtv_count != 1 ||
      snapshot.resources[0].Get() == nullptr || snapshot.resources[1].Get() == nullptr ||
      snapshot.rtv_handles[0].ptr == 0 || snapshot.dsv_handle.ptr == 0 ||
      snapshot.rtv_formats[0] == DXGI_FORMAT_UNKNOWN ||
      snapshot.dsv_format == DXGI_FORMAT_UNKNOWN) {
    return E_INVALIDARG;
  }
  for (std::size_t i = 2; i < snapshot.resources.size(); ++i) {
    if (snapshot.resources[i] != nullptr) return E_INVALIDARG;
  }

  ID3D12Resource* const original_depth = snapshot.resources[0].Get();
  ID3D12Resource* const original_color = snapshot.resources[1].Get();
  const D3D12_RESOURCE_DESC actual_original_depth = original_depth->GetDesc();
  const D3D12_RESOURCE_DESC actual_original_color = original_color->GetDesc();
  if (!detail::SameResourceDesc788(actual_original_depth, snapshot.resource_descs[0]) ||
      !detail::SameResourceDesc788(actual_original_color, snapshot.resource_descs[1]) ||
      !detail::ValidPreparedTexture923(snapshot.resource_descs[0], width, height) ||
      !detail::ValidPreparedTexture923(snapshot.resource_descs[1], width, height) ||
      !detail::SamePreparedSampleDesc923(snapshot.resource_descs[0],
                                         snapshot.resource_descs[1]) ||
      (actual_original_depth.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) == 0 ||
      (actual_original_color.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) == 0) {
    return E_INVALIDARG;
  }

  ID3D12Resource* const left_color = boundary.Color(Eye::kLeft);
  ID3D12Resource* const left_depth = boundary.Depth(Eye::kLeft);
  ID3D12Resource* const right_color = boundary.Color(Eye::kRight);
  ID3D12Resource* const right_depth = boundary.Depth(Eye::kRight);
  const D3D12_RESOURCE_DESC left_color_desc = left_color ? left_color->GetDesc() : D3D12_RESOURCE_DESC{};
  const D3D12_RESOURCE_DESC left_depth_desc = left_depth ? left_depth->GetDesc() : D3D12_RESOURCE_DESC{};
  const D3D12_RESOURCE_DESC right_color_desc = right_color ? right_color->GetDesc() : D3D12_RESOURCE_DESC{};
  const D3D12_RESOURCE_DESC right_depth_desc = right_depth ? right_depth->GetDesc() : D3D12_RESOURCE_DESC{};
  const D3D12_CPU_DESCRIPTOR_HANDLE left_rtv = boundary.RenderTargetView(Eye::kLeft);
  const D3D12_CPU_DESCRIPTOR_HANDLE left_dsv = boundary.DepthStencilView(Eye::kLeft);
  const D3D12_CPU_DESCRIPTOR_HANDLE right_rtv = boundary.RenderTargetView(Eye::kRight);
  const D3D12_CPU_DESCRIPTOR_HANDLE right_dsv = boundary.DepthStencilView(Eye::kRight);
  if (!detail::DistinctResources788({original_depth, original_color, left_color, left_depth,
                                     right_color, right_depth}) ||
      left_rtv.ptr == 0 || left_dsv.ptr == 0 || right_rtv.ptr == 0 || right_dsv.ptr == 0 ||
      !detail::ValidPreparedTexture923(left_color_desc, width, height) ||
      !detail::ValidPreparedTexture923(left_depth_desc, width, height) ||
      !detail::ValidPreparedTexture923(right_color_desc, width, height) ||
      !detail::ValidPreparedTexture923(right_depth_desc, width, height) ||
      !detail::SamePreparedSampleDesc923(left_color_desc, snapshot.resource_descs[1]) ||
      !detail::SamePreparedSampleDesc923(left_depth_desc, snapshot.resource_descs[1]) ||
      !detail::SamePreparedSampleDesc923(right_color_desc, snapshot.resource_descs[1]) ||
      !detail::SamePreparedSampleDesc923(right_depth_desc, snapshot.resource_descs[1]) ||
      left_color_desc.Format != snapshot.rtv_formats[0] ||
      right_color_desc.Format != snapshot.rtv_formats[0] ||
      left_depth_desc.Format != snapshot.dsv_format ||
      right_depth_desc.Format != snapshot.dsv_format) {
    return E_INVALIDARG;
  }

  detail::PreparedGuestEyeRestoreGuard788 restore(
      list, boundary, snapshot, saved_viewport, saved_scissor,
      system_root_index, original_cbv);
  const std::array<Eye, 2> eyes{{Eye::kLeft, Eye::kRight}};
  constexpr FLOAT kBlack[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  for (const Eye eye : eyes) {
    restore.BeforeBegin(eye);
    HRESULT hr = boundary.Begin(&list, eye, width, height);
    if (FAILED(hr)) return hr;

    // Begin installs its default viewport/scissor; restore the guest's actual
    // raster state directly on this recorder without touching CP-side caches.
    list.RSSetViewport(saved_viewport);
    list.RSSetScissorRect(saved_scissor);
    const D3D12_CPU_DESCRIPTOR_HANDLE rtv = boundary.RenderTargetView(eye);
    const D3D12_CPU_DESCRIPTOR_HANDLE dsv = boundary.DepthStencilView(eye);
    list.D3DClearRenderTargetView(rtv, kBlack, 0, nullptr);
    list.D3DClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth_clear, 0, 0, nullptr);
    const D3D12_GPU_VIRTUAL_ADDRESS eye_cbv = eye == Eye::kLeft ? left_cbv : right_cbv;
    list.D3DSetGraphicsRootConstantBufferView(system_root_index, eye_cbv);
    const UINT query_index = eye == Eye::kLeft ? 0u : 1u;
    if (indexed) {
      if (pipeline_statistics_query_heap) {
        list.D3DBeginQuery(pipeline_statistics_query_heap,
                           D3D12_QUERY_TYPE_PIPELINE_STATISTICS, query_index);
      }
      list.D3DDrawIndexedInstanced(draw_count, 1, 0, 0, 0);
      if (pipeline_statistics_query_heap) {
        list.D3DEndQuery(pipeline_statistics_query_heap,
                         D3D12_QUERY_TYPE_PIPELINE_STATISTICS, query_index);
      }
    } else {
      if (pipeline_statistics_query_heap) {
        list.D3DBeginQuery(pipeline_statistics_query_heap,
                           D3D12_QUERY_TYPE_PIPELINE_STATISTICS, query_index);
      }
      list.D3DDrawInstanced(draw_count, 1, 0, 0);
      if (pipeline_statistics_query_heap) {
        list.D3DEndQuery(pipeline_statistics_query_heap,
                         D3D12_QUERY_TYPE_PIPELINE_STATISTICS, query_index);
      }
    }
    hr = boundary.End(&list, eye);
    if (FAILED(hr)) return hr;
    restore.EyeEnded();
  }
  return restore.Restore();
}

}  // namespace venom::sbs
