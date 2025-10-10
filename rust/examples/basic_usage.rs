//! Basic usage example for libfex Rust bindings

use libfex::{Context, Arch, readl, writel, read_memory, write_memory, payloads_init};

fn main() {
    println!("libfex Rust Bindings - Basic Usage Example");
    
    // Create Context object
    let mut ctx = match Context::new() {
        Ok(ctx) => ctx,
        Err(err) => {
            eprintln!("Failed to create Context: {:?}", err);
            return;
        }
    };
    
    // Test architecture constants
    println!("\nArchitecture Constants:");
    println!("- Arch::ARM32");
    println!("- Arch::AARCH64");
    println!("- Arch::RISCV32_E907");
    
    // Scan for devices
    match ctx.scan_usb_device() {
        Ok(true) => println!("\nFEL device found!") ,
        Ok(false) | Err(_) => {
            println!("\nNo FEL device found");
            println!("Note: Put your device in FEL mode and try again.");
            return;
        }
    }
    
    // Initialize USB connection
    if let Err(err) = ctx.usb_init() {
        eprintln!("USB initialization failed: {:?}", err);
        return;
    }
    println!("USB initialization successful!");
    
    // Initialize FEL mode
    if let Err(err) = ctx.fel_init() {
        eprintln!("FEL initialization failed: {:?}", err);
        return;
    }
    println!("FEL initialization successful!");
    
    // Get device response data
    if let Ok(resp) = ctx.get_device_resp() {
        println!("\nDevice Response Data:");
        println!("- Magic: {}", resp.magic);
        println!("- ID: 0x{:08X}", resp.id);
        println!("- Firmware: 0x{:08X}", resp.firmware);
        println!("- Mode: {}", resp.mode);
        println!("- Data Flag: {}", resp.data_flag);
        println!("- Data Length: {}", resp.data_length);
        println!("- Data Start Address: 0x{:08X}", resp.data_start_address);
    }
    
    // Initialize payload
    if let Err(err) = payloads_init(Arch::ARM32) {
        eprintln!("Payload initialization failed: {:?}", err);
        return;
    }
    println!("ARM32 architecture payload initialized!");
    
    println!("\nNote: For security reasons, this example does not perform actual memory read/write operations.");
    println!("Modify the code to include memory operations for your specific use case.");
    println!("\nExample memory operations (commented out):");
    println!("// let address = 0x10000000;");
    println!("// writel(&ctx, 0x12345678, address);");
    println!("// let value = readl(&ctx, address);");
    println!("// println!(\"Read value: 0x{:08X}\", value);");
}