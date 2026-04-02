use libefex::{Context, FesDataType, FesToolMode, MultiContext};
use std::env;
use std::fs;
use std::thread;
use std::sync::{Arc, Mutex};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let firmware_path = env::args().nth(1).unwrap_or_else(|| "firmware.bin".to_string());
    let firmware = fs::read(&firmware_path)?;
    println!("Firmware: {} ({} bytes)\n", firmware_path, firmware.len());

    Context::set_usb_backend_static(libefex::UsbBackend::Auto)?;
    sequential_flash(&firmware)?;
    parallel_flash(&firmware)?;
    Ok(())
}

fn scan_devices() -> Vec<Context> {
    Context::set_usb_backend_static(libefex::UsbBackend::Auto).ok();
    let mut devices = Vec::new();
    for _ in 0..16 {
        let mut ctx = Context::new();
        if ctx.scan_usb_device().is_ok() {
            ctx.efex_init().ok();
            devices.push(ctx);
        } else {
            break;
        }
    }
    devices
}

fn flash_device(device: &Context, firmware: &[u8]) -> Result<(), libefex::EfexError> {
    device.fes_down(firmware, 0, FesDataType::Flash)
}

fn sequential_flash(firmware: &[u8]) -> Result<(), Box<dyn std::error::Error>> {
    println!("Sequential flashing:");

    let mut multi_ctx = MultiContext::new()?;
    let count = multi_ctx.scan_devices()?;
    if count == 0 {
        println!("No devices found\n");
        return Ok(());
    }

    println!("Found {} devices\n", count);

    let success = multi_ctx.flash_all(firmware, 0, FesDataType::Flash)?;
    println!("Sequential: {}/{}\n", success, count);

    Ok(())
}

fn parallel_flash(firmware: &[u8]) -> Result<(), Box<dyn std::error::Error>> {
    println!("Parallel flashing:");

    let mut multi_ctx = MultiContext::new()?;
    let count = multi_ctx.scan_devices()?;
    if count == 0 {
        println!("No devices found\n");
        return Ok(());
    }

    println!("Found {} devices\n", count);

    let success = multi_ctx.flash_all_parallel(firmware, 0, FesDataType::Flash, None)?;
    println!("Parallel: {}/{}\n", success, count);

    println!("Rebooting all devices:");
    let rebooted = multi_ctx.reboot_all(FesToolMode::Reboot, FesToolMode::Normal)?;
    println!("Rebooted: {}/{}\n", rebooted, count);

    Ok(())
}
