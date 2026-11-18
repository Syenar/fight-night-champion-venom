// Game-specific function overrides for Fight Night Champion (default.xex v0.0.0.3 + title update 2 (v0.0.2.3),
// media 2DC50868). Addresses are only valid for that executable.
//
// Recompiled functions are weak symbols; defining one here replaces it, and
// __imp__<name> is always the original.

#include <atomic>
#include <cstring>
#include <chrono>
#include <filesystem>
#include <memory>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc.h>
#include <rex/filesystem/devices/host_path_device.h>
#include <rex/filesystem/vfs.h>
#include <rex/runtime.h>

REXCVAR_DEFINE_DOUBLE(fnc_frame_rate, 60.0, "Game",
                      "Frame rate cap. The retail game runs at 30. Set 30 to restore it.");

// ---------------------------------------------------------------------------
// 60 FPS.
// sub_827C9378 builds one of the game's fixed-rate frame timers; f1 = rate in Hz,
// which a helper turns into a tick period (timebase / rate). The game makes
// exactly two, each once:
//   sub_822A9540 -> 30 Hz (constant 0x82008574), returns to 0x822A9564  <- changed
//   sub_822AA530 -> 60 Hz, returns to 0x822AA558  <- untouched
// The community "Unlock FPS" patch for TU2 (illusion, xenia-canary/game-patches)
// raises the same 30.0 constant (it sits at 0x82008574 in TU2).
// ---------------------------------------------------------------------------
// Debug menu (community "Debug Menu" patch for TU2, Adrian, xenia-canary/game-patches): the
// UI's native method "IsFinal" at 0x82218884 is renamed "IsDebug", which turns on the
// developer menu (hold left on the left stick and click it). Applied once at startup, from
// the frame timer hook below.
REXCVAR_DEFINE_BOOL(fnc_debug_menu, false, "Game",
                    "Enable the game's developer menu (hold left on the left stick and click it)");

extern "C" void __imp__sub_827C9378(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void sub_827C9378(PPCContext& __restrict ctx, uint8_t* base) {
  if (ctx.lr == 0x822A9564 && REXCVAR_GET(fnc_debug_menu)) {
    char* name = reinterpret_cast<char*>(base + 0x82218884);
    if (std::memcmp(name, "IsFinal", 8) == 0) {
      // The string sits in read-only image data.
      DWORD old_protect;
      if (VirtualProtect(name, 8, PAGE_READWRITE, &old_protect)) {
        std::memcpy(name, "IsDebug", 8);
        VirtualProtect(name, 8, old_protect, &old_protect);
        REXLOG_INFO("[FNC-DEBUG] developer menu enabled (fnc_debug_menu)");
      } else {
        REXLOG_WARN("[FNC-DEBUG] could not make the IsFinal string writable");
      }
    }
  }
  if (ctx.lr == 0x822A9564 && ctx.f1.f64 == 30.0) {
    double rate = REXCVAR_GET(fnc_frame_rate);
    if (rate >= 10.0 && rate <= 240.0 && rate != 30.0) {
      REXLOG_INFO("[FNC-60FPS] frame timer 30 Hz -> {:.0f} Hz (fnc_frame_rate)", rate);
      ctx.f1.f64 = rate;
    } else {
      REXLOG_INFO("[FNC-60FPS] frame timer left at 30 Hz (fnc_frame_rate = {})", rate);
    }
  }
  __imp__sub_827C9378(ctx, base);
}

// Input manager probe. The title's menu path is expected to poll XamInputGetState
// through sub_82801C40 before dispatching the controller state to the active menu.
// Keep this observation at the title boundary so a missing callback can be
// distinguished from an input-device failure.
extern "C" void __imp__sub_82801C40(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void __imp__sub_82802370(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void __imp__sub_8280D1A8(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void __imp__sub_8280D128(PPCContext& __restrict ctx, uint8_t* base);
REXCVAR_DEFINE_BOOL(venom_force_menu_input, false, "Input/Research",
                    "Opt-in bounded title input-manager callback probe");
REXCVAR_DEFINE_INT32(venom_force_menu_window_seconds, 0, "Input/Research",
                     "Stop title input overrides after this many seconds; 0 uses the full probe")
    .range(0, 300);
REXCVAR_DEFINE_BOOL(venom_log_boxer_growth_tabs, false, "Game/Research",
                    "Log Athleticism and Boosts tab queries without changing their results");
REXCVAR_DEFINE_BOOL(venom_log_menu_input_translation, false, "Input/Research",
                    "Observe title menu input masks and key dispatch without changing them");

extern "C" void __imp__sub_82DABA08(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void __imp__sub_82DAB960(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void __imp__sub_82DF13C0(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void __imp__sub_82DF1910(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void __imp__sub_82D4A480(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void __imp__sub_82836E98(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void __imp__sub_82838888(PPCContext& __restrict ctx, uint8_t* base);

static uint32_t ReadMenuWord(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, sizeof(value));
  return __builtin_bswap32(value);
}

extern "C" void sub_82D4A480(PPCContext& __restrict ctx, uint8_t* base) {
  if (REXCVAR_GET(venom_log_menu_input_translation) && ctx.r3.u32 != 0) {
    const auto input = ctx.r3.u32;
    const auto previous = ReadMenuWord(base, input);
    const auto buttons = ReadMenuWord(base, input + 32);
    const auto directions = ReadMenuWord(base, input + 48);
    if (previous || buttons || directions) {
      REXLOG_INFO("VENOM_APT_POLL prior={:#x} current={:#x} directions={:#x} connected={} enabled={} suppressed={}",
                  previous, buttons, directions, base[input + 52],
                  base[0x8306182F], base[0x8306183E]);
    }
  }
  __imp__sub_82D4A480(ctx, base);
}

extern "C" void sub_82836E98(PPCContext& __restrict ctx, uint8_t* base) {
  if (REXCVAR_GET(venom_log_menu_input_translation) && ctx.r3.u32 != 0) {
    const auto menu = ctx.r3.u32;
    REXLOG_INFO("VENOM_APT_KEY index={} release={} user={} max_user={} mode={} filter={}",
                ctx.r4.u32, ctx.r5.u32, ctx.r6.u32, ReadMenuWord(base, menu + 360),
                base[menu + 380], base[menu + 381]);
  }
  __imp__sub_82836E98(ctx, base);
}

extern "C" void sub_82838888(PPCContext& __restrict ctx, uint8_t* base) {
  if (REXCVAR_GET(venom_log_menu_input_translation)) {
    REXLOG_INFO("VENOM_APT_TRANSLATE index={} release={} user={}",
                ctx.r4.u32, ctx.r5.u32, ctx.r6.u32);
  }
  __imp__sub_82838888(ctx, base);
}

extern "C" void sub_82DABA08(PPCContext& __restrict ctx, uint8_t* base) {
  __imp__sub_82DABA08(ctx, base);
  if (REXCVAR_GET(venom_log_boxer_growth_tabs)) {
    REXLOG_INFO("VENOM_GROWTH_TAB athleticism result={}", ctx.r3.u32 & 0xFF);
  }
}

extern "C" void sub_82DAB960(PPCContext& __restrict ctx, uint8_t* base) {
  const uint32_t category = ctx.r4.u32;
  __imp__sub_82DAB960(ctx, base);
  if (REXCVAR_GET(venom_log_boxer_growth_tabs)) {
    REXLOG_INFO("VENOM_GROWTH_TAB boosts category={} result={}", category, ctx.r3.u32 & 0xFF);
  }
}

extern "C" void sub_82DF13C0(PPCContext& __restrict ctx, uint8_t* base) {
  if (REXCVAR_GET(venom_log_boxer_growth_tabs)) {
    REXLOG_INFO("VENOM_GROWTH_APPLY Athleticism boost bridge entered");
  }
  __imp__sub_82DF13C0(ctx, base);
}

extern "C" void sub_82DF1910(PPCContext& __restrict ctx, uint8_t* base) {
  if (REXCVAR_GET(venom_log_boxer_growth_tabs)) {
    REXLOG_INFO("VENOM_GROWTH_APPLY ConfirmChanges bridge entered");
  }
  __imp__sub_82DF1910(ctx, base);
}
REXCVAR_DEFINE_BOOL(venom_clean_roster_route, false, "Input/Research",
                    "Disable legacy probe pulses and run only the fighter-select route");
REXCVAR_DEFINE_BOOL(venom_bare_knuckles_route, false, "Input/Research",
                    "Run the bounded Game Modes and bout setup confirmation replay");
REXCVAR_DEFINE_BOOL(venom_legacy_route, false, "Input/Research",
                    "Run the bounded replay into Legacy Mode");
REXCVAR_DEFINE_STRING(venom_legacy_route_steps, "", "Input/Research",
                      "Up to 24 Legacy replay steps: a accept, b back, d down, u up, . wait");
REXCVAR_DEFINE_INT32(venom_bout_roster_steps, 0, "Input/Research",
                     "Move up or down in fighter select before confirming the bout")
    .range(-32, 32);
REXCVAR_DEFINE_BOOL(venom_direct_roster_route, false, "Input/Research",
                    "Assume the fresh main menu starts on Fight Now");
static std::atomic<uint32_t> g_forced_input_code{0};
static std::atomic<uint32_t> g_forced_input_value{4};
enum class RouteEvent : uint8_t {
  kNone,
  kAccept,
  kDpad,
  kBack,
  kProbe12,
  kProbe13,
  kUp,
  kProbe15,
  kLateUp,
  kLateFighterDown,
  kLateFighterAccept,
};
static std::atomic<uint32_t> g_route_event_index{0};
static bool MenuReplayActive() {
  if (!REXCVAR_GET(venom_force_menu_input)) return false;
  const int seconds = REXCVAR_GET(venom_force_menu_window_seconds);
  if (seconds == 0) return true;
  static const auto start = std::chrono::steady_clock::now();
  return std::chrono::steady_clock::now() - start < std::chrono::seconds(seconds);
}

extern "C" void fnc_inject_menu_state(PPCContext& __restrict ctx, uint8_t* base) {
  static uint32_t trace_calls = 0;
  if (ctx.r26.u32 == 0 && trace_calls++ < 4) {
    REXLOG_INFO("[VENOM_INPUT_STACK_ENTRY] force={} user={} r1={:#x}",
                REXCVAR_GET(venom_force_menu_input), ctx.r26.u32, ctx.r1.u32);
  }
  if (ctx.r26.u32 != 0 || !MenuReplayActive()) return;
  static std::atomic<uint64_t> user_polls{0};
  const uint64_t poll = user_polls.fetch_add(1, std::memory_order_relaxed);
  const bool clean_route = REXCVAR_GET(venom_clean_roster_route);
  const bool bare_knuckles_route = REXCVAR_GET(venom_bare_knuckles_route);
  const bool legacy_route = REXCVAR_GET(venom_legacy_route);
  // The callback can skip many polls while a menu loads, and multiple callback
  // threads can mature thresholds together. Advance one ordered event at a time.
  struct RoutePulse {
    uint64_t threshold;
    RouteEvent event;
  };
  static constexpr RoutePulse full_route[] = {
      {300, RouteEvent::kAccept}, {720, RouteEvent::kDpad}, {840, RouteEvent::kDpad},
      {960, RouteEvent::kAccept}, {1080, RouteEvent::kBack}, {1200, RouteEvent::kProbe12},
      {1260, RouteEvent::kAccept}, {1320, RouteEvent::kProbe13}, {1380, RouteEvent::kAccept},
      {1440, RouteEvent::kUp}, {1500, RouteEvent::kAccept}, {1560, RouteEvent::kProbe15},
      {1620, RouteEvent::kAccept}, {3000, RouteEvent::kLateUp}, {3120, RouteEvent::kLateUp},
      {3240, RouteEvent::kLateUp}, {3360, RouteEvent::kLateUp}, {3480, RouteEvent::kLateUp},
      {3600, RouteEvent::kLateUp}, {3840, RouteEvent::kLateFighterAccept},
      {4500, RouteEvent::kLateFighterDown}, {4620, RouteEvent::kLateFighterAccept},
  };
  // Confirm the profile, open Game Modes from the main menu, and confirm
  // the first entry after the submenu has loaded. Allow each setup screen
  // to settle before confirming its default selection.
  static const auto bare_knuckles_pulses = [] {
    std::vector<RoutePulse> pulses{
        {300, RouteEvent::kAccept}, {720, RouteEvent::kDpad}, {840, RouteEvent::kDpad},
        {960, RouteEvent::kAccept}, {1140, RouteEvent::kAccept}, {1380, RouteEvent::kAccept},
    };
    const int steps = REXCVAR_GET(venom_bout_roster_steps);
    const auto direction = steps < 0 ? RouteEvent::kUp : RouteEvent::kDpad;
    const auto count = static_cast<uint64_t>(steps < 0 ? -steps : steps);
    for (uint64_t i = 0; i < count; ++i) {
      pulses.push_back({1560 + 120 * i, direction});
    }
    for (uint64_t poll = 1560; poll <= 2280; poll += 180) {
      pulses.push_back({poll + 120 * count, RouteEvent::kAccept});
    }
    return pulses;
  }();
  static const auto legacy_pulses = [] {
    std::vector<RoutePulse> pulses{
        {300, RouteEvent::kAccept}, {720, RouteEvent::kDpad}, {840, RouteEvent::kDpad},
        {960, RouteEvent::kAccept}, {1200, RouteEvent::kDpad}, {1380, RouteEvent::kAccept},
    };
    const auto steps = REXCVAR_GET(venom_legacy_route_steps);
    uint64_t threshold = 1740;
    for (size_t i = 0; i < steps.size() && i < 24; ++i, threshold += 240) {
      switch (steps[i]) {
        case 'a': pulses.push_back({threshold, RouteEvent::kAccept}); break;
        case 'b': pulses.push_back({threshold, RouteEvent::kBack}); break;
        case 'd': pulses.push_back({threshold, RouteEvent::kDpad}); break;
        case 'u': pulses.push_back({threshold, RouteEvent::kUp}); break;
      }
    }
    return pulses;
  }();
  const RoutePulse* pulses = full_route;
  size_t route_count = std::size(full_route);
  if (legacy_route) {
    pulses = legacy_pulses.data();
    route_count = legacy_pulses.size();
  } else if (bare_knuckles_route) {
    pulses = bare_knuckles_pulses.data();
    route_count = bare_knuckles_pulses.size();
  }
  RouteEvent route_event = RouteEvent::kNone;
  uint32_t route_index = g_route_event_index.load(std::memory_order_relaxed);
  if (clean_route && !bare_knuckles_route && !legacy_route) {
    if (route_index == 0 && poll >= 300 &&
        g_route_event_index.compare_exchange_strong(route_index, 1,
                                                    std::memory_order_relaxed)) {
      route_event = RouteEvent::kAccept;
    }
  } else if (route_index < route_count && poll >= pulses[route_index].threshold) {
    const RouteEvent next_event = pulses[route_index].event;
    if (g_route_event_index.compare_exchange_strong(route_index, route_index + 1,
                                                    std::memory_order_relaxed)) {
      route_event = next_event;
    }
  }
  const bool back = route_event == RouteEvent::kBack;
  const bool dpad = route_event == RouteEvent::kDpad;
  const bool probe_code_12 = route_event == RouteEvent::kProbe12;
  const bool probe_code_13 = route_event == RouteEvent::kProbe13;
  const bool probe_code_14 = route_event == RouteEvent::kUp;
  const bool probe_code_15 = route_event == RouteEvent::kProbe15;
  const bool up = probe_code_14 || route_event == RouteEvent::kLateUp;
  const bool direction_probe = probe_code_12 || probe_code_13 || probe_code_14 || probe_code_15;
  const bool late_fight_now = false;
  const bool late_roster_down = route_event == RouteEvent::kLateFighterDown;
  const bool late_menu_up = route_event == RouteEvent::kLateUp;
  const bool late_menu_accept = false;
  const bool late_fighter_down = route_event == RouteEvent::kLateFighterDown;
  const bool late_fighter_accept = route_event == RouteEvent::kLateFighterAccept;
  const bool main_menu_fight_now = false;
  const bool fighter_roster_down = false;
  const bool accept = route_event == RouteEvent::kAccept || late_fighter_accept;
  if (!back && !up && !dpad && !accept && !direction_probe && !late_fight_now && !late_roster_down &&
      !main_menu_fight_now && !fighter_roster_down && !late_menu_up && !late_menu_accept &&
      !late_fighter_down && !late_fighter_accept) return;
  const uint32_t button_mask = back ? 0x2000u :
    (up || late_menu_up ? 0x0001u : ((dpad || late_roster_down || direction_probe || fighter_roster_down || late_fighter_down) ? 0x0002u : 0x1000u));
  const uint32_t probe_code = probe_code_12 ? 12u : (probe_code_13 ? 13u : (probe_code_14 ? 14u : (probe_code_15 ? 15u : 4u)));
  g_forced_input_code.store(back ? 5u : (late_menu_up ? 14u : ((dpad || late_roster_down || fighter_roster_down || late_fighter_down) ? 13u : probe_code)), std::memory_order_relaxed);
  g_forced_input_value.store(back ? 5u : 4u, std::memory_order_relaxed);
  *reinterpret_cast<volatile uint16_t*>(base + ctx.r1.u32 + 100) =
      __builtin_bswap16(static_cast<uint16_t>(button_mask));
  *reinterpret_cast<volatile uint16_t*>(base + ctx.r1.u32 + 212) = 0;
  *reinterpret_cast<volatile uint8_t*>(base + ctx.r31.u32 + 176) = 0;
  REXLOG_INFO("[VENOM_INPUT_STACK] kind={} user=0 buttons1={:#x} buttons2=0",
              back ? "back" : ((up || late_menu_up) ? "up" : ((dpad || late_roster_down || fighter_roster_down || late_fighter_down) ? "dpad" :
                (direction_probe ? "probe" : "accept"))), button_mask);
}
extern "C" void fnc_apply_forced_menu_event(PPCContext& __restrict ctx, uint8_t* base) {
  const uint32_t code = g_forced_input_code.exchange(0, std::memory_order_relaxed);
  const uint32_t value = g_forced_input_value.exchange(4, std::memory_order_relaxed);
  if (code == 0 || !MenuReplayActive()) return;
  *reinterpret_cast<volatile uint32_t*>(base + ctx.r1.u32 + 112) = __builtin_bswap32(1u);
  *reinterpret_cast<volatile uint32_t*>(base + ctx.r1.u32 + 116) = __builtin_bswap32(value);
  *reinterpret_cast<volatile uint32_t*>(base + ctx.r1.u32 + 120) = __builtin_bswap32(code);
  REXLOG_INFO("[VENOM_MENU_EVENT_FORCE] code={} value={}", code, value);
}
extern "C" void sub_82801C40(PPCContext& __restrict ctx, uint8_t* base) {
  const uint32_t object = ctx.r3.u32;
  const uint32_t user = ctx.r4.u32;
  static uint32_t polls = 0;
  if ((polls++ & 0x3FFu) == 0) {
    REXLOG_INFO("[VENOM_INPUT_MANAGER] user={} poll={}", user, polls);
  }
  uint8_t saved_active = 0;
  const bool force_active = user == 0 && object != 0 && MenuReplayActive();
  if (force_active) {
    saved_active = *reinterpret_cast<volatile uint8_t*>(base + object + 41);
    REXLOG_INFO("[VENOM_INPUT_ACTIVE] object={:#x} byte41={:#x} -> 0", object, saved_active);
    *reinterpret_cast<volatile uint8_t*>(base + object + 41) = 0;
  }
  __imp__sub_82801C40(ctx, base);
  if (force_active && (polls & 0x3FFu) == 0) {
    REXLOG_INFO("[VENOM_INPUT_RETURN] user={} state={:#x} callback={:#x} buttons1={:#x} buttons2={:#x}",
                user, object + 64, __builtin_bswap32(*reinterpret_cast<volatile uint32_t*>(base + object + 0)),
                __builtin_bswap16(*reinterpret_cast<volatile uint16_t*>(base + object + 64 + 96)),
                __builtin_bswap16(*reinterpret_cast<volatile uint16_t*>(base + object + 64 + 120)));
  }
  if (force_active) {
    *reinterpret_cast<volatile uint8_t*>(base + object + 41) = saved_active;
  }
  if (!force_active) return;
  static const auto first_poll = std::chrono::steady_clock::now();
  const double age = std::chrono::duration<double>(std::chrono::steady_clock::now() - first_poll).count();
  // The title's input poll can begin just after the original half-second probe
  // window. Keep the same single navigation pulse available for one second so
  // at least one title poll observes it without changing the selected action.
  const bool dpad = age >= 30.0 && age < 31.0;
  const bool accept = age >= 33.0 && age < 33.5;
  if (!dpad && !accept) return;
  const uint32_t state_object = object + 64;
  const uint32_t offset = dpad ? 96u : 120u;
  // These fields are the title's digital button masks. The previous probe wrote
  // 1.0f bits here, which logged timing but never asserted a usable XInput button.
  // XInput's canonical masks are D-pad down = 0x0002 and A = 0x1000.
  const uint32_t button_mask = dpad ? 0x0002u : 0x1000u;
  const auto guest_u32 = [&](uint32_t address) {
    return __builtin_bswap32(*reinterpret_cast<volatile uint32_t*>(base + address));
  };
  REXLOG_INFO("[VENOM_INPUT_STATE] kind={} state={:#x} down={:#x} accept={:#x} digital={:#x}",
              dpad ? "dpad" : "accept", state_object, guest_u32(state_object + 96),
              guest_u32(state_object + 120), guest_u32(state_object + 168));
  *reinterpret_cast<volatile uint32_t*>(base + state_object + offset) =
      __builtin_bswap32(button_mask);
  REXLOG_INFO("[VENOM_INPUT_DIRECT] kind={} user={} state={:#x} button_mask={:#x} offset={}",
              dpad ? "dpad" : "accept", user, state_object, button_mask, offset);
}

extern "C" void sub_82802370(PPCContext& __restrict ctx, uint8_t* base) {
  const uint32_t object = ctx.r3.u32;
  const uint32_t update = __builtin_bswap32(*reinterpret_cast<volatile uint32_t*>(base + object + 20));
  const uint32_t update_arg = __builtin_bswap32(*reinterpret_cast<volatile uint32_t*>(base + object + 24));
  REXLOG_INFO("[VENOM_INPUT_UPDATE] object={:#x} callback={:#x} arg={:#x}", object, update, update_arg);
  __imp__sub_82802370(ctx, base);
}

extern "C" void sub_8280D1A8(PPCContext& __restrict ctx, uint8_t* base) {
  REXLOG_INFO("[VENOM_INPUT_REGISTER] r3={:#x} r4={:#x} r5={:#x} r6={:#x}",
              ctx.r3.u32, ctx.r4.u32, ctx.r5.u32, ctx.r6.u32);
  __imp__sub_8280D1A8(ctx, base);
}

extern "C" void sub_8280D128(PPCContext& __restrict ctx, uint8_t* base) {
  REXLOG_INFO("[VENOM_INPUT_SET_CALLBACK] object={:#x} callback={:#x} context={:#x}",
              ctx.r3.u32, ctx.r4.u32, ctx.r5.u32);
  __imp__sub_8280D128(ctx, base);
}











// ---------------------------------------------------------------------------
// cache: drive.
// On a console the game mounts the hard drive's cache partition as cache: (a "utility drive")
// and keeps working files there, including its replay data. The runtime has no such drive (its
// Cache0/Cache1 partitions are placeholders), so the game's cache stayed unavailable and it
// skipped knockdown replays and round highlights. A folder in the save folder is mounted as
// cache: instead, before the game starts (FncApp::OnPreLaunchModule).
REXCVAR_DEFINE_BOOL(fnc_cache_drive, true, "Game",
                    "Give the game a cache: drive (needed for replays); stored in the save folder");

namespace fnc_cache {
void Mount() {
  if (!REXCVAR_GET(fnc_cache_drive)) return;
  auto* runtime = rex::Runtime::instance();
  if (!runtime || !runtime->file_system()) return;
  auto dir = runtime->user_data_root() / "cache_drive";
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  auto device = std::make_unique<rex::filesystem::HostPathDevice>("\\CACHE", dir, false);
  if (!device->Initialize() || !runtime->file_system()->RegisterDevice(std::move(device)) ||
      !runtime->file_system()->RegisterSymbolicLink("cache:", "\\CACHE")) {
    REXLOG_WARN("[FNC-CACHE] could not mount {} as cache:", dir.string());
    return;
  }
  REXLOG_INFO("[FNC-CACHE] cache: is {}", dir.string());
}
}  // namespace fnc_cache

// ---------------------------------------------------------------------------
// Skip the start-up movies.
// At start-up sub_82DBC770 tells the menus to begin the boot flow: "StartFullBootflow" (EA
// logo, intro movie) or, when the game is returning to the menus, "StartQuickBootflow". Both
// go through sub_82837D38(?, scope "_global", event name, args, ...). With fnc_skip_intro the
// full boot flow is replaced by the quick one, the game's own short path (no EA logo).
REXCVAR_DEFINE_BOOL(fnc_skip_intro, false, "Game", "Skip the EA logo and intro movies at start-up");

extern "C" void __imp__sub_82837D38(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void sub_82837D38(PPCContext& __restrict ctx, uint8_t* base) {
  constexpr uint32_t kStartFullBootflow = 0x821FFE60;   // "StartFullBootflow"
  constexpr uint32_t kQuickScope = 0x821FFE90;          // "_global"
  constexpr uint32_t kStartQuickBootflow = 0x821FFE7C;  // "StartQuickBootflow"
  if (ctx.r5.u32 == kStartFullBootflow && REXCVAR_GET(fnc_skip_intro)) {
    REXLOG_INFO("[FNC-INTRO] start-up movies skipped (fnc_skip_intro)");
    ctx.r4.u64 = kQuickScope;
    ctx.r5.u64 = kStartQuickBootflow;
    ctx.r6.u64 = 0;  // the quick boot flow takes no arguments
  }
  __imp__sub_82837D38(ctx, base);
}

// The quick boot flow still plays the Fight Night intro movie (boot screen CUSTOM_FN4): the menu
// asks ION_Media.GetVideoAttributes (implementation sub_82DD8CE0, r4 = video type) for its
// path. Type 3 is that intro (type 0 the EA logo, 1 the attract video). With fnc_skip_intro the
// first type-3 request of the session gets no attributes, so the boot flow finds no movie and
// moves on to the title screen; later requests (watching it from a menu) are answered normally.
extern "C" void __imp__sub_82DD8CE0(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void sub_82DD8CE0(PPCContext& __restrict ctx, uint8_t* base) {
  constexpr uint32_t kIntroVideoType = 3;
  static bool boot_intro_seen = false;
  if (ctx.r4.u32 == kIntroVideoType && !boot_intro_seen) {
    boot_intro_seen = true;
    if (REXCVAR_GET(fnc_skip_intro)) {
      REXLOG_INFO("[FNC-INTRO] intro movie skipped (fnc_skip_intro)");
      return;  // the caller's result holder stays empty: no movie
    }
  }
  __imp__sub_82DD8CE0(ctx, base);
}

