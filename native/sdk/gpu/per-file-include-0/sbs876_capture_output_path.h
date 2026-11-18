#pragma once

#include <string>
#include <string_view>

namespace venom::sbs::capture_output876 {

struct CaptureOutputPaths {
  std::wstring stage;
  std::wstring frames;
};

// Accept only one exact physical-D capture stage spelling. This parser is
// portable and performs no filesystem access or directory creation.
inline bool ParseCaptureOutputRoot(std::wstring_view value,
                                   CaptureOutputPaths* output) {
  constexpr std::wstring_view kPrefix =
      L"D:\\Fight Night Champion Venom\\xbox-full-game-sbs\\research\\active-work\\";
  constexpr std::wstring_view kStagePrefix = L"cap";
  constexpr std::wstring_view kSuffix = L"-gameplay-eye-pair";
  constexpr std::size_t kMaximumLength = 200;
  if (output == nullptr || value.size() > kMaximumLength ||
      value.size() <= kPrefix.size() || value.substr(0, kPrefix.size()) != kPrefix) {
    return false;
  }

  const std::wstring_view leaf = value.substr(kPrefix.size());
  if (leaf.size() <= kStagePrefix.size() + kSuffix.size() ||
      leaf.substr(0, kStagePrefix.size()) != kStagePrefix ||
      leaf.substr(leaf.size() - kSuffix.size()) != kSuffix) {
    return false;
  }
  const std::wstring_view digits = leaf.substr(
      kStagePrefix.size(), leaf.size() - kStagePrefix.size() - kSuffix.size());
  if (digits.size() < 3 || digits.size() > 6) return false;
  for (wchar_t digit : digits) {
    if (digit < L'0' || digit > L'9') return false;
  }

  output->stage.assign(value);
  output->frames.assign(value);
  output->frames.append(L"\\frames\\");
  return true;
}

}  // namespace venom::sbs::capture_output876
