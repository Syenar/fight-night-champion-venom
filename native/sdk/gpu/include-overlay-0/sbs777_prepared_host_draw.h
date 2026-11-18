#ifndef VENOM_SBS777_PREPARED_HOST_DRAW_H_
#define VENOM_SBS777_PREPARED_HOST_DRAW_H_

#include <cstdint>

namespace venom {
namespace sbs {

template <class CommandList>
inline void EmitPreparedHostDraw777(CommandList& command_list,
                                    std::uint32_t vertex_count,
                                    bool indexed) {
  if (indexed) {
    command_list.D3DDrawIndexedInstanced(vertex_count, 1, 0, 0, 0);
  } else {
    command_list.D3DDrawInstanced(vertex_count, 1, 0, 0);
  }
}

}  // namespace sbs
}  // namespace venom

#endif  // VENOM_SBS777_PREPARED_HOST_DRAW_H_