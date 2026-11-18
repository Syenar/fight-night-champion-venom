#pragma once
#include <cstdint>

struct ID3D12Resource;
namespace rex::graphics::d3d12 {
class D3D12CommandProcessor;
class D3D12SharedMemory;
ID3D12Resource* PreparePrivateDepthTextureSource(D3D12CommandProcessor& processor,
                                                D3D12SharedMemory& shared,
                                                uint32_t address, uint32_t bytes);
}
