#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace fnc::runtime {

enum class ContractStatus : std::uint8_t {
  Ready,
  Unsupported,
  Failed,
};

struct ContractResult {
  ContractStatus status;
  std::string detail;
  explicit operator bool() const { return status == ContractStatus::Ready; }
};

struct RuntimeConfig {
  std::filesystem::path writable_root;
  std::uint64_t guest_memory_size = 0x1'0000'0000ull;
  std::uint32_t guest_base = 0x82000000u;
};

class Runtime {
public:
  explicit Runtime(RuntimeConfig config);

  ContractResult initialize();
  ContractResult tick(std::uint64_t guest_ticks);
  ContractResult shutdown();

  ContractResult kernel_ready() const;
  ContractResult memory_ready() const;
  ContractResult timebase_ready() const;
  ContractResult threads_ready() const;
  ContractResult filesystem_ready() const;
  ContractResult input_ready() const;
  ContractResult audio_ready() const;
  ContractResult video_ready() const;
  ContractResult graphics_ready() const;
  ContractResult saves_ready() const;
  ContractResult dlc_ready() const;

  std::uint64_t guest_ticks() const { return guest_ticks_; }
  const std::vector<std::string>& events() const { return events_; }

private:
  RuntimeConfig config_;
  std::uint64_t guest_ticks_ = 0;
  bool initialized_ = false;
  bool stopped_ = false;
  std::vector<std::string> events_;
};

} // namespace fnc::runtime
