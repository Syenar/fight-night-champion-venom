#pragma once

#include "sbs762_constants_writer.h"

#include <array>
#include <cstdint>
#include <cstring>

namespace venom::sbs {

// Apply the reviewed 762 writer and invalidate the system cbuffer only when
// its 16-byte stereo payload actually changes.
template <typename Constants>
inline bool SetStereoConstants769(Constants& constants, bool& cbuffer_up_to_date,
                                 const StereoClipOffsetParameters& parameters,
                                 bool qualified_perspective_pass = false) noexcept {
  const bool changed = WriteStereoConstants762(constants, parameters,
                                               qualified_perspective_pass);
  if (changed) {
    cbuffer_up_to_date = false;
  }
  return changed;
}

// Temporarily bind one eye's constants and restore only the two host-owned
// tail arrays when the scope exits, including exception unwinding.
template <typename Constants>
class ScopedStereoConstants769 final {
 public:
  ScopedStereoConstants769(Constants& constants, bool& cbuffer_up_to_date,
                           const StereoClipOffsetParameters& parameters,
                           bool qualified_perspective_pass = false) noexcept
      : constants_(constants), cbuffer_up_to_date_(cbuffer_up_to_date) {
    std::memcpy(previous_.data(), constants_.venom_stereo_clip_offset, 8);
    std::memcpy(previous_.data() + 8, constants_.venom_stereo_control, 8);
    SetStereoConstants769(constants_, cbuffer_up_to_date_, parameters,
                          qualified_perspective_pass);
  }

  ~ScopedStereoConstants769() noexcept {
    if (std::memcmp(previous_.data(), constants_.venom_stereo_clip_offset, 8) != 0 ||
        std::memcmp(previous_.data() + 8, constants_.venom_stereo_control, 8) != 0) {
      std::memcpy(constants_.venom_stereo_clip_offset, previous_.data(), 8);
      std::memcpy(constants_.venom_stereo_control, previous_.data() + 8, 8);
      cbuffer_up_to_date_ = false;
    }
  }

  ScopedStereoConstants769(const ScopedStereoConstants769&) = delete;
  ScopedStereoConstants769& operator=(const ScopedStereoConstants769&) = delete;
  ScopedStereoConstants769(ScopedStereoConstants769&&) = delete;
  ScopedStereoConstants769& operator=(ScopedStereoConstants769&&) = delete;

 private:
  Constants& constants_;
  bool& cbuffer_up_to_date_;
  std::array<std::uint8_t, 16> previous_{};
};

}  // namespace venom::sbs
