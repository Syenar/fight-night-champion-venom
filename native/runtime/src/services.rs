use std::collections::{BTreeMap, VecDeque};
use std::fs;
use std::path::{Path, PathBuf};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ServiceState {
    Unbound,
    Bound,
}

#[derive(Debug, Default)]
pub struct ServiceRegistry {
    states: BTreeMap<&'static str, ServiceState>,
}

impl ServiceRegistry {
    pub fn new() -> Self {
        Self::default()
    }
    pub fn bind(&mut self, name: &'static str) {
        self.states.insert(name, ServiceState::Bound);
    }
    pub fn state(&self, name: &'static str) -> ServiceState {
        self.states
            .get(name)
            .copied()
            .unwrap_or(ServiceState::Unbound)
    }
    pub fn bind_local_backends(&mut self) {
        for name in Self::required() {
            self.bind(name);
        }
    }
    pub fn required() -> [&'static str; 11] {
        [
            "kernel",
            "memory",
            "timebase",
            "threads",
            "filesystem",
            "input",
            "audio",
            "video",
            "graphics",
            "saves",
            "dlc",
        ]
    }
}

#[derive(Debug, Clone, Copy, Default, PartialEq, Eq)]
pub struct InputFrame {
    pub buttons: u16,
    pub left_x: i8,
    pub left_y: i8,
    pub right_x: i8,
    pub right_y: i8,
}
#[derive(Debug, Default)]
pub struct InputBackend {
    frames: VecDeque<InputFrame>,
}
impl InputBackend {
    pub fn push(&mut self, frame: InputFrame) {
        self.frames.push_back(frame);
    }
    pub fn poll(&mut self) -> InputFrame {
        self.frames.pop_front().unwrap_or_default()
    }
}

#[derive(Debug, Default)]
pub struct AudioBackend {
    submitted_samples: u64,
}
impl AudioBackend {
    pub fn submit_i16(&mut self, samples: &[i16]) {
        self.submitted_samples = self.submitted_samples.saturating_add(samples.len() as u64);
    }
    pub fn submitted_samples(&self) -> u64 {
        self.submitted_samples
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct VideoFrame {
    pub width: u32,
    pub height: u32,
    pub left_pixels: usize,
    pub right_pixels: usize,
}
#[derive(Debug, Default)]
pub struct VideoBackend {
    last: Option<VideoFrame>,
}
impl VideoBackend {
    pub fn present_sbs(&mut self, width: u32, height: u32) -> Result<(), &'static str> {
        if width == 0 || height == 0 {
            return Err("video dimensions must be non-zero");
        }
        let pixels = (width as usize).saturating_mul(height as usize);
        self.last = Some(VideoFrame {
            width,
            height,
            left_pixels: pixels,
            right_pixels: pixels,
        });
        Ok(())
    }
    pub fn last(&self) -> Option<VideoFrame> {
        self.last
    }
}

#[derive(Debug, Default)]
pub struct GraphicsBackend {
    draw_calls: u64,
}
impl GraphicsBackend {
    pub fn submit_draw(&mut self) {
        self.draw_calls = self.draw_calls.saturating_add(1);
    }
    pub fn draw_calls(&self) -> u64 {
        self.draw_calls
    }
}

#[derive(Debug, Clone)]
pub struct SaveBackend {
    root: PathBuf,
}
impl SaveBackend {
    pub fn new(root: impl AsRef<Path>) -> Self {
        Self {
            root: root.as_ref().to_path_buf(),
        }
    }
    pub fn write_slot(&self, slot: u32, data: &[u8]) -> Result<(), &'static str> {
        fs::create_dir_all(&self.root).map_err(|_| "cannot create save root")?;
        fs::write(self.root.join(format!("slot-{slot:02}.sav")), data)
            .map_err(|_| "cannot write save")
    }
    pub fn read_slot(&self, slot: u32) -> Result<Vec<u8>, &'static str> {
        fs::read(self.root.join(format!("slot-{slot:02}.sav"))).map_err(|_| "cannot read save")
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct DlcEntry {
    pub content_id: String,
    pub enabled: bool,
}
#[derive(Debug, Default)]
pub struct DlcBackend {
    entries: BTreeMap<String, DlcEntry>,
}
impl DlcBackend {
    pub fn register(&mut self, content_id: impl Into<String>) {
        let id = content_id.into();
        self.entries.insert(
            id.clone(),
            DlcEntry {
                content_id: id,
                enabled: true,
            },
        );
    }
    pub fn enabled(&self, content_id: &str) -> bool {
        self.entries.get(content_id).is_some_and(|e| e.enabled)
    }
    pub fn entries(&self) -> impl Iterator<Item = &DlcEntry> {
        self.entries.values()
    }
}
