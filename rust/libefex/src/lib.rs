use std::ffi::{c_char, c_int, CStr};
use std::str;

use libefex_sys::*;
use thiserror::Error;

/// Success error code
const EFEX_ERR_SUCCESS: i32 = 0;

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
