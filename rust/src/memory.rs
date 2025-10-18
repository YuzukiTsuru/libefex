//! Memory operation related functionality

use super::{Context, EfexError, Arch};
use std::ptr;

/// Read a 32-bit value from the specified memory address
pub fn readl(ctx: &Context, address: u32) -> Result<u32, EfexError> {
    unsafe {
        let value = super::sunxi_efex_readl(ctx.as_ptr(), address);
        Ok(value)
    }
}

/// Write a 32-bit value to the specified memory address
pub fn writel(ctx: &Context, value: u32, address: u32) -> Result<(), EfexError> {
    unsafe {
        super::sunxi_efex_writel(ctx.as_ptr(), value, address);
        Ok(())
    }
}

/// Read a block of memory data from the specified address
pub fn read_memory(ctx: &Context, address: u32, length: usize) -> Result<Vec<u8>, EfexError> {
    let mut buffer = vec![0u8; length];
    unsafe {
        super::sunxi_efex_read_memory(
            ctx.as_ptr(), 
            address, 
            buffer.as_mut_ptr() as *const libc::c_char,
            length
        );
        Ok(buffer)
    }
}

/// Write a block of memory data to the specified address
pub fn write_memory(ctx: &Context, address: u32, data: &[u8]) -> Result<(), EfexError> {
    unsafe {
        super::sunxi_efex_write_memory(
            ctx.as_ptr(), 
            address, 
            data.as_ptr() as *const libc::c_char,
            data.len()
        );
        Ok(())
    }
}

/// Execute code at the specified memory address
pub fn exec(ctx: &Context, address: u32) -> Result<(), EfexError> {
    unsafe {
        super::sunxi_efex_exec(ctx.as_ptr(), address);
        Ok(())
    }
}

/// Initialize payload for a specific architecture
pub fn payloads_init(arch: Arch) -> Result<(), EfexError> {
    unsafe {
        super::sunxi_efex_payloads_init(arch.to_raw() as u32);
        Ok(())
    }
}

/// Read a 32-bit value from the specified memory address using the current payload
pub fn payloads_readl(ctx: &Context, address: u32) -> Result<u32, EfexError> {
    unsafe {
        let value = super::sunxi_efex_payloads_readl(ctx.as_ptr(), address);
        Ok(value)
    }
}

/// Write a 32-bit value to the specified memory address using the current payload
pub fn payloads_writel(ctx: &Context, value: u32, address: u32) -> Result<(), EfexError> {
    unsafe {
        super::sunxi_efex_payloads_writel(ctx.as_ptr(), value, address);
        Ok(())
    }
}