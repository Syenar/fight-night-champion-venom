#pragma once

#include "sbs777_prepared_host_draw.h"

#include "sbs736_matched_eye_resources.h"
#include "sbs737_matched_deferred_eye_boundary.h"
#include "sbs788_prepared_guest_eye_pair.h"
#include "sbs792_gameplay_eye_color_readback.h"

#include <array>
#include <cstddef>
#include <cstring>
#include <memory>
#include <new>
#include <utility>
#include <wrl/client.h>

namespace venom::sbs {

// Owns one prepared guest draw pair and its fenced color readback. This class
// does not submit work or wait in its destructor. The caller MUST keep this
// owner, the deferred recorder, resources, and descriptors alive through the
// actual submission fence (or until GPU idle during shutdown).
class GameplayPairOwner794 {
 public:
  using Snapshot = PreparedGuestTargetsSnapshot788;
  using FramePair = GameplayEyeColorReadback792::FramePair;
  using PipelineStatisticsPair =
      std::array<D3D12_QUERY_DATA_PIPELINE_STATISTICS, 3>;
  static_assert(sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS) == 88,
                "D3D12 pipeline statistics must contain eleven UINT64 counters");
  static_assert(sizeof(PipelineStatisticsPair) == 264,
                "three pipeline-statistics records must occupy 264 bytes");

  GameplayPairOwner794() = default;
  GameplayPairOwner794(const GameplayPairOwner794&) = delete;
  GameplayPairOwner794& operator=(const GameplayPairOwner794&) = delete;
  GameplayPairOwner794(GameplayPairOwner794&&) = delete;
  GameplayPairOwner794& operator=(GameplayPairOwner794&&) = delete;

  HRESULT Initialize(ID3D12Device* device, const Snapshot& snapshot) noexcept {
    if (!device || state_ || attempted_) return E_INVALIDARG;
    if (!snapshot.has_depth || snapshot.rtv_count != 1 ||
        !snapshot.resources[0] || !snapshot.resources[1] ||
        snapshot.dsv_handle.ptr == 0 || snapshot.rtv_handles[0].ptr == 0 ||
        snapshot.dsv_format == DXGI_FORMAT_UNKNOWN ||
        snapshot.rtv_formats[0] != DXGI_FORMAT_R8G8B8A8_UNORM) {
      return E_INVALIDARG;
    }
    for (std::size_t i = 2; i < snapshot.resources.size(); ++i) {
      if (snapshot.resources[i]) return E_INVALIDARG;
    }

    const D3D12_RESOURCE_DESC original_depth = snapshot.resources[0]->GetDesc();
    const D3D12_RESOURCE_DESC original_color = snapshot.resources[1]->GetDesc();
    if (!detail::SameResourceDesc788(original_depth, snapshot.resource_descs[0]) ||
        !detail::SameResourceDesc788(original_color, snapshot.resource_descs[1]) ||
        snapshot.resource_descs[0].Width == 0 ||
        snapshot.resource_descs[0].Height == 0 ||
        snapshot.resource_descs[0].Width > 4096 ||
        snapshot.resource_descs[0].Height > 4096 ||
        !detail::ValidPreparedTexture923(snapshot.resource_descs[0],
                                          static_cast<UINT>(snapshot.resource_descs[0].Width),
                                          snapshot.resource_descs[0].Height) ||
        !detail::ValidPreparedTexture923(snapshot.resource_descs[1],
                                          static_cast<UINT>(snapshot.resource_descs[1].Width),
                                          snapshot.resource_descs[1].Height) ||
        !detail::SamePreparedSampleDesc923(snapshot.resource_descs[0],
                                           snapshot.resource_descs[1]) ||
        snapshot.resource_descs[0].Width != snapshot.resource_descs[1].Width ||
        snapshot.resource_descs[0].Height != snapshot.resource_descs[1].Height ||
        snapshot.resource_descs[0].Width > 4096 ||
        snapshot.resource_descs[0].Height > 4096 ||
        (original_depth.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) == 0 ||
        (original_color.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) == 0) {
      return E_INVALIDARG;
    }

    D3D12_RESOURCE_DESC private_color = snapshot.resource_descs[1];
    private_color.Format = snapshot.rtv_formats[0];
    private_color.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    private_color.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    private_color.Alignment = 0;

    D3D12_RESOURCE_DESC private_depth = snapshot.resource_descs[0];
    private_depth.Format = snapshot.dsv_format;
    private_depth.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    private_depth.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    private_depth.Alignment = 0;

    D3D12_RENDER_TARGET_VIEW_DESC rtv{};
    rtv.Format = snapshot.rtv_formats[0];
    if (private_color.SampleDesc.Count == 4) {
      rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
    } else {
      rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
      rtv.Texture2D.MipSlice = 0;
      rtv.Texture2D.PlaneSlice = 0;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC dsv{};
    dsv.Format = snapshot.dsv_format;
    dsv.Flags = D3D12_DSV_FLAG_NONE;
    if (private_depth.SampleDesc.Count == 4) {
      dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
    } else {
      dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      dsv.Texture2D.MipSlice = 0;
    }

    try {
      std::unique_ptr<State> pending(new State());
      std::shared_ptr<MatchedEyeResources> resources =
          std::make_shared<MatchedEyeResources>();
      HRESULT hr = resources->Initialize(device, private_color, private_depth, rtv, dsv);
      if (FAILED(hr)) return hr;
      hr = pending->boundary.Initialize(resources);
      if (FAILED(hr)) return hr;
      hr = pending->readback.Initialize(device,
                                        resources->ColorResource(MatchedEyeResources::Eye::kLeft),
                                        resources->ColorResource(MatchedEyeResources::Eye::kRight));
      if (FAILED(hr)) return hr;

      D3D12_QUERY_HEAP_DESC query_desc{};
      query_desc.Type = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
      query_desc.Count = 3;
      query_desc.NodeMask = 0;
      hr = device->CreateQueryHeap(
          &query_desc, IID_PPV_ARGS(
                            pending->pipeline_statistics_query_heap.GetAddressOf()));
      if (FAILED(hr)) return hr;

      D3D12_HEAP_PROPERTIES statistics_readback_heap{};
      statistics_readback_heap.Type = D3D12_HEAP_TYPE_READBACK;
      statistics_readback_heap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      statistics_readback_heap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
      statistics_readback_heap.CreationNodeMask = 1;
      statistics_readback_heap.VisibleNodeMask = 1;
      D3D12_RESOURCE_DESC statistics_buffer{};
      statistics_buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      statistics_buffer.Alignment = 0;
      statistics_buffer.Width = sizeof(PipelineStatisticsPair);
      statistics_buffer.Height = 1;
      statistics_buffer.DepthOrArraySize = 1;
      statistics_buffer.MipLevels = 1;
      statistics_buffer.Format = DXGI_FORMAT_UNKNOWN;
      statistics_buffer.SampleDesc.Count = 1;
      statistics_buffer.SampleDesc.Quality = 0;
      statistics_buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      statistics_buffer.Flags = D3D12_RESOURCE_FLAG_NONE;
      hr = device->CreateCommittedResource(
          &statistics_readback_heap, D3D12_HEAP_FLAG_NONE, &statistics_buffer,
          D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
          IID_PPV_ARGS(pending->pipeline_statistics_readback.GetAddressOf()));
      if (FAILED(hr)) return hr;
      pending->resources = std::move(resources);
      state_ = std::move(pending);
      result_ = S_OK;
      return S_OK;
    } catch (const std::bad_alloc&) {
      result_ = E_OUTOFMEMORY;
      return result_;
    }
  }

  void TraceOriginalDraw(rex::graphics::d3d12::DeferredCommandList& list,
                         UINT draw_count, bool indexed) noexcept {
    auto* query_heap = state_->pipeline_statistics_query_heap.Get();
    list.D3DBeginQuery(query_heap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 2);
    EmitPreparedHostDraw777(list, draw_count, indexed);
    list.D3DEndQuery(query_heap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 2);
  }

  HRESULT Record(rex::graphics::d3d12::DeferredCommandList& list,
                 const Snapshot& snapshot, const D3D12_VIEWPORT& viewport,
                 const D3D12_RECT& scissor, UINT system_root_index,
                 D3D12_GPU_VIRTUAL_ADDRESS original_cbv,
                 D3D12_GPU_VIRTUAL_ADDRESS left_cbv,
                 D3D12_GPU_VIRTUAL_ADDRESS right_cbv,
                 UINT draw_count, bool indexed, FLOAT depth_clear,
                 std::uint64_t submission, bool qualified = false,
                 bool stencil_disabled = false) noexcept {
    if (!state_ || attempted_ || submission == 0) return E_INVALIDARG;
    attempted_ = true;
    recorded_submission_ = submission;
    result_ = RecordPreparedGuestEyePair788(
        list, state_->boundary, snapshot, viewport, scissor, system_root_index,
        original_cbv, left_cbv, right_cbv, draw_count, indexed,
        static_cast<UINT>(snapshot.resource_descs[1].Width),
        snapshot.resource_descs[1].Height, depth_clear, qualified, stencil_disabled,
        state_->pipeline_statistics_query_heap.Get());
    state_->pipeline_queries_ended = SUCCEEDED(result_);
    if (SUCCEEDED(result_) && state_->pipeline_queries_ended) {
      list.D3DResolveQueryData(
          state_->pipeline_statistics_query_heap.Get(),
          D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0, 3,
          state_->pipeline_statistics_readback.Get(), 0);
      state_->pipeline_statistics_resolved = true;
      result_ = state_->readback.Record(&list, submission);
    }
    recorded_ = SUCCEEDED(result_);
    return result_;
  }

  HRESULT ReadFrames(std::uint64_t completed_submission, FramePair* out) noexcept {
    if (!out) return E_INVALIDARG;
    if (!state_ || !recorded_ || completed_submission < recorded_submission_)
      return E_PENDING;
    return state_->readback.ReadFrames(completed_submission, out);
  }

  HRESULT ReadPipelineStatistics(std::uint64_t completed_submission,
                                 PipelineStatisticsPair* out) noexcept {
    if (!out) return E_INVALIDARG;
    if (!state_ || !recorded_ || !state_->pipeline_queries_ended ||
        !state_->pipeline_statistics_resolved ||
        completed_submission < recorded_submission_) {
      return E_PENDING;
    }
    const D3D12_RANGE read_range{0, sizeof(PipelineStatisticsPair)};
    void* mapped = nullptr;
    const HRESULT map_result =
        state_->pipeline_statistics_readback->Map(0, &read_range, &mapped);
    if (FAILED(map_result)) return map_result;
    if (mapped == nullptr) {
      const D3D12_RANGE no_write{0, 0};
      state_->pipeline_statistics_readback->Unmap(0, &no_write);
      return E_FAIL;
    }
    PipelineStatisticsPair statistics{};
    std::memcpy(&statistics, mapped, sizeof(statistics));
    const D3D12_RANGE no_write{0, 0};
    state_->pipeline_statistics_readback->Unmap(0, &no_write);
    *out = statistics;
    return S_OK;
  }

  bool initialized() const noexcept { return state_ != nullptr; }
  bool attempted() const noexcept { return attempted_; }
  bool recorded() const noexcept { return recorded_; }
  std::uint64_t recorded_submission() const noexcept { return recorded_submission_; }
  HRESULT result() const noexcept { return result_; }

 private:
  struct State {
    std::shared_ptr<MatchedEyeResources> resources;
    MatchedDeferredEyeBoundary boundary;
    GameplayEyeColorReadback792 readback;
    Microsoft::WRL::ComPtr<ID3D12QueryHeap> pipeline_statistics_query_heap;
    Microsoft::WRL::ComPtr<ID3D12Resource> pipeline_statistics_readback;
    bool pipeline_queries_ended = false;
    bool pipeline_statistics_resolved = false;
  };

  std::unique_ptr<State> state_;
  std::uint64_t recorded_submission_ = 0;
  HRESULT result_ = E_UNEXPECTED;
  bool attempted_ = false;
  bool recorded_ = false;
};

}  // namespace venom::sbs
