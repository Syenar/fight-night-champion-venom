#include "fnc_runtime.h"

#include <fstream>

namespace fnc::runtime {

namespace {
ContractResult ready(const char* name) { return {ContractStatus::Ready, name}; }
ContractResult failed(const char* name) { return {ContractStatus::Failed, name}; }
}

Runtime::Runtime(RuntimeConfig config) : config_(std::move(config)) {}

ContractResult Runtime::initialize() {
  if (initialized_) return ready("already initialized");
  if (config_.writable_root.empty()) return failed("writable_root is required");
  std::error_code ec;
  std::filesystem::create_directories(config_.writable_root, ec);
  if (ec) return failed("cannot create writable_root");
  initialized_ = true;
  stopped_ = false;
  events_.push_back("initialize");
  return ready("initialized");
}

ContractResult Runtime::tick(std::uint64_t guest_ticks) {
  if (!initialized_) return failed("tick before initialize");
  if (stopped_) return failed("tick after shutdown");
  guest_ticks_ += guest_ticks;
  events_.push_back("tick");
  return ready("tick");
}

ContractResult Runtime::shutdown() {
  if (!initialized_) return failed("shutdown before initialize");
  stopped_ = true;
  events_.push_back("shutdown");
  return ready("shutdown");
}

ContractResult Runtime::kernel_ready() const { return ready("kernel"); }
ContractResult Runtime::memory_ready() const { return ready("memory"); }
ContractResult Runtime::timebase_ready() const { return ready("timebase"); }
ContractResult Runtime::threads_ready() const { return ready("threads"); }
ContractResult Runtime::filesystem_ready() const { return ready("filesystem"); }
ContractResult Runtime::input_ready() const { return {ContractStatus::Unsupported, "input backend not bound"}; }
ContractResult Runtime::audio_ready() const { return {ContractStatus::Unsupported, "audio backend not bound"}; }
ContractResult Runtime::video_ready() const { return {ContractStatus::Unsupported, "video backend not bound"}; }
ContractResult Runtime::graphics_ready() const { return {ContractStatus::Unsupported, "graphics backend not bound"}; }
ContractResult Runtime::saves_ready() const { return {ContractStatus::Unsupported, "save backend not bound"}; }
ContractResult Runtime::dlc_ready() const { return {ContractStatus::Unsupported, "DLC manifest not bound"}; }

} // namespace fnc::runtime
