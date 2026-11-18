#ifndef VENOM_SBS746_STEREO_CLIP_OFFSET_H_
#define VENOM_SBS746_STEREO_CLIP_OFFSET_H_

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace venom {
namespace sbs {

// Dedicated host constants: four 32-bit words, mirrored by sbs746_stereo_clip_offset.hlsli.
// This layout is not a guest constant slot and must be bound independently per eye.
struct alignas(16) StereoClipOffsetParameters {
  float signed_eye_translation_clip;
  float convergence_clip_w;
  std::uint32_t enabled;
  std::uint32_t reserved;
};

struct HomogeneousClipPosition {
  float x;
  float y;
  float z;
  float w;
};

static_assert(sizeof(StereoClipOffsetParameters) == 16, "stereo clip parameters must be four DWORDs");
static_assert(alignof(StereoClipOffsetParameters) == 16, "stereo clip parameters must be 16-byte aligned");
static_assert(offsetof(StereoClipOffsetParameters, signed_eye_translation_clip) == 0);
static_assert(offsetof(StereoClipOffsetParameters, convergence_clip_w) == 4);
static_assert(offsetof(StereoClipOffsetParameters, enabled) == 8);
static_assert(offsetof(StereoClipOffsetParameters, reserved) == 12);

// Apply only after the caller has reconstructed the guest homogeneous clip position.
// Use a negative signed_eye_translation_clip for left and positive for right. Unknown
// guest passes/orthographic cases must be filtered by the caller; this helper guesses none.
inline HomogeneousClipPosition ApplyStereoClipOffset(
    const HomogeneousClipPosition& guest_clip,
    const StereoClipOffsetParameters& parameters) noexcept {
  if (parameters.enabled == 0) return guest_clip;
  if (parameters.enabled != 1 || parameters.reserved != 0) return guest_clip;
  if (!std::isfinite(guest_clip.x) || !std::isfinite(guest_clip.y) ||
      !std::isfinite(guest_clip.z) || !std::isfinite(guest_clip.w) ||
      !std::isfinite(parameters.signed_eye_translation_clip) ||
      !std::isfinite(parameters.convergence_clip_w) ||
      parameters.convergence_clip_w <= 0.0f ||
      parameters.signed_eye_translation_clip == 0.0f) {
    return guest_clip;
  }

  HomogeneousClipPosition shifted = guest_clip;
  shifted.x = guest_clip.x + parameters.signed_eye_translation_clip *
      (guest_clip.w / parameters.convergence_clip_w - 1.0f);
  if (!std::isfinite(shifted.x)) return guest_clip;
  return shifted;
}

}  // namespace sbs
}  // namespace venom

#endif  // VENOM_SBS746_STEREO_CLIP_OFFSET_H_
