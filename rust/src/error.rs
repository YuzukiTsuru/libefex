use thiserror::Error;
use std::io;
use std::ffi::NulError;

/// Error types that can occur during libefex operations
#[derive(Error, Debug)]
pub enum EfexError {
    /// USB device related errors
    #[error("USB device error: {0}")]
    UsbError(String),
    
    /// Device not found error
    #[error("No EFEX device found")]
    DeviceNotFound,
    
    /// EFEX initialization error
    #[error("EFEX initialization failed")]
    EfexInitializationFailed,
    
    /// Memory operation error
    #[error("Memory operation failed: {0}")]
    MemoryError(String),
    
    /// I/O operation error
    #[error("I/O error: {0}")]
    IoError(#[from] io::Error),
    
    /// String format error
    #[error("String format error: {0}")]
    StringError(#[from] NulError),
    
    /// Other unknown errors
    #[error("Unknown error: {0}")]
    Unknown(String),
}