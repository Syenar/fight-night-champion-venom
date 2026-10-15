#include "fnc_runtime.h"

#include <cassert>
#include <filesystem>

int main() {
  const auto root = std::filesystem::temp_directory_path() / "fnc-runtime-contract-test";
  std::filesystem::remove_all(root);
  fnc::runtime::Runtime runtime({root});

  assert(!runtime.tick(1));
  assert(runtime.initialize());
  assert(runtime.kernel_ready());
  assert(runtime.memory_ready());
  assert(runtime.timebase_ready());
  assert(runtime.threads_ready());
  assert(runtime.filesystem_ready());
  assert(!runtime.input_ready());
  assert(!runtime.audio_ready());
  assert(!runtime.video_ready());
  assert(!runtime.graphics_ready());
  assert(!runtime.saves_ready());
  assert(!runtime.dlc_ready());
  assert(runtime.tick(60));
  assert(runtime.guest_ticks() == 60);
  assert(runtime.shutdown());
  assert(!runtime.tick(1));
  std::filesystem::remove_all(root);
  return 0;
}
