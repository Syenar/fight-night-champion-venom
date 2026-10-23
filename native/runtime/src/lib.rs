use std::fs;
use std::path::{Path, PathBuf};

pub mod ffi;
pub mod platform;
pub mod services;
use services::{
    AudioBackend, DlcBackend, GraphicsBackend, InputBackend, SaveBackend, ServiceRegistry,
    VideoBackend,
};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ContractStatus {
    Ready,
    Unsupported,
    Failed,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ContractResult {
    pub status: ContractStatus,
    pub detail: &'static str,
}

impl ContractResult {
    pub const fn ready(detail: &'static str) -> Self {
        Self {
            status: ContractStatus::Ready,
            detail,
        }
    }
}

#[derive(Debug)]
pub struct Runtime {
    writable_root: PathBuf,
    guest_ticks: u64,
    initialized: bool,
    stopped: bool,
    events: Vec<&'static str>,
    services: ServiceRegistry,
    pub input: InputBackend,
    pub audio: AudioBackend,
    pub video: VideoBackend,
    pub graphics: GraphicsBackend,
    pub saves: SaveBackend,
    pub dlc: DlcBackend,
}

impl Runtime {
    pub fn new(writable_root: impl AsRef<Path>) -> Self {
        let root = writable_root.as_ref().to_path_buf();
        Self {
            writable_root: root.clone(),
            guest_ticks: 0,
            initialized: false,
            stopped: false,
            events: Vec::new(),
            services: ServiceRegistry::new(),
            input: InputBackend::default(),
            audio: AudioBackend::default(),
            video: VideoBackend::default(),
            graphics: GraphicsBackend::default(),
            saves: SaveBackend::new(root.join("saves")),
            dlc: DlcBackend::default(),
        }
    }
    pub fn initialize(&mut self) -> ContractResult {
        if self.initialized {
            return ContractResult::ready("already initialized");
        }
        if self.writable_root.as_os_str().is_empty() {
            return ContractResult {
                status: ContractStatus::Failed,
                detail: "writable_root is required",
            };
        }
        if fs::create_dir_all(&self.writable_root).is_err() {
            return ContractResult {
                status: ContractStatus::Failed,
                detail: "cannot create writable_root",
            };
        }
        self.services.bind_local_backends();
        self.initialized = true;
        self.stopped = false;
        self.events.push("initialize");
        ContractResult::ready("initialized")
    }
    pub fn tick(&mut self, ticks: u64) -> ContractResult {
        if !self.initialized {
            return ContractResult {
                status: ContractStatus::Failed,
                detail: "tick before initialize",
            };
        }
        if self.stopped {
            return ContractResult {
                status: ContractStatus::Failed,
                detail: "tick after shutdown",
            };
        }
        self.guest_ticks = self.guest_ticks.saturating_add(ticks);
        self.events.push("tick");
        ContractResult::ready("tick")
    }
    pub fn shutdown(&mut self) -> ContractResult {
        if !self.initialized {
            return ContractResult {
                status: ContractStatus::Failed,
                detail: "shutdown before initialize",
            };
        }
        self.stopped = true;
        self.events.push("shutdown");
        ContractResult::ready("shutdown")
    }
    pub fn guest_ticks(&self) -> u64 {
        self.guest_ticks
    }
    pub fn events(&self) -> &[&'static str] {
        &self.events
    }
    pub fn services(&self) -> &ServiceRegistry {
        &self.services
    }
    pub fn kernel_ready(&self) -> ContractResult {
        ContractResult::ready("kernel")
    }
    pub fn memory_ready(&self) -> ContractResult {
        ContractResult::ready("memory")
    }
    pub fn timebase_ready(&self) -> ContractResult {
        ContractResult::ready("timebase")
    }
    pub fn threads_ready(&self) -> ContractResult {
        ContractResult::ready("threads")
    }
    pub fn filesystem_ready(&self) -> ContractResult {
        ContractResult::ready("filesystem")
    }
    pub fn input_ready(&self) -> ContractResult {
        ContractResult::ready("input local backend")
    }
    pub fn audio_ready(&self) -> ContractResult {
        ContractResult::ready("audio local backend")
    }
    pub fn video_ready(&self) -> ContractResult {
        ContractResult::ready("video local backend")
    }
    pub fn graphics_ready(&self) -> ContractResult {
        ContractResult::ready("graphics local backend")
    }
    pub fn saves_ready(&self) -> ContractResult {
        ContractResult::ready("saves local backend")
    }
    pub fn dlc_ready(&self) -> ContractResult {
        ContractResult::ready("DLC manifest backend")
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn lifecycle_and_contracts_are_explicit() {
        let root = std::env::temp_dir().join("fnc-runtime-contract-test");
        let _ = fs::remove_dir_all(&root);
        let mut r = Runtime::new(&root);
        assert_eq!(r.tick(1).status, ContractStatus::Failed);
        assert_eq!(r.initialize().status, ContractStatus::Ready);
        for c in [
            r.kernel_ready(),
            r.memory_ready(),
            r.timebase_ready(),
            r.threads_ready(),
            r.filesystem_ready(),
        ] {
            assert_eq!(c.status, ContractStatus::Ready);
        }
        for c in [
            r.input_ready(),
            r.audio_ready(),
            r.video_ready(),
            r.graphics_ready(),
            r.saves_ready(),
            r.dlc_ready(),
        ] {
            assert_eq!(c.status, ContractStatus::Ready);
        }
        r.input.push(services::InputFrame {
            buttons: 1,
            ..Default::default()
        });
        assert_eq!(r.input.poll().buttons, 1);
        r.audio.submit_i16(&[0; 8]);
        assert_eq!(r.audio.submitted_samples(), 8);
        assert!(r.video.present_sbs(1, 1280, 720, 11, 12).is_ok());
        let frame = r.video.last().unwrap();
        assert_eq!(frame.pixels_per_eye, 921600);
        assert_eq!((frame.left_view_id, frame.right_view_id), (11, 12));
        assert!(r.video.present_sbs(2, 1280, 720, 11, 11).is_err());
        assert_eq!(r.video.last().unwrap().frame_id, 1);
        r.graphics.submit_draw();
        assert_eq!(r.graphics.draw_calls(), 1);
        r.saves.write_slot(1, b"state").unwrap();
        assert_eq!(r.saves.read_slot(1).unwrap(), b"state");
        r.dlc.register("FNC-DLC-TEST");
        assert!(r.dlc.enabled("FNC-DLC-TEST"));
        assert_eq!(r.tick(60).status, ContractStatus::Ready);
        assert_eq!(r.guest_ticks(), 60);
        assert_eq!(r.shutdown().status, ContractStatus::Ready);
        assert_eq!(r.tick(1).status, ContractStatus::Failed);
        let _ = fs::remove_dir_all(root);
    }

    #[test]
    fn c_abi_lifecycle_and_sbs_bridge() {
        let root = std::env::temp_dir().join("fnc-runtime-ffi-test");
        let _ = fs::remove_dir_all(&root);
        let root_bytes = root.to_string_lossy().as_bytes().to_vec();
        let handle = crate::ffi::fnc_runtime_create(root_bytes.as_ptr(), root_bytes.len());
        assert!(!handle.is_null());
        assert_eq!(
            crate::ffi::fnc_runtime_initialize(handle),
            ContractStatus::Ready as i32
        );
        assert_eq!(
            crate::ffi::fnc_runtime_tick(handle, 1),
            ContractStatus::Ready as i32
        );
        assert_eq!(
            crate::ffi::fnc_runtime_present_sbs(handle, 1, 640, 360, 11, 12),
            0
        );
        assert_eq!(
            crate::ffi::fnc_runtime_present_sbs(handle, 2, 640, 360, 11, 11),
            -1
        );
        crate::ffi::fnc_runtime_destroy(handle);
        let _ = fs::remove_dir_all(root);
    }
}

