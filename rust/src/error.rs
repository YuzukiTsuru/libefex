use thiserror::Error;
use std::io;
use std::ffi::NulError;

/// Error types that can occur during libfex operations
#[derive(Error, Debug)]
pub enum FexError {
    /// USB device related errors
    #[error("USB device error: {0}")]
    UsbError(String),
    
    /// Device not found error
    #[error("No FEL device found")]
    DeviceNotFound,
    
    /// FEL initialization error
    #[error("FEL initialization failed")]
    FelInitializationFailed,
    
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