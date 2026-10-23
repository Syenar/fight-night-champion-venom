use crate::services::InputFrame;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PlatformStatus {
    Ready,
    Unavailable,
}

#[cfg(windows)]
pub fn poll_xinput(player: u32) -> Result<InputFrame, &'static str> {
    #[repr(C)]
    struct State {
        packet: u32,
        gamepad: Gamepad,
    }
    #[repr(C)]
    struct Gamepad {
        buttons: u16,
        left_trigger: u8,
        right_trigger: u8,
        left_x: i16,
        left_y: i16,
        right_x: i16,
        right_y: i16,
    }
    #[link(name = "kernel32")]
    extern "system" {
        fn LoadLibraryA(name: *const u8) -> *mut core::ffi::c_void;
        fn GetProcAddress(
            module: *mut core::ffi::c_void,
            name: *const u8,
        ) -> *mut core::ffi::c_void;
    }
    type GetState = unsafe extern "system" fn(u32, *mut State) -> u32;
    let module = unsafe { LoadLibraryA(c"xinput1_4.dll".as_ptr().cast()) };
    if module.is_null() {
        return Err("XInput runtime DLL is unavailable");
    }
    let address = unsafe { GetProcAddress(module, c"XInputGetState".as_ptr().cast()) };
    if address.is_null() {
        return Err("XInputGetState is unavailable");
    }
    let get_state: GetState = unsafe { std::mem::transmute(address) };
    let mut state = State {
        packet: 0,
        gamepad: Gamepad {
            buttons: 0,
            left_trigger: 0,
            right_trigger: 0,
            left_x: 0,
            left_y: 0,
            right_x: 0,
            right_y: 0,
        },
    };
    let code = unsafe { get_state(player, &mut state) };
    if code != 0 {
        return Err("XInput controller is unavailable");
    }
    Ok(InputFrame {
        buttons: state.gamepad.buttons,
        left_x: (state.gamepad.left_x / 256) as i8,
        left_y: (state.gamepad.left_y / 256) as i8,
        right_x: (state.gamepad.right_x / 256) as i8,
        right_y: (state.gamepad.right_y / 256) as i8,
    })
}

#[cfg(not(windows))]
pub fn poll_xinput(_player: u32) -> Result<InputFrame, &'static str> {
    Err("XInput is only available on Windows")
}

