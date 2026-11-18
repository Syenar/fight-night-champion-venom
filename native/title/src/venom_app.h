#pragma once
#include <rex/rex_app.h>
#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/filesystem/devices/host_path_device.h>
#include <rex/filesystem/vfs.h>
#include <rex/runtime.h>
#include <rex/system/interfaces/graphics.h>
#include <rex/ui/presenter.h>
#include <chrono>
#include <fstream>
#include <windows.h>
#include <renderdoc_app.h>

REXCVAR_DEFINE_STRING(venom_capture_ppm, "", "Venom/Research",
                     "Optional private guest-output capture path, after 15 seconds");
REXCVAR_DEFINE_INT32(venom_capture_delay_seconds, 15, "Venom/Research",
                    "Diagnostic capture delay, 1 to 300 seconds");
REXCVAR_DEFINE_INT32(venom_capture_count, 1, "Venom/Research",
                    "Diagnostic capture count, 1 to 120; interval configured separately");
REXCVAR_DEFINE_INT32(venom_capture_interval_seconds, 30, "Venom/Research",
                    "Diagnostic interval between captures, 1 to 300 seconds");
REXCVAR_DEFINE_STRING(venom_renderdoc_capture, "", "Venom/Research",
                     "Opt-in private RenderDoc capture template; requires injected RenderDoc");
REXCVAR_DEFINE_BOOL(venom_writable_guest_cache, false, "Venom/Research",
                    "Use a writable guest cache in the private user data root");

class VenomFncApp final : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;
  static std::unique_ptr<rex::ui::WindowedApp> Create(rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<VenomFncApp>(new VenomFncApp(ctx, "Fight Night Champion Venom", PPCImageConfig));
  }
 protected:
  void OnPreLaunchModule() override {
    if (!REXCVAR_GET(venom_writable_guest_cache)) return;
    std::error_code ec;
    const auto path = std::filesystem::weakly_canonical(user_data_root() / "venom-guest-cache", ec);
    if (ec || user_data_root().empty()) return;
    // Reject a cache inside either source tree, including through a junction.
    for (const auto& source : {game_data_root(), update_data_root()}) {
      if (source.empty()) continue;
      for (auto parent = path; !parent.empty(); parent = parent.parent_path()) {
        if (std::filesystem::equivalent(parent, source, ec)) {
          REXLOG_ERROR("VENOM_CACHE overlaps source data at {}", source.string());
          return;
        }
        if (parent == parent.root_path()) break;
      }
    }
    std::filesystem::create_directories(path / "replay", ec);
    if (ec) return;
    std::filesystem::create_directories(path / "replay_metadata", ec);
    if (ec) return;
    constexpr std::string_view mount = "\\Device\\VenomGuestCache";
    auto device = std::make_unique<rex::filesystem::HostPathDevice>(mount, path, false);
    auto* fs = runtime()->file_system();
    if (!device->Initialize() || !fs->RegisterDevice(std::move(device))) return;
    fs->UnregisterSymbolicLink("cache:");
    fs->RegisterSymbolicLink("cache:", mount);
    REXLOG_INFO("VENOM_CACHE mounted writable private guest cache at {}", path.string());
  }

  void OnPostSetup() override {
    const auto path = REXCVAR_GET(venom_capture_ppm);
    if (path.empty()) return;
    const int seconds = REXCVAR_GET(venom_capture_delay_seconds);
    const int count = REXCVAR_GET(venom_capture_count);
    const int interval = REXCVAR_GET(venom_capture_interval_seconds);
    if (seconds < 1 || seconds > 300 || count < 1 || count > 120 || interval < 1 || interval > 300) return;
    const auto rdc_path = REXCVAR_GET(venom_renderdoc_capture);
    RENDERDOC_API_1_0_0* rdc = nullptr;
    if (!rdc_path.empty()) {
      auto module = GetModuleHandleW(L"renderdoc.dll");
      auto get_api = module ? reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(module, "RENDERDOC_GetAPI")) : nullptr;
      if (get_api && get_api(eRENDERDOC_API_Version_1_0_0, reinterpret_cast<void**>(&rdc)) && rdc) {
        rdc->SetLogFilePathTemplate(rdc_path.c_str());
        REXLOG_INFO("VENOM_RENDERDOC connected; capture template {}", rdc_path);
      } else {
        rdc = nullptr;
        REXLOG_WARN("VENOM_RENDERDOC unavailable; no graphics frame capture will be claimed");
      }
    }
    capture_ = std::jthread([this, path, seconds, count, interval, rdc](std::stop_token stop) {
      for (int shot = 0; shot < count; ++shot) {
      for (int i = 0; i < (shot ? interval * 10 : seconds * 10) && !stop.stop_requested(); ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if (stop.stop_requested()) return;
      if (rdc) {
        rdc->TriggerCapture();
        REXLOG_INFO("VENOM_RENDERDOC requested next presented frame, shot {}", shot + 1);
      }
      auto output_path = std::filesystem::path(path);
      if (shot) output_path.replace_filename(output_path.stem().string() + "-" +
                                             std::to_string(shot + 1) + output_path.extension().string());
      auto* graphics = runtime()->graphics_system();
      auto* presenter = graphics ? graphics->presenter() : nullptr;
      rex::ui::RawImage image;
      if (!presenter || !presenter->CaptureGuestOutput(image)) {
        REXLOG_WARN("VENOM_CAPTURE failed: no guest output");
        return;
      }
      if (!image.width || !image.height || image.stride < size_t(image.width) * 4 ||
          image.data.size() < size_t(image.height - 1) * image.stride + size_t(image.width) * 4) {
        REXLOG_WARN("VENOM_CAPTURE failed: invalid image layout");
        return;
      }
      std::ofstream out(output_path, std::ios::binary);
      out << "P6\n" << image.width << ' ' << image.height << "\n255\n";
      for (uint32_t y = 0; y < image.height; ++y)
        for (uint32_t x = 0; x < image.width; ++x)
          out.write(reinterpret_cast<const char*>(image.data.data() + y * image.stride + x * 4), 3);
      out.flush();
      if (out) REXLOG_INFO("VENOM_CAPTURE saved {}x{} to {}", image.width, image.height, output_path.string());
      else REXLOG_WARN("VENOM_CAPTURE failed: output write");
      }
    });
  }
  void OnShutdown() override {
    capture_.request_stop();
    if (capture_.joinable()) capture_.join();
  }
 private:
  std::jthread capture_;
};
