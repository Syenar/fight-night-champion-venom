use crate::{ContractStatus, Runtime};
use std::ffi::c_void;
use std::path::PathBuf;

#[no_mangle]
pub extern "C" fn fnc_runtime_create(root: *const u8, len: usize) -> *mut c_void {
    if root.is_null() {
        return std::ptr::null_mut();
    }
    let bytes = unsafe { std::slice::from_raw_parts(root, len) };
    let Ok(path) = std::str::from_utf8(bytes) else {
        return std::ptr::null_mut();
    };
    Box::into_raw(Box::new(Runtime::new(PathBuf::from(path)))) as *mut c_void
}

#[no_mangle]
pub extern "C" fn fnc_runtime_initialize(handle: *mut c_void) -> i32 {
    let Some(runtime) = (unsafe { (handle as *mut Runtime).as_mut() }) else {
        return -1;
    };
    runtime.initialize().status as i32
}

#[no_mangle]
pub extern "C" fn fnc_runtime_tick(handle: *mut c_void, ticks: u64) -> i32 {
    let Some(runtime) = (unsafe { (handle as *mut Runtime).as_mut() }) else {
        return -1;
    };
    runtime.tick(ticks).status as i32
}

#[no_mangle]
pub extern "C" fn fnc_runtime_present_sbs(handle: *mut c_void, width: u32, height: u32) -> i32 {
    let Some(runtime) = (unsafe { (handle as *mut Runtime).as_mut() }) else {
        return -1;
    };
    if runtime.video_ready().status != ContractStatus::Ready {
        return -1;
    }
    runtime
        .video
        .present_sbs(width, height)
        .map(|_| 0)
        .unwrap_or(-1)
}

#[no_mangle]
pub extern "C" fn fnc_runtime_destroy(handle: *mut c_void) {
    if !handle.is_null() {
        unsafe {
            drop(Box::from_raw(handle as *mut Runtime));
        }
    }
}
