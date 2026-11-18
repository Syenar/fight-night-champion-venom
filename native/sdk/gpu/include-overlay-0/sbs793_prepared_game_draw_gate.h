#pragma once

#include <cstdint>

namespace venom::sbs {

// Facts must be populated from the current, authoritative draw/pipeline state.
// Defaults deliberately fail closed.
struct PreparedGameDrawFacts793 {
  bool host_render_targets = false;
  bool is_rasterization_done = false;
  bool has_pixel_shader = false;
  bool stencil_disabled = false;
  bool memexport_vertex = true;
  bool memexport_pixel = true;
  bool active_occlusion = true;
  bool tessellated = true;
  std::uint64_t vertex_shader_hash = 0;
  std::uint64_t vertex_modification = 0;
  std::uint32_t draw_count = 0;
  std::uint32_t bound_depth_and_color_bits = 0;
  bool root_signature_valid = false;
  bool original_system_cbv_up_to_date = false;
  bool pipeline_formats_match_snapshot = false;
  std::uint32_t depth_compare = 0;  // Xenos CompareFunction::kNever.
  bool depth_enabled = false;
};

inline bool GetPreparedDepthClear793(const PreparedGameDrawFacts793& facts,
                                     float* clear_value) {
  if (!clear_value) return false;
  if (!facts.depth_enabled || facts.depth_compare == 7) {
    *clear_value = 1.0f;
    return true;
  }
  switch (facts.depth_compare) {
    case 1:  // LESS
    case 3:  // LESS_EQUAL
      *clear_value = 1.0f;
      return true;
    case 4:  // GREATER
    case 6:  // GREATER_EQUAL
      *clear_value = 0.0f;
      return true;
    default:  // NEVER, EQUAL, NOT_EQUAL, or an unknown value.
      return false;
  }
}

inline bool QualifiesPreparedColorDraw(
    const PreparedGameDrawFacts793& facts) {
  float unused_depth_clear = 0.0f;
  return facts.host_render_targets && facts.is_rasterization_done &&
         facts.has_pixel_shader && facts.stencil_disabled &&
         !facts.memexport_vertex && !facts.memexport_pixel &&
         !facts.active_occlusion && !facts.tessellated &&
         facts.draw_count != 0 &&
         facts.bound_depth_and_color_bits == 3 && facts.root_signature_valid &&
         facts.original_system_cbv_up_to_date &&
         facts.pipeline_formats_match_snapshot &&
         GetPreparedDepthClear793(facts, &unused_depth_clear);
}

inline bool QualifiesPreparedGameDraw793(const PreparedGameDrawFacts793& facts) {
  return facts.vertex_shader_hash == UINT64_C(0x1c6d1798cfae936b) &&
         facts.vertex_modification == 7 && QualifiesPreparedColorDraw(facts);
}

}  // namespace venom::sbs
