#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace venom::sbs {
enum class Eye : uint8_t { Left=0, Right=1 };
struct ResourceSet { uint64_t color=0, depth=0, resolved=0, history=0; };
struct FrameInput {
  uint64_t guest_execution_id=0; uint32_t width=0,height=0; std::array<ResourceSet,2> eye{};
  const ResourceSet& resources(Eye e) const { return eye[static_cast<size_t>(e)]; }
  bool valid() const {
    if (!guest_execution_id || !width || !height) return false;
    const uint64_t ids[] = {eye[0].color, eye[0].depth, eye[0].resolved,
                            eye[0].history, eye[1].color, eye[1].depth,
                            eye[1].resolved, eye[1].history};
    for (std::size_t i = 0; i < std::size(ids); ++i) {
      if (!ids[i]) return false;
      for (std::size_t j = 0; j < i; ++j) if (ids[i] == ids[j]) return false;
    }
    return true;
  }
};
}
