# libfex Rust Bindings

Rust language binding library for interacting with Allwinner chips in FEL mode.

## Features

- Scan and connect to Allwinner devices in FEL mode
- Read and write device memory
- Execute code in device memory
- Support for multiple processor architectures (ARM32, AARCH64, RISC-V32 E907)
- Safe resource management (automatic memory and device handle release)
- Error handling (using `Result` type and custom error enum)

## Build Guide

### Prerequisites

- Rust 1.56 or higher (supports 2021 edition)
- Cargo package manager
- C compiler (GCC, Clang, MSVC, etc.)
- libusb development library

### Build Steps

From the project root directory, execute the following commands:

```bash
cd rust
cargo build
```

For release builds:

```bash
cargo build --release
```

## Usage Examples

Here is a basic usage example:

```rust
use libfex::{Context, Arch, readl, writel};

fn main() {
    // 创建Context对象
    let mut ctx = Context::new().expect("Failed to create context");
    
    // 扫描设备
    if ctx.scan_usb_device().is_ok() {
        println!("FEL device found!");
        
        // 初始化USB连接
        ctx.usb_init().expect("USB initialization failed");
        
        // 初始化FEL模式
        ctx.fel_init().expect("FEL initialization failed");
        
        // 初始化特定架构的负载
        libfex::payloads_init(Arch::ARM32).expect("Payload initialization failed");
        
        // 内存操作示例
        let address = 0x10000000;
        writel(&ctx, 0x12345678, address).expect("Failed to write memory");
        let value = readl(&ctx, address).expect("Failed to read memory");
        println!("Read value: 0x{:08X}", value);
    }
}
```

For more examples, please check the `examples` directory.

## API Reference

### Main Structs

- **`Context`**: FEL device context for interacting with devices
  - `new()`: Create a new Context instance
  - `scan_usb_device()`: Scan for USB devices
  - `usb_init()`: Initialize USB connection
  - `fel_init()`: Initialize FEL mode
  - `get_device_resp()`: Get device response data

### Memory Operation Functions

- **`readl(ctx, address)`**: Read a 32-bit value from the specified address
- **`writel(ctx, value, address)`**: Write a 32-bit value to the specified address
- **`read_memory(ctx, address, length)`**: Read a block of memory from the specified address
- **`write_memory(ctx, address, data)`**: Write a block of memory to the specified address
- **`exec(ctx, address)`**: Execute code at the specified address

### Payload Operation Functions

- **`payloads_init(arch)`**: Initialize payload for a specific architecture
- **`payloads_readl(ctx, address)`**: Read a 32-bit value using the current payload
- **`payloads_writel(ctx, value, address)`**: Write a 32-bit value using the current payload

### Architecture Enum

- **`Arch`**: Supported processor architectures
  - `ARM32`: ARM 32-bit architecture
  - `AARCH64`: ARM 64-bit architecture
  - `RISCV32_E907`: RISC-V 32-bit E907 architecture

## Error Handling

The library uses `Result<T, FexError>` type for error handling. The `FexError` enum includes the following error types:

- `UsbError`: USB device related errors
- `DeviceNotFound`: Device not found
- `FelInitializationFailed`: FEL initialization failed
- `MemoryError`: Memory operation error
- `IoError`: I/O operation error
- `StringError`: String format error
- `Unknown`: Unknown error

## Notes

- Performing memory write and code execution operations may affect the device, please use with caution
- Ensure that resources are properly released after using the `Context` object (Rust's ownership system will handle this automatically)
- Different libusb configurations may be required on different platforms

## License

This project is licensed under the GNU General Public License v2.0.