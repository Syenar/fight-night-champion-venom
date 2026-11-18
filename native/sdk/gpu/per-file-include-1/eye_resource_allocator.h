#pragma once

#include <array>
#include <cstdint>
#include <wrl/client.h>
#include <d3d12.h>
#include "sbs_eye_frame_input.h"

namespace venom::sbs {

enum class EyeResourceKind : std::size_t { Color = 0, Depth = 1, Resolved = 2, History = 3 };

// Owns distinct per-eye textures. The caller must ensure none of the current
// resources are in flight before a successful Allocate replaces/releases them.
class EyeResourceAllocator {
 public:
  using ResourcePtr = Microsoft::WRL::ComPtr<ID3D12Resource>;

  HRESULT Allocate(ID3D12Device* device, UINT width, UINT height) noexcept {
    if (!device || width == 0 || height == 0) return E_INVALIDARG;

    std::array<ResourceSet, 2> pending{};
    for (auto& eye : pending) {
      HRESULT hr = CreateTexture(device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                 D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, eye.color);
      if (FAILED(hr)) return hr;
      hr = CreateTexture(device, width, height, DXGI_FORMAT_D32_FLOAT,
                         D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, eye.depth);
      if (FAILED(hr)) return hr;
      hr = CreateTexture(device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
                         D3D12_RESOURCE_FLAG_NONE, eye.resolved);
      if (FAILED(hr)) return hr;
      hr = CreateTexture(device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
                         D3D12_RESOURCE_FLAG_NONE, eye.history);
      if (FAILED(hr)) return hr;
    }
    resources_.swap(pending);
    return S_OK;
  }

  ID3D12Resource* Get(Eye eye, EyeResourceKind kind) const noexcept {
    const auto e = static_cast<std::size_t>(eye);
    const auto k = static_cast<std::size_t>(kind);
    if (e >= resources_.size() || k >= 4) return nullptr;
    const auto& set = resources_[e];
    switch (kind) {
      case EyeResourceKind::Color: return set.color.Get();
      case EyeResourceKind::Depth: return set.depth.Get();
      case EyeResourceKind::Resolved: return set.resolved.Get();
      case EyeResourceKind::History: return set.history.Get();
    }
    return nullptr;
  }

 private:
  struct ResourceSet {
    ResourcePtr color;
    ResourcePtr depth;
    ResourcePtr resolved;
    ResourcePtr history;
  };

  static HRESULT CreateTexture(ID3D12Device* device, UINT width, UINT height,
                               DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags,
                               ResourcePtr& resource) noexcept {
    D3D12_HEAP_PROPERTIES heap{};
    heap.Type = D3D12_HEAP_TYPE_DEFAULT;
    heap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap.CreationNodeMask = 1;
    heap.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = flags;
    return device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
                                           D3D12_RESOURCE_STATE_COMMON, nullptr,
                                           IID_PPV_ARGS(resource.ReleaseAndGetAddressOf()));
  }

  std::array<ResourceSet, 2> resources_{};
};

}  // namespace venom::sbs
