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
    pub frame_id: u64,
    pub width: u32,
    pub height: u32,
    pub left_view_id: u64,
    pub right_view_id: u64,
    pub pixels_per_eye: usize,
}
#[derive(Debug, Default)]
pub struct VideoBackend {
    last: Option<VideoFrame>,
}
impl VideoBackend {
    /// Record a complete SBS pair after both eye views have been produced.
    /// This metadata contract does not render, validate disparity, or composite pixels.
    pub fn present_sbs(
        &mut self,
        frame_id: u64,
        width: u32,
        height: u32,
        left_view_id: u64,
        right_view_id: u64,
    ) -> Result<(), &'static str> {
        if frame_id == 0 {
            return Err("frame id must be non-zero");
        }
        if width == 0 || height == 0 {
            return Err("video dimensions must be non-zero");
        }
        if left_view_id == 0 || right_view_id == 0 {
            return Err("eye view ids must be non-zero");
        }
        if left_view_id == right_view_id {
            return Err("left and right views must be distinct");
        }
        let Some(pixels_per_eye) = (width as usize).checked_mul(height as usize) else {
            return Err("video dimensions overflow addressable pixel count");
        };
        self.last = Some(VideoFrame {
            frame_id,
            width,
            height,
            left_view_id,
            right_view_id,
            pixels_per_eye,
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

