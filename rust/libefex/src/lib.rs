use std::ffi::{c_char, c_int, CStr};
use std::str;

use libefex_sys::*;
use thiserror::Error;

/// Success error code
const EFEX_ERR_SUCCESS: i32 = 0;

/// Maximum size of EFEX code xfer
const EFEX_CODE_MAX_SIZE: usize = 64 * 1024;

/// Error type definition
#[derive(Error, Debug)]
pub enum EfexError {
    /// Invalid parameter
    #[error("Invalid parameter")]
    InvalidParam,
    /// Null pointer error
    #[error("Null pointer error")]
    NullPtr,
    /// Memory allocation error
    #[error("Memory allocation error")]
    Memory,
    /// Operation not supported
    #[error("Operation not supported")]
    NotSupported,
    /// USB initialization failed
    #[error("USB initialization failed")]
    UsbInit,
    /// Device not found
    #[error("Device not found")]
    UsbDeviceNotFound,
    /// Failed to open device
    #[error("Failed to open device")]
    UsbOpen,
    /// USB transfer failed
    #[error("USB transfer failed")]
    UsbTransfer,
    /// USB transfer timeout
    #[error("USB transfer timeout")]
    UsbTimeout,
    /// Protocol error
    #[error("Protocol error")]
    Protocol,
    /// Invalid response from device
    #[error("Invalid response from device")]
    InvalidResponse,
    /// Unexpected status code
    #[error("Unexpected status code")]
    UnexpectedStatus,
    /// Invalid device state
    #[error("Invalid device state")]
    InvalidState,
    /// Invalid device mode
    #[error("Invalid device mode")]
    InvalidDeviceMode,
    /// Operation failed
    #[error("Operation failed")]
    OperationFailed,
    /// Device is busy
    #[error("Device is busy")]
    DeviceBusy,
    /// Device not ready
    #[error("Device not ready")]
    DeviceNotReady,
    /// Flash access error
    #[error("Flash access error")]
    FlashAccess,
    /// Flash size probing failed
    #[error("Flash size probing failed")]
    FlashSizeProbe,
    /// Failed to set flash on/off
    #[error("Failed to set flash on/off")]
    FlashSetOnOff,
    /// Verification failed
    #[error("Verification failed")]
    Verification,
    /// CRC mismatch error
    #[error("CRC mismatch error")]
    CrcMismatch,
    /// Failed to open file
    #[error("Failed to open file")]
    FileOpen,
    /// Failed to read file
    #[error("Failed to read file")]
    FileRead,
    /// Failed to write file
    #[error("Failed to write file")]
    FileWrite,
    /// File size error
    #[error("File size error")]
    FileSize,
    /// Unknown error
    #[error("Unknown error: {0}")]
    Unknown(i32),
}

/// Convert C error code to Rust error type
fn c_error_to_rust(error_code: i32) -> EfexError {
    if error_code == EFEX_ERR_SUCCESS {
        unreachable!("Success code should not be converted to error");
    }

    match error_code {
        -1 => EfexError::InvalidParam,       // EFEX_ERR_INVALID_PARAM
        -2 => EfexError::NullPtr,            // EFEX_ERR_NULL_PTR
        -3 => EfexError::Memory,             // EFEX_ERR_MEMORY
        -4 => EfexError::NotSupported,       // EFEX_ERR_NOT_SUPPORT
        -10 => EfexError::UsbInit,           // EFEX_ERR_USB_INIT
        -11 => EfexError::UsbDeviceNotFound, // EFEX_ERR_USB_DEVICE_NOT_FOUND
        -12 => EfexError::UsbOpen,           // EFEX_ERR_USB_OPEN
        -13 => EfexError::UsbTransfer,       // EFEX_ERR_USB_TRANSFER
        -14 => EfexError::UsbTimeout,        // EFEX_ERR_USB_TIMEOUT
        -20 => EfexError::Protocol,          // EFEX_ERR_PROTOCOL
        -21 => EfexError::InvalidResponse,   // EFEX_ERR_INVALID_RESPONSE
        -22 => EfexError::UnexpectedStatus,  // EFEX_ERR_UNEXPECTED_STATUS
        -30 => EfexError::InvalidState,      // EFEX_ERR_INVALID_STATE
        -31 => EfexError::InvalidDeviceMode, // EFEX_ERR_INVALID_DEVICE_MODE
        -32 => EfexError::OperationFailed,   // EFEX_ERR_OPERATION_FAILED
        -33 => EfexError::DeviceBusy,        // EFEX_ERR_DEVICE_BUSY
        -34 => EfexError::DeviceNotReady,    // EFEX_ERR_DEVICE_NOT_READY
        -40 => EfexError::FlashAccess,       // EFEX_ERR_FLASH_ACCESS
        -41 => EfexError::FlashSizeProbe,    // EFEX_ERR_FLASH_SIZE_PROBE
        -42 => EfexError::FlashSetOnOff,     // EFEX_ERR_FLASH_SET_ONOFF
        -50 => EfexError::Verification,      // EFEX_ERR_VERIFICATION
        -51 => EfexError::CrcMismatch,       // EFEX_ERR_CRC_MISMATCH
        -60 => EfexError::FileOpen,          // EFEX_ERR_FILE_OPEN
        -61 => EfexError::FileRead,          // EFEX_ERR_FILE_READ
        -62 => EfexError::FileWrite,         // EFEX_ERR_FILE_WRITE
        -63 => EfexError::FileSize,          // EFEX_ERR_FILE_SIZE
        _ => EfexError::Unknown(error_code),
    }
}

/// Device mode enumeration
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum DeviceMode {
    /// Null mode
    Null,
    /// FEL mode
    Fel,
    /// SRV mode
    Srv,
    /// Cool update mode
    UpdateCool,
    /// Hot update mode
    UpdateHot,
    /// Unknown mode
    Unknown(u16),
}

/// Convert C device mode to Rust device mode
fn c_mode_to_rust(mode: sunxi_verify_device_mode_t) -> DeviceMode {
    match mode {
        sunxi_verify_device_mode_t::DEVICE_MODE_NULL => DeviceMode::Null,
        sunxi_verify_device_mode_t::DEVICE_MODE_FEL => DeviceMode::Fel,
        sunxi_verify_device_mode_t::DEVICE_MODE_SRV => DeviceMode::Srv,
        sunxi_verify_device_mode_t::DEVICE_MODE_UPDATE_COOL => DeviceMode::UpdateCool,
        sunxi_verify_device_mode_t::DEVICE_MODE_UPDATE_HOT => DeviceMode::UpdateHot,
    }
}

/// EFEX context structure
pub struct Context {
    ctx: sunxi_efex_ctx_t,
    initialized: bool,
}

impl Context {
    /// Create a new context
    pub fn new() -> Self {
        Context {
            ctx: unsafe { std::mem::zeroed() },
            initialized: false,
        }
    }

    /// Set USB backend type (static method)
    pub fn set_usb_backend_static(backend: UsbBackend) -> Result<(), EfexError> {
        let result =
            unsafe { libefex_sys::sunxi_efex_set_usb_backend(rust_usb_backend_to_c(backend)) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(())
    }

    /// Get current USB backend type (static method)
    pub fn get_usb_backend_static() -> UsbBackend {
        let backend = unsafe { libefex_sys::sunxi_efex_get_usb_backend() };
        c_usb_backend_to_rust(backend)
    }

    /// Scan USB devices
    pub fn scan_usb_device(&mut self) -> Result<(), EfexError> {
        let result = unsafe { sunxi_scan_usb_device(&mut self.ctx) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(())
    }

    /// Initialize USB
    pub fn usb_init(&mut self) -> Result<(), EfexError> {
        let result = unsafe { sunxi_usb_init(&mut self.ctx) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(())
    }

    /// Initialize EFEX
    pub fn efex_init(&mut self) -> Result<(), EfexError> {
        let result = unsafe { sunxi_efex_init(&mut self.ctx) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        self.initialized = true;
        Ok(())
    }

    /// Get device mode
    pub fn get_device_mode(&self) -> DeviceMode {
        let mode = unsafe { sunxi_efex_get_device_mode(&self.ctx) };
        c_mode_to_rust(mode)
    }

    /// Get device mode string
    pub fn get_device_mode_str(&self) -> &str {
        let c_str = unsafe { sunxi_efex_get_device_mode_str(&self.ctx) };
        unsafe { CStr::from_ptr(c_str) }
            .to_str()
            .unwrap_or("Unknown")
    }

    /// Get error message
    pub fn strerror(error: &EfexError) -> String {
        match error {
            EfexError::Unknown(code) => {
                let c_str = unsafe { sunxi_efex_strerror(*code) };
                unsafe { CStr::from_ptr(c_str) }
                    .to_str()
                    .unwrap_or("Unknown error")
                    .to_string()
            }
            _ => format!("{:?}", error),
        }
    }

    /// Get internal C context pointer
    pub fn as_ptr(&self) -> *const sunxi_efex_ctx_t {
        &self.ctx as *const _
    }

    /// Get internal mutable C context pointer
    pub fn as_mut_ptr(&mut self) -> *mut sunxi_efex_ctx_t {
        &mut self.ctx as *mut _
    }

    // FEL related methods

    /// Execute command at specified address
    pub fn fel_exec(&self, addr: u32) -> Result<(), EfexError> {
        let result = unsafe { sunxi_efex_fel_exec(self.as_ptr(), addr) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(())
    }

    /// Read memory from specified address
    pub fn fel_read(&self, addr: u32, buf: &mut [u8]) -> Result<(), EfexError> {
        let result = unsafe {
            sunxi_efex_fel_read(
                self.as_ptr(),
                addr,
                buf.as_mut_ptr() as *mut c_char,
                buf.len() as c_int,
            )
        };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(())
    }

    /// Write memory to specified address
    pub fn fel_write(&self, addr: u32, buf: &[u8]) -> Result<(), EfexError> {
        let result = unsafe {
            sunxi_efex_fel_write(
                self.as_ptr(),
                addr,
                buf.as_ptr() as *const c_char,
                buf.len() as c_int,
            )
        };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(())
    }

    // FES related methods

    /// Query storage device type
    pub fn fes_query_storage(&self) -> Result<u32, EfexError> {
        let mut storage_type: u32 = 0;
        let result = unsafe { sunxi_efex_fes_query_storage(self.as_ptr(), &mut storage_type) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(storage_type)
    }

    /// Query secure mode type
    pub fn fes_query_secure(&self) -> Result<u32, EfexError> {
        let mut secure_type: u32 = 0;
        let result = unsafe { sunxi_efex_fes_query_secure(self.as_ptr(), &mut secure_type) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(secure_type)
    }

    /// Probe flash size
    pub fn fes_probe_flash_size(&self) -> Result<u32, EfexError> {
        let mut flash_size: u32 = 0;
        let result = unsafe { sunxi_efex_fes_probe_flash_size(self.as_ptr(), &mut flash_size) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(flash_size)
    }

    /// Set flash on/off status
    pub fn fes_flash_set_onoff(&self, storage_type: u32, on_off: bool) -> Result<(), EfexError> {
        let result = unsafe {
            sunxi_efex_fes_flash_set_onoff(self.as_ptr(), &storage_type, if on_off { 1 } else { 0 })
        };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(())
    }

    /// Get chip ID
    pub fn fes_get_chipid(&self) -> Result<String, EfexError> {
        let mut chip_id: Vec<u8> = vec![0; 64];
        let result = unsafe {
            sunxi_efex_fes_get_chipid(self.as_ptr(), chip_id.as_mut_ptr() as *mut c_char)
        };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        let c_str = unsafe { CStr::from_ptr(chip_id.as_ptr() as *const c_char) };
        Ok(c_str.to_str().unwrap_or("").to_string())
    }

    /// Download data to device
    pub fn fes_down(&self, buf: &[u8], addr: u32, data_type: FesDataType) -> Result<(), EfexError> {
        let result = unsafe {
            sunxi_efex_fes_down(
                self.as_ptr(),
                buf.as_ptr() as *const c_char,
                buf.len() as c_int,
                addr,
                rust_fes_data_type_to_c(data_type),
            )
        };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(())
    }

    /// Download data to device with progress callback
    /// This is an optimized version that reuses the context for multiple transfers
    pub fn fes_down_batch<F>(
        &self,
        buf: &[u8],
        addr: u32,
        data_type: FesDataType,
        mut progress_callback: F,
    ) -> Result<(), EfexError>
    where
        F: FnMut(usize, usize),
    {
        const EFEX_CODE_MAX_SIZE: usize = 64 * 1024;

        if buf.is_empty() {
            return Err(EfexError::InvalidParam);
        }

        let total_len = buf.len();
        let mut remain_data = total_len;
        let mut buff_ptr = buf.as_ptr();
        let mut addr_cur = addr;
        let mut written: usize = 0;

        let data_type_raw = rust_fes_data_type_to_c(data_type) as u32;
        let is_data_type = data_type_raw
            & (libefex_sys::sunxi_fes_data_type_t::SUNXI_EFEX_DATA_TYPE_MASK as u32)
            != 0;

        while remain_data > 0 {
            let length = if remain_data > EFEX_CODE_MAX_SIZE {
                EFEX_CODE_MAX_SIZE
            } else {
                remain_data
            };
            remain_data -= length;

            let mut current_flags = data_type_raw;

            if remain_data == 0 {
                current_flags |=
                    libefex_sys::sunxi_fes_data_type_t::SUNXI_EFEX_TRANS_FINISH_TAG as u32;
            }

            let trans = libefex_sys::sunxi_fes_trans_t {
                addr: addr_cur,
                len: length as u32,
                flags: current_flags,
            };

            let result = unsafe {
                libefex_sys::sunxi_usb_fes_xfer(
                    self.as_ptr(),
                    libefex_sys::sunxi_usb_fes_xfer_type_t::FES_XFER_SEND,
                    libefex_sys::sunxi_efex_cmd_t::EFEX_CMD_FES_DOWN as u32,
                    &trans as *const _ as *const c_char,
                    std::mem::size_of::<libefex_sys::sunxi_fes_trans_t>() as isize,
                    buff_ptr as *const c_char,
                    length as isize,
                )
            };

            if result != EFEX_ERR_SUCCESS {
                return Err(c_error_to_rust(result));
            }

            addr_cur += if is_data_type {
                length as u32
            } else {
                (length / 512) as u32
            };
            buff_ptr = unsafe { buff_ptr.add(length) };
            written += length;

            progress_callback(written, total_len);
        }

        Ok(())
    }

    /// Upload data from device
    pub fn fes_up(
        &self,
        buf: &mut [u8],
        addr: u32,
        data_type: FesDataType,
    ) -> Result<(), EfexError> {
        let result = unsafe {
            sunxi_efex_fes_up(
                self.as_ptr(),
                buf.as_mut_ptr() as *const c_char,
                buf.len() as c_int,
                addr,
                rust_fes_data_type_to_c(data_type),
            )
        };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(())
    }

    /// Verify value
    pub fn fes_verify_value(&self, addr: u32, size: u64) -> Result<FesVerifyResp, EfexError> {
        let resp: sunxi_fes_verify_resp_t = unsafe { std::mem::zeroed() };
        let result = unsafe { sunxi_efex_fes_verify_value(self.as_ptr(), addr, size, &resp) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(c_fes_verify_resp_to_rust(&resp))
    }

    /// Verify status
    pub fn fes_verify_status(&self, tag: u32) -> Result<FesVerifyResp, EfexError> {
        let resp: sunxi_fes_verify_resp_t = unsafe { std::mem::zeroed() };
        let result = unsafe { sunxi_efex_fes_verify_status(self.as_ptr(), tag, &resp) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(c_fes_verify_resp_to_rust(&resp))
    }

    /// Verify U-Boot block
    pub fn fes_verify_uboot_blk(&self, tag: u32) -> Result<FesVerifyResp, EfexError> {
        let resp: sunxi_fes_verify_resp_t = unsafe { std::mem::zeroed() };
        let result = unsafe { sunxi_efex_fes_verify_uboot_blk(self.as_ptr(), tag, &resp) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(c_fes_verify_resp_to_rust(&resp))
    }

    /// Set tool mode
    pub fn fes_tool_mode(
        &self,
        tool_mode: FesToolMode,
        next_mode: FesToolMode,
    ) -> Result<(), EfexError> {
        let result = unsafe {
            sunxi_efex_fes_tool_mode(
                self.as_ptr(),
                rust_fes_tool_mode_to_c(tool_mode),
                rust_fes_tool_mode_to_c(next_mode),
            )
        };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(())
    }

    fn fes_up_down_with_progress<F>(
        &self,
        buf: *mut u8,
        len: usize,
        addr: u32,
        data_type: FesDataType,
        cmd: sunxi_efex_cmd_t,
        mut progress_callback: F,
    ) -> Result<u64, EfexError>
    where
        F: FnMut(u64, u64),
    {
        let total_len = len as u64;
        let mut remain_data = len as u32;
        let mut buff_ptr = buf;
        let mut addr_cur = addr;
        let mut transferred: u64 = 0;

        let base_type = rust_fes_data_type_to_c(data_type) as u32;
        let is_data_type =
            (base_type & sunxi_fes_data_type_t::SUNXI_EFEX_DATA_TYPE_MASK as u32) != 0;
        let mut current_type = base_type;

        let xfer_type = if cmd == sunxi_efex_cmd_t::EFEX_CMD_FES_DOWN {
            sunxi_usb_fes_xfer_type_t::FES_XFER_SEND
        } else {
            sunxi_usb_fes_xfer_type_t::FES_XFER_RECV
        };

        while remain_data > 0 {
            let length = if remain_data > EFEX_CODE_MAX_SIZE as u32 {
                EFEX_CODE_MAX_SIZE as u32
            } else {
                remain_data
            };
            remain_data -= length;

            if remain_data == 0 {
                current_type |= sunxi_fes_data_type_t::SUNXI_EFEX_TRANS_FINISH_TAG as u32;
            }

            let trans = sunxi_fes_trans_t {
                addr: addr_cur,
                len: length,
                flags: current_type,
            };

            let result = unsafe {
                sunxi_usb_fes_xfer(
                    self.as_ptr(),
                    xfer_type,
                    cmd as u32,
                    &trans as *const _ as *const c_char,
                    std::mem::size_of::<sunxi_fes_trans_t>() as isize,
                    buff_ptr as *const c_char,
                    length as isize,
                )
            };

            if result != EFEX_ERR_SUCCESS {
                return Err(c_error_to_rust(result));
            }

            addr_cur += if is_data_type { length } else { length / 512 };
            buff_ptr = unsafe { buff_ptr.add(length as usize) };
            transferred += length as u64;

            progress_callback(transferred, total_len);
        }

        Ok(transferred)
    }

    /// Download data to device with progress callback
    /// Each callback is invoked after every EFEX_CODE_MAX_SIZE (64KB) transfer
    pub fn fes_down_with_progress<F>(
        &self,
        buf: &[u8],
        addr: u32,
        data_type: FesDataType,
        progress_callback: F,
    ) -> Result<u64, EfexError>
    where
        F: FnMut(u64, u64),
    {
        self.fes_up_down_with_progress(
            buf.as_ptr() as *mut u8,
            buf.len(),
            addr,
            data_type,
            sunxi_efex_cmd_t::EFEX_CMD_FES_DOWN,
            progress_callback,
        )
    }

    /// Upload data from device with progress callback
    /// Each callback is invoked after every EFEX_CODE_MAX_SIZE (64KB) transfer
    pub fn fes_up_with_progress<F>(
        &self,
        buf: &mut [u8],
        addr: u32,
        data_type: FesDataType,
        progress_callback: F,
    ) -> Result<u64, EfexError>
    where
        F: FnMut(u64, u64),
    {
        self.fes_up_down_with_progress(
            buf.as_mut_ptr(),
            buf.len(),
            addr,
            data_type,
            sunxi_efex_cmd_t::EFEX_CMD_FES_UP,
            progress_callback,
        )
    }
}

/// Implement Drop trait to automatically release resources
impl Drop for Context {
    fn drop(&mut self) {
        // Exit USB
        unsafe {
            sunxi_usb_exit(&mut self.ctx);
        }
    }
}

/// Architecture type enumeration
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum PayloadArch {
    /// ARM32 architecture
    Arm32,
    /// AArch64 architecture
    Aarch64,
    /// RISC-V architecture
    Riscv,
}

/// USB backend type enumeration
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum UsbBackend {
    /// Auto select (Windows default winusb, Linux libusb)
    Auto,
    /// Force use libusb
    Libusb,
    /// Force use winusb (Windows only)
    Winusb,
}

/// Convert Rust USB backend to C USB backend
fn rust_usb_backend_to_c(backend: UsbBackend) -> libefex_sys::usb_backend_type {
    match backend {
        UsbBackend::Auto => libefex_sys::usb_backend_type::USB_BACKEND_AUTO,
        UsbBackend::Libusb => libefex_sys::usb_backend_type::USB_BACKEND_LIBUSB,
        UsbBackend::Winusb => libefex_sys::usb_backend_type::USB_BACKEND_WINUSB,
    }
}

/// Convert C USB backend to Rust USB backend
fn c_usb_backend_to_rust(backend: libefex_sys::usb_backend_type) -> UsbBackend {
    match backend {
        libefex_sys::usb_backend_type::USB_BACKEND_AUTO => UsbBackend::Auto,
        libefex_sys::usb_backend_type::USB_BACKEND_LIBUSB => UsbBackend::Libusb,
        libefex_sys::usb_backend_type::USB_BACKEND_WINUSB => UsbBackend::Winusb,
    }
}

/// FES data type enumeration
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum FesDataType {
    /// No tag
    None,
    /// DRAM configuration data tag
    Dram,
    /// MBR partition table tag
    Mbr,
    /// BOOT1 tag
    Boot1,
    /// BOOT0 tag
    Boot0,
    /// Erase command tag
    Erase,
    /// Full image size tag
    FullImgSize,
    /// EXT4/UBIFS file system tag
    Ext4Ubifs,
    /// FLASH operation tag
    Flash,
}

/// Convert Rust FES data type to C FES data type
fn rust_fes_data_type_to_c(data_type: FesDataType) -> sunxi_fes_data_type_t {
    match data_type {
        FesDataType::None => sunxi_fes_data_type_t::SUNXI_EFEX_TAG_NONE,
        FesDataType::Dram => sunxi_fes_data_type_t::SUNXI_EFEX_DRAM_TAG,
        FesDataType::Mbr => sunxi_fes_data_type_t::SUNXI_EFEX_MBR_TAG,
        FesDataType::Boot1 => sunxi_fes_data_type_t::SUNXI_EFEX_BOOT1_TAG,
        FesDataType::Boot0 => sunxi_fes_data_type_t::SUNXI_EFEX_BOOT0_TAG,
        FesDataType::Erase => sunxi_fes_data_type_t::SUNXI_EFEX_ERASE_TAG,
        FesDataType::FullImgSize => sunxi_fes_data_type_t::SUNXI_EFEX_FULLIMG_SIZE_TAG,
        FesDataType::Ext4Ubifs => sunxi_fes_data_type_t::SUNXI_EFEX_EXT4_UBIFS_TAG,
        FesDataType::Flash => sunxi_fes_data_type_t::SUNXI_EFEX_FLASH_TAG,
    }
}

/// FES tool mode enumeration
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum FesToolMode {
    /// Normal mode
    Normal,
    /// Reboot mode
    Reboot,
    /// Power off mode
    PowerOff,
    /// Re-update mode
    Reupdate,
    /// Boot mode
    Boot,
}

/// Convert Rust FES tool mode to C FES tool mode
fn rust_fes_tool_mode_to_c(tool_mode: FesToolMode) -> sunxi_fes_tool_mode_t {
    match tool_mode {
        FesToolMode::Normal => sunxi_fes_tool_mode_t::TOOL_MODE_NORMAL,
        FesToolMode::Reboot => sunxi_fes_tool_mode_t::TOOL_MODE_REBOOT,
        FesToolMode::PowerOff => sunxi_fes_tool_mode_t::TOOL_MODE_POWEROFF,
        FesToolMode::Reupdate => sunxi_fes_tool_mode_t::TOOL_MODE_REUPDATE,
        FesToolMode::Boot => sunxi_fes_tool_mode_t::TOOL_MODE_BOOT,
    }
}

/// FES verify response structure
#[derive(Debug, Clone)]
pub struct FesVerifyResp {
    pub flag: u32,
    pub fes_crc: i32,
    pub media_crc: i32,
}

/// Convert C FES verify response to Rust
fn c_fes_verify_resp_to_rust(resp: &sunxi_fes_verify_resp_t) -> FesVerifyResp {
    FesVerifyResp {
        flag: resp.flag,
        fes_crc: resp.fes_crc,
        media_crc: resp.media_crc,
    }
}

/// Convert Rust architecture to C architecture
fn rust_arch_to_c(arch: PayloadArch) -> sunxi_efex_fel_payloads_arch {
    match arch {
        PayloadArch::Arm32 => sunxi_efex_fel_payloads_arch::ARCH_ARM32,
        PayloadArch::Aarch64 => sunxi_efex_fel_payloads_arch::ARCH_AARCH64,
        PayloadArch::Riscv => sunxi_efex_fel_payloads_arch::ARCH_RISCV,
    }
}

/// Payloads related functions
pub mod payloads {
    use super::*;

    /// Initialize payloads
    pub fn init(arch: PayloadArch) -> Result<(), EfexError> {
        let result = unsafe { sunxi_efex_fel_payloads_init(rust_arch_to_c(arch)) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(())
    }

    /// Read 32-bit value
    pub fn readl(ctx: &Context, addr: u32) -> Result<u32, EfexError> {
        let mut val: u32 = 0;
        let result = unsafe { sunxi_efex_fel_payloads_readl(ctx.as_ptr(), addr, &mut val) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(val)
    }

    /// Write 32-bit value
    pub fn writel(ctx: &Context, val: u32, addr: u32) -> Result<(), EfexError> {
        let result = unsafe { sunxi_efex_fel_payloads_writel(ctx.as_ptr(), val, addr) };
        if result != EFEX_ERR_SUCCESS {
            return Err(c_error_to_rust(result));
        }
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_context_creation() {
        // Test context creation
        let ctx = Context::new();
        assert!(!ctx.initialized);
    }

    #[test]
    fn test_error_conversion() {
        // Test error code conversion
        assert!(matches!(c_error_to_rust(-1), EfexError::InvalidParam)); // EFEX_ERR_INVALID_PARAM
        assert!(matches!(c_error_to_rust(-11), EfexError::UsbDeviceNotFound)); // EFEX_ERR_USB_DEVICE_NOT_FOUND
    }

    #[test]
    fn test_mode_conversion() {
        // Test device mode conversion
        assert_eq!(
            c_mode_to_rust(sunxi_verify_device_mode_t::DEVICE_MODE_FEL),
            DeviceMode::Fel
        );
        assert_eq!(
            c_mode_to_rust(sunxi_verify_device_mode_t::DEVICE_MODE_SRV),
            DeviceMode::Srv
        );
    }

    #[test]
    fn test_arch_conversion() {
        // Test architecture conversion
        assert_eq!(
            rust_arch_to_c(PayloadArch::Arm32),
            sunxi_efex_fel_payloads_arch::ARCH_ARM32
        );
        assert_eq!(
            rust_arch_to_c(PayloadArch::Aarch64),
            sunxi_efex_fel_payloads_arch::ARCH_AARCH64
        );
        assert_eq!(
            rust_arch_to_c(PayloadArch::Riscv),
            sunxi_efex_fel_payloads_arch::ARCH_RISCV
        );
    }
}
