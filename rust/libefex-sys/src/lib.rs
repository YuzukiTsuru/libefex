#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(dead_code)]

use libc::{c_char, c_int, c_void, size_t};

// Error code enumeration
#[repr(C)]
pub enum sunxi_efex_error_t {
    EFEX_ERR_SUCCESS = 0,        // Success
    EFEX_ERR_INVALID_PARAM = -1, // Invalid parameter
    EFEX_ERR_NULL_PTR = -2,      // Null pointer error
    EFEX_ERR_MEMORY = -3,        // Memory allocation error
    EFEX_ERR_NOT_SUPPORT = -4,   // Operation not supported

    // USB Communication Errors
    EFEX_ERR_USB_INIT = -10,             // USB initialization failed
    EFEX_ERR_USB_DEVICE_NOT_FOUND = -11, // Device not found
    EFEX_ERR_USB_OPEN = -12,             // Failed to open device
    EFEX_ERR_USB_TRANSFER = -13,         // USB transfer failed
    EFEX_ERR_USB_TIMEOUT = -14,          // USB transfer timeout

    // Protocol Errors
    EFEX_ERR_PROTOCOL = -20,            // Protocol error
    EFEX_ERR_INVALID_RESPONSE = -21,    // Invalid response from device
    EFEX_ERR_UNEXPECTED_STATUS = -22,   // Unexpected status code
    EFEX_ERR_INVALID_STATE = -23,       // Invalid device state
    EFEX_ERR_INVALID_DEVICE_MODE = -24, // Invalid device mode

    // Operation Errors
    EFEX_ERR_OPERATION_FAILED = -30, // Operation failed
    EFEX_ERR_DEVICE_BUSY = -31,      // Device is busy
    EFEX_ERR_DEVICE_NOT_READY = -32, // Device not ready

    // Flash Related Errors
    EFEX_ERR_FLASH_ACCESS = -40,     // Flash access error
    EFEX_ERR_FLASH_SIZE_PROBE = -41, // Flash size probing failed
    EFEX_ERR_FLASH_SET_ONOFF = -42,  // Failed to set flash on/off

    // Verification Errors
    EFEX_ERR_VERIFICATION = -50, // Verification failed
    EFEX_ERR_CRC_MISMATCH = -51, // CRC mismatch error

    // File Operation Errors
    EFEX_ERR_FILE_OPEN = -60,  // Failed to open file
    EFEX_ERR_FILE_READ = -61,  // Failed to read file
    EFEX_ERR_FILE_WRITE = -62, // Failed to write file
    EFEX_ERR_FILE_SIZE = -63,  // File size error
}

// Command type enumeration
#[repr(C)]
#[derive(PartialEq, Debug, Copy, Clone)]
pub enum sunxi_efex_cmd_t {
    // Common Commands
    EFEX_CMD_VERIFY_DEVICE = 0x0001,
    EFEX_CMD_SWITCH_ROLE = 0x0002,
    EFEX_CMD_IS_READY = 0x0003,
    EFEX_CMD_GET_CMD_SET_VER = 0x0004,
    EFEX_CMD_DISCONNECT = 0x0010,
    // FEL Commands
    EFEX_CMD_FEL_WRITE = 0x0101,
    EFEX_CMD_FEL_EXEC = 0x0102,
    EFEX_CMD_FEL_READ = 0x0103,
    // FES Commands
    EFEX_CMD_FES_TRANS = 0x0201,
    EFEX_CMD_FES_RUN = 0x0202,
    EFEX_CMD_FES_INFO = 0x0203,
    EFEX_CMD_FES_GET_MSG = 0x0204,
    EFEX_CMD_FES_UNREG_FED = 0x0205,
    EFEX_CMD_FES_DOWN = 0x0206,
    EFEX_CMD_FES_UP = 0x0207,
    EFEX_CMD_FES_VERIFY = 0x0208,
    EFEX_CMD_FES_QUERY_STORAGE = 0x0209,
    EFEX_CMD_FES_FLASH_SET_ON = 0x020A,
    EFEX_CMD_FES_FLASH_SET_OFF = 0x020B,
    EFEX_CMD_FES_VERIFY_VALUE = 0x020C,
    EFEX_CMD_FES_VERIFY_STATUS = 0x020D,
    EFEX_CMD_FES_FLASH_SIZE_PROBE = 0x020E,
    EFEX_CMD_FES_TOOL_MODE = 0x020F,
    EFEX_CMD_FES_VERIFY_UBOOT_BLK = 0x0214,
    EFEX_CMD_FES_FORCE_ERASE_FLASH = 0x0220,
    EFEX_CMD_FES_FORCE_ERASE_KEY = 0x0221,
    EFEX_CMD_FES_QUERY_SECURE = 0x0230,
    EFEX_CMD_FES_QUERY_INFO = 0x0231,
    EFEX_CMD_FES_GET_CHIPID = 0x0232,
}

// Device mode enumeration
#[repr(C)]
pub enum sunxi_verify_device_mode_t {
    DEVICE_MODE_NULL = 0x0,
    DEVICE_MODE_FEL = 0x1,
    DEVICE_MODE_SRV = 0x2,
    DEVICE_MODE_UPDATE_COOL = 0x3,
    DEVICE_MODE_UPDATE_HOT = 0x4,
}

// FES tool mode enumeration
#[repr(C)]
pub enum sunxi_fes_tool_mode_t {
    TOOL_MODE_NORMAL = 0x1,
    TOOL_MODE_REBOOT = 0x2,
    TOOL_MODE_POWEROFF = 0x3,
    TOOL_MODE_REUPDATE = 0x4,
    TOOL_MODE_BOOT = 0x5,
}

// USB request structure
#[repr(C)]
pub struct sunxi_usb_request_t {
    pub magic: [c_char; 4],
    pub tab: u32,
    pub data_length: u32,
    pub resvered1: u16,
    pub resvered2: u8,
    pub cmd_length: u8,
    pub cmd_package: [u8; 16],
}

// USB response structure
#[repr(C)]
pub struct sunxi_usb_response_t {
    pub magic: [c_char; 4],
    pub tag: u32,
    pub residue: u32,
    pub status: u8,
}

// EFEX request structure
#[repr(C)]
pub struct sunxi_efex_request_t {
    pub cmd: u16,
    pub tag: u16,
    pub address: u32,
    pub len: u32,
    pub flags: u32,
}

// EFEX response structure
#[repr(C)]
pub struct sunxi_efex_response_t {
    pub magic: u16,
    pub tag: u16,
    pub status: u8,
    pub reserve: [u8; 3],
}

// FES reserved structure
#[repr(C)]
pub struct sunxi_fes_reserved_t {
    pub reserved: [*mut c_char; 12],
}

// FES Flash structure
#[repr(C)]
pub struct sunxi_fes_flash_t {
    pub flash_type: u32,
    pub reserved: [*mut c_char; 8],
}

// FES transfer structure
#[repr(C)]
pub struct sunxi_fes_trans_t {
    pub addr: u32,
    pub len: u32,
    pub flags: u32,
}

// FES verify value structure
#[repr(C)]
pub struct sunxi_fes_verify_value_t {
    pub addr: u32,
    pub size: u64,
}

// FES verify status structure
#[repr(C)]
pub struct sunxi_fes_verify_status_t {
    pub addr: u32,
    pub size: u32,
    pub tag: u32,
}

// FES verify response structure
#[repr(C)]
pub struct sunxi_fes_verify_resp_t {
    pub flag: u32,
    pub fes_crc: i32,
    pub media_crc: i32,
}

// FES set tool mode structure
#[repr(C)]
pub struct sunxi_fes_set_tool_mode_t {
    pub tool_mode: u32,
    pub next_mode: u32,
    pub reserved: u32,
}

// FES transfer structure
#[repr(C)]
pub struct sunxi_fes_xfer_t {
    pub cmd: u16,
    pub tag: u16,
    pub buf: [c_char; 12],
    pub magic: [c_char; 4],
}

// EFEX device response structure
#[repr(C)]
pub struct sunxi_efex_device_resp_t {
    pub magic: [c_char; 8],
    pub id: u32,
    pub firmware: u32,
    pub mode: u16,
    pub data_flag: u8,
    pub data_length: u8,
    pub data_start_address: u32,
    pub reserved: [u8; 8],
}

// EFEX context structure
#[repr(C)]
pub struct sunxi_efex_ctx_t {
    pub hdl: *mut c_void,
    pub usb_context: *mut c_void,
    pub dev_name: *mut c_char,
    pub epout: c_int,
    pub epin: c_int,
    pub resp: sunxi_efex_device_resp_t,
}

// USB request type enumeration
#[repr(C)]
pub enum sunxi_efex_usb_request_t {
    AW_USB_READ = 0x11,
    AW_USB_WRITE = 0x12,
}

// USB FES transfer type enumeration
#[repr(C)]
#[derive(Copy, Clone)]
pub enum sunxi_usb_fes_xfer_type_t {
    FES_XFER_SEND = 0x0,
    FES_XFER_RECV = 0x1,
    FES_XFER_NONE = 0x2,
}

// FES data type enumeration
#[repr(C)]
pub enum sunxi_fes_data_type_t {
    SUNXI_EFEX_TAG_NONE = 0x0, // No tag
    // Data type tag
    SUNXI_EFEX_DRAM_TAG = 0x7f00,         // DRAM configuration data tag
    SUNXI_EFEX_MBR_TAG = 0x7f01,          // MBR partition table tag
    SUNXI_EFEX_BOOT1_TAG = 0x7f02,        // BOOT1 tag
    SUNXI_EFEX_BOOT0_TAG = 0x7f03,        // BOOT0 tag
    SUNXI_EFEX_ERASE_TAG = 0x7f04,        // Erase command tag
    SUNXI_EFEX_FULLIMG_SIZE_TAG = 0x7f10, // Full image size tag
    SUNXI_EFEX_EXT4_UBIFS_TAG = 0x7ff0,   // EXT4/UBIFS file system tag
    SUNXI_EFEX_FLASH_TAG = 0x8000,        // FLASH operation tag
    // Data type mask
    SUNXI_EFEX_DATA_TYPE_MASK = 0x7fff, // Data type mask

    // Transfer tag
    SUNXI_EFEX_TRANS_START_TAG = 0x20000,  // Transfer start tag
    SUNXI_EFEX_TRANS_FINISH_TAG = 0x10000, // Transfer finish tag

    // Transfer mask
    SUNXI_EFEX_TRANS_MASK = 0x30000, // Transfer control mask
}

// Payloads architecture enumeration
#[repr(C)]
#[derive(PartialEq, Debug)]
pub enum sunxi_efex_fel_payloads_arch {
    ARCH_ARM32 = 0,
    ARCH_AARCH64 = 1,
    ARCH_RISCV = 2,
}

// USB backend type enumeration
#[repr(C)]
#[derive(PartialEq, Debug, Copy, Clone)]
pub enum usb_backend_type {
    USB_BACKEND_AUTO = 0,
    USB_BACKEND_LIBUSB = 1,
    USB_BACKEND_WINUSB = 2,
}

// Declare C functions
extern "C" {
    // Common functions
    pub fn sunxi_send_efex_request(
        ctx: *const sunxi_efex_ctx_t,
        typ: sunxi_efex_cmd_t,
        addr: u32,
        length: u32,
    ) -> c_int;

    pub fn sunxi_read_efex_status(ctx: *const sunxi_efex_ctx_t) -> c_int;

    pub fn sunxi_scan_usb_device(ctx: *mut sunxi_efex_ctx_t) -> c_int;

    pub fn sunxi_efex_get_device_mode(ctx: *const sunxi_efex_ctx_t) -> sunxi_verify_device_mode_t;

    pub fn sunxi_efex_get_device_mode_str(ctx: *const sunxi_efex_ctx_t) -> *const c_char;

    pub fn sunxi_efex_init(ctx: *mut sunxi_efex_ctx_t) -> c_int;

    pub fn sunxi_efex_strerror(error_code: c_int) -> *const c_char;

    // USB =====
    pub fn sunxi_usb_bulk_send(
        handle: *mut c_void,
        ep: c_int,
        buf: *const c_char,
        len: c_int,
    ) -> c_int;

    pub fn sunxi_usb_bulk_recv(
        handle: *mut c_void,
        ep: c_int,
        buf: *mut c_char,
        len: c_int,
    ) -> c_int;

    pub fn sunxi_send_usb_request(
        ctx: *const sunxi_efex_ctx_t,
        typ: sunxi_efex_usb_request_t,
        length: size_t,
    ) -> c_int;

    pub fn sunxi_read_usb_response(ctx: *const sunxi_efex_ctx_t) -> c_int;

    pub fn sunxi_usb_write(ctx: *const sunxi_efex_ctx_t, buf: *const c_void, len: size_t) -> c_int;

    pub fn sunxi_usb_read(ctx: *const sunxi_efex_ctx_t, data: *const c_void, len: size_t) -> c_int;

    pub fn sunxi_usb_init(ctx: *mut sunxi_efex_ctx_t) -> c_int;

    pub fn sunxi_usb_exit(ctx: *mut sunxi_efex_ctx_t) -> c_int;

    pub fn sunxi_usb_fes_xfer(
        ctx: *const sunxi_efex_ctx_t,
        typ: sunxi_usb_fes_xfer_type_t,
        cmd: u32,
        request_buf: *const c_char,
        request_len: isize,
        buf: *const c_char,
        len: isize,
    ) -> c_int;

    // FEL =====
    pub fn sunxi_efex_fel_exec(ctx: *const sunxi_efex_ctx_t, addr: u32) -> c_int;

    pub fn sunxi_efex_fel_read(
        ctx: *const sunxi_efex_ctx_t,
        addr: u32,
        buf: *mut c_char,
        len: c_int,
    ) -> c_int;

    pub fn sunxi_efex_fel_write(
        ctx: *const sunxi_efex_ctx_t,
        addr: u32,
        buf: *const c_char,
        len: c_int,
    ) -> c_int;

    pub fn sunxi_efex_fel_read_cb(
        ctx: *const sunxi_efex_ctx_t,
        addr: u32,
        buf: *const c_char,
        len: c_int,
        callback: Option<extern "C" fn(c_int)>,
    ) -> c_int;

    pub fn sunxi_efex_fel_write_cb(
        ctx: *const sunxi_efex_ctx_t,
        addr: u32,
        buf: *const c_char,
        len: c_int,
        callback: Option<extern "C" fn(c_int)>,
    ) -> c_int;

    // FES =====
    pub fn sunxi_efex_fes_query_storage(
        ctx: *const sunxi_efex_ctx_t,
        storage_type: *mut u32,
    ) -> c_int;

    pub fn sunxi_efex_fes_query_secure(
        ctx: *const sunxi_efex_ctx_t,
        secure_type: *mut u32,
    ) -> c_int;

    pub fn sunxi_efex_fes_probe_flash_size(
        ctx: *const sunxi_efex_ctx_t,
        flash_size: *mut u32,
    ) -> c_int;

    pub fn sunxi_efex_fes_flash_set_onoff(
        ctx: *const sunxi_efex_ctx_t,
        storage_type: *const u32,
        on_off: u32,
    ) -> c_int;

    pub fn sunxi_efex_fes_get_chipid(ctx: *const sunxi_efex_ctx_t, chip_id: *const c_char)
        -> c_int;

    pub fn sunxi_efex_fes_down(
        ctx: *const sunxi_efex_ctx_t,
        buf: *const c_char,
        len: c_int,
        addr: u32,
        typ: sunxi_fes_data_type_t,
    ) -> c_int;

    pub fn sunxi_efex_fes_up(
        ctx: *const sunxi_efex_ctx_t,
        buf: *const c_char,
        len: c_int,
        addr: u32,
        typ: sunxi_fes_data_type_t,
    ) -> c_int;

    pub fn sunxi_efex_fes_verify_value(
        ctx: *const sunxi_efex_ctx_t,
        addr: u32,
        size: u64,
        buf: *const sunxi_fes_verify_resp_t,
    ) -> c_int;

    pub fn sunxi_efex_fes_verify_status(
        ctx: *const sunxi_efex_ctx_t,
        tag: u32,
        buf: *const sunxi_fes_verify_resp_t,
    ) -> c_int;

    pub fn sunxi_efex_fes_verify_uboot_blk(
        ctx: *const sunxi_efex_ctx_t,
        tag: u32,
        buf: *const sunxi_fes_verify_resp_t,
    ) -> c_int;

    pub fn sunxi_efex_fes_tool_mode(
        ctx: *const sunxi_efex_ctx_t,
        tool_mode: sunxi_fes_tool_mode_t,
        next_mode: sunxi_fes_tool_mode_t,
    ) -> c_int;

    // Payloads =====
    pub fn sunxi_efex_fel_payloads_init(arch: sunxi_efex_fel_payloads_arch) -> c_int;

    pub fn sunxi_efex_fel_payloads_readl(
        ctx: *const sunxi_efex_ctx_t,
        addr: u32,
        val: *mut u32,
    ) -> c_int;

    pub fn sunxi_efex_fel_payloads_writel(
        ctx: *const sunxi_efex_ctx_t,
        val: u32,
        addr: u32,
    ) -> c_int;

    // USB backend functions
    pub fn sunxi_efex_set_usb_backend(backend: usb_backend_type) -> c_int;

    pub fn sunxi_efex_get_usb_backend() -> usb_backend_type;
}
