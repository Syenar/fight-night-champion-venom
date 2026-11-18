#ifndef VENOM_SBS762_CONSTANTS_WRITER_H_
#define VENOM_SBS762_CONSTANTS_WRITER_H_

#include "sbs746_stereo_clip_offset.h"

#include <array>
#include <cstring>
#include <type_traits>

namespace venom {
namespace sbs {

// Writes only the 16-byte stereo payload at the tail of a SystemConstants-like
// destination. The destination remains templated so this research helper does
// not impose or include a particular translator ABI.
template <typename Destination>
inline bool WriteStereoConstants762(
    Destination& destination,
    const StereoClipOffsetParameters& parameters,
    bool qualified_perspective_pass = false) noexcept {
  static_assert(std::is_same<decltype(destination.venom_stereo_clip_offset),
                             float[2]>::value,
                "destination clip offset must be float[2]");
  static_assert(std::is_same<decltype(destination.venom_stereo_control),
                             std::uint32_t[2]>::value,
                "destination stereo control must be uint32_t[2]");
  static_assert(sizeof(float) == 4 && sizeof(std::uint32_t) == 4,
                "stereo payload fields must be 32-bit values");
  static_assert(sizeof(destination.venom_stereo_clip_offset) == 8);
  static_assert(sizeof(destination.venom_stereo_control) == 8);

  std::array<unsigned char, 16> previous{};
  std::array<unsigned char, 16> canonical{};
  std::memcpy(previous.data(), destination.venom_stereo_clip_offset, 8);
  std::memcpy(previous.data() + 8, destination.venom_stereo_control, 8);

  if (qualified_perspective_pass && parameters.enabled == 1 &&
      parameters.reserved == 0 &&
      std::isfinite(parameters.signed_eye_translation_clip) &&
      std::isfinite(parameters.convergence_clip_w) &&
      parameters.convergence_clip_w > 0.0f &&
      parameters.signed_eye_translation_clip != 0.0f) {
    const std::uint32_t enabled = 1;
    const std::uint32_t reserved = 0;
    std::memcpy(canonical.data(), &parameters.signed_eye_translation_clip, 4);
    std::memcpy(canonical.data() + 4, &parameters.convergence_clip_w, 4);
    std::memcpy(canonical.data() + 8, &enabled, 4);
    std::memcpy(canonical.data() + 12, &reserved, 4);
  }

  const bool changed = std::memcmp(previous.data(), canonical.data(), 16) != 0;
  std::memcpy(destination.venom_stereo_clip_offset, canonical.data(), 8);
  std::memcpy(destination.venom_stereo_control, canonical.data() + 8, 8);
  return changed;
}

}  // namespace sbs
}  // namespace venom

#endif  // VENOM_SBS762_CONSTANTS_WRITER_H_
