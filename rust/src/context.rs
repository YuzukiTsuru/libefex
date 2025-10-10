//! Context struct and related operations

use super::{FexError, Arch};
use std::ptr;
use std::ffi::CString;
use std::ops::Drop;

/// FEL device context struct
pub struct Context {
    ptr: *mut super::sunxi_fel_ctx_t,
}

impl Context {
    /// Create a new Context instance
    pub fn new() -> Result<Self, FexError> {
        unsafe {
            // Allocate memory
            let ctx_ptr = libc::calloc(1, std::mem::size_of::<super::sunxi_fel_ctx_t>())
                as *mut super::sunxi_fel_ctx_t;
            
            if ctx_ptr.is_null() {
                return Err(FexError::Unknown("Failed to allocate memory for context".to_string()));
            }
            
            Ok(Context { ptr: ctx_ptr })
        }
    }
    
    /// Get internal C pointer (for internal use only)
    pub(crate) fn as_ptr(&self) -> *mut super::sunxi_fel_ctx_t {
        self.ptr
    }
    
    /// Scan for USB devices
    pub fn scan_usb_device(&mut self) -> Result<bool, FexError> {
        unsafe {
            let result = super::sunxi_scan_usb_device(self.ptr);
            if result > 0 {
                Ok(true)
            } else {
                Err(FexError::DeviceNotFound)
            }
        }
    }
    
    /// Initialize USB connection
    pub fn usb_init(&mut self) -> Result<(), FexError> {
        unsafe {
            let result = super::sunxi_usb_init(self.ptr);
            if result > 0 {
                Ok(())
            } else {
                Err(FexError::UsbError("USB initialization failed".to_string()))
            }
        }
    }
    
    /// Initialize FEL mode
    pub fn fel_init(&mut self) -> Result<(), FexError> {
        unsafe {
            let result = super::sunxi_fel_init(self.ptr);
            if result >= 0 {
                Ok(())
            } else {
                Err(FexError::FelInitializationFailed)
            }
        }
    }
    
    /// Get device response data
    pub fn get_device_resp(&self) -> Result<DeviceResponse, FexError> {
        unsafe {
            let resp = (*self.ptr).resp;
            
            // Convert magic field (C char array) to Rust string
            let magic_slice = std::slice::from_raw_parts(
                resp.magic.as_ptr() as *const u8,
                resp.magic.len()
            );
            
            // Find the position of the first null terminator
            let magic_len = magic_slice.iter().position(|&c| c == 0).unwrap_or(magic_slice.len());
            let magic = String::from_utf8_lossy(&magic_slice[..magic_len]).to_string();
            
            Ok(DeviceResponse {
                magic,
                id: resp.id,
                firmware: resp.firmware,
                mode: resp.mode,
                data_flag: resp.data_flag,
                data_length: resp.data_length,
                data_start_address: resp.data_start_address,
                reserved: resp.reserved,
            })
        }
    }
}

impl Drop for Context {
    /// Release Context resources
    fn drop(&mut self) {
        unsafe {
            // Free device handle
            if !(*self.ptr).hdl.is_null() {
                libc::free((*self.ptr).hdl as *mut libc::c_void);
                (*self.ptr).hdl = ptr::null_mut();
            }
            
            // Free Context struct
            libc::free(self.ptr as *mut libc::c_void);
        }
    }
}

/// Device response data struct
pub struct DeviceResponse {
    /// Magic string
    pub magic: String,
    /// Device ID
    pub id: u32,
    /// Firmware version
    pub firmware: u32,
    /// Device mode
    pub mode: u16,
    /// Data flag
    pub data_flag: u8,
    /// Data length
    pub data_length: u8,
    /// Data start address
    pub data_start_address: u32,
    /// Reserved field
    pub reserved: [u8; 8],
}