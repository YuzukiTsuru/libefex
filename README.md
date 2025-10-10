# libfex

libfex is a cross-platform library for interacting with Allwinner chips in FEL mode. FEL is a low-level subroutine contained in the BootROM on Allwinner devices. It is used for initial programming and recovery of devices using USB.

## Features

- Scan and connect to Allwinner devices in FEL mode
- Read and write device memory
- Execute code in device memory
- Support for multiple processor architectures (ARM32, AARCH64, RISC-V32 E907)
- C language API interface
- Python bindings
- Rust bindings

## Build Guide

### Prerequisites

- CMake 3.11 or higher
- C compiler (GCC, Clang, MSVC, etc.)
- libusb development library

### Build Steps

```bash
# Clone the repository
git clone https://github.com/yourusername/libfex.git
cd libfex

# Create build directory
mkdir build
cd build

# Configure the project
cmake ..

# Build the project
cmake --build .
```

## Usage Examples

### C Language Examples

#### Scanning for Devices

```c
#include <stdio.h>
#include "libFEx.h"
#include "usb_layer.h"
#include "fel-protocol.h"

int main() {
    struct sunxi_fel_ctx_t ctx = {0};
    int ret = 0;

    // Scan for FEL devices
    ret = sunxi_scan_usb_device(&ctx);
    if (ret <= 0) {
        fprintf(stderr, "ERROR: Can't get valid FEL device\r\n");
        return -1;
    }
    
    // Initialize USB
    ret = sunxi_usb_init(&ctx);
    if (ret <= 0) {
        fprintf(stderr, "ERROR: FEL device USB init failed\r\n");
        return -1;
    }
    
    // Initialize FEL
    ret = sunxi_fel_init(&ctx);
    if (ret < 0) {
        fprintf(stderr, "ERROR: FEL device init failed\r\n");
        return -1;
    }

    // Print device information
    printf("Found FEL device\n");
    printf("Magic: %s\n", ctx.resp.magic);
    printf("ID: 0x%08x\n", ctx.resp.id);
    // ... print more device information ...

    return 0;
}
```

#### Memory Read/Write

```c
// Write 32-bit value to specified address
uint32_t value = 0x12345678;
uint32_t address = 0x10000000;
sunxi_fel_writel(&ctx, value, address);

// Read 32-bit value from specified address
uint32_t read_value = sunxi_fel_readl(&ctx, address);
printf("Read value: 0x%08x\n", read_value);

// Write memory block
char buffer[1024] = "Hello, libfex!";
sunxi_fel_write_memory(&ctx, address, buffer, sizeof(buffer));

// Read memory block
char read_buffer[1024];
sunxi_fel_read_memory(&ctx, address, read_buffer, sizeof(read_buffer));
```

### Python Bindings

libfex provides complete Python bindings for all the core functionality of the library, making it easy to interact with Allwinner devices in FEL mode using Python.

#### Installation

Before building the Python bindings, ensure you have the following dependencies installed:
- Python 3.6 or higher
- pip
- libusb development library
- C compiler

```bash
# From the root directory of the repository
cd python
pip install .

# Alternatively, install in development mode
pip install -e .
```

For detailed build instructions and troubleshooting, please refer to the [BUILDING.md](python/BUILDING.md) file.

#### Basic Usage Example

```python
import libfex

# Create a FEL context object
ctx = libfex.Context()

# Scan for FEL devices
if libfex.scan_usb_device(ctx) <= 0:
    print("No FEL device found")
    exit(1)

# Initialize USB connection
if libfex.usb_init(ctx) <= 0:
    print("USB initialization failed")
    exit(1)

# Initialize FEL mode
if libfex.fel_init(ctx) < 0:
    print("FEL initialization failed")
    exit(1)

print("Successfully connected to FEL device")

# Initialize payload for specific architecture
# Available architectures: Arch.ARM32, Arch.AARCH64, Arch.RISCV32_E907
libfex.payloads_init(libfex.Arch.ARM32)

# Memory operations example
address = 0x10000000

# Write a 32-bit value
libfex.writel(ctx, 0x12345678, address)

# Read back the value
value = libfex.readl(ctx, address)
print(f"Read value: 0x{value:x}")

# Write a block of memory
buffer = b"Hello from libfex Python bindings!"
libfex.write_memory(ctx, address + 0x100, buffer)

# Read back the block
read_buffer = libfex.read_memory(ctx, address + 0x100, len(buffer))
print(f"Read buffer: {read_buffer.decode('utf-8', errors='ignore')}")

# Execute code at a specific address (use with caution)
# libfex.exec(ctx, code_address)
```

#### Python API Reference

The Python API mirrors the C API closely, providing the following key functions:

- `Context()`: Create a new FEL context object
- `scan_usb_device(ctx)`: Scan for USB devices in FEL mode
- `usb_init(ctx)`: Initialize USB connection to the device
- `fel_init(ctx)`: Initialize FEL mode on the device
- `writel(ctx, value, address)`: Write a 32-bit value to memory
- `readl(ctx, address)`: Read a 32-bit value from memory
- `write_memory(ctx, address, data)`: Write a block of data to memory
- `read_memory(ctx, address, length)`: Read a block of data from memory
- `exec(ctx, address)`: Execute code at the specified address
- `payloads_init(arch)`: Initialize payload for a specific architecture
- `payloads_readl(ctx, address)`: Read a 32-bit value using the current payload
- `payloads_writel(ctx, value, address)`: Write a 32-bit value using the current payload

The `Arch` dictionary provides architecture constants:
- `Arch.ARM32`: ARM 32-bit architecture
- `Arch.AARCH64`: ARM 64-bit architecture
- `Arch.RISCV32_E907`: RISC-V 32-bit E907 architecture

#### Example Script

A complete example script is available in the repository at `python/example.py`, demonstrating how to use the Python bindings to scan for devices and perform basic operations.

## API Reference

### Core Structures

- `struct sunxi_fel_ctx_t`: FEL context, containing device handle and response information
- `struct payloads_ops`: Payload operations structure, containing architecture type and read/write functions

### Main Functions

#### Device Operations
- `int sunxi_scan_usb_device(struct sunxi_fel_ctx_t *ctx)`: Scan for USB devices
- `int sunxi_fel_init(struct sunxi_fel_ctx_t *ctx)`: Initialize FEL context

#### Memory Operations
- `void sunxi_fel_writel(const struct sunxi_fel_ctx_t *ctx, uint32_t val, uint32_t addr)`: Write 32-bit value to specified address
- `uint32_t sunxi_fel_readl(const struct sunxi_fel_ctx_t *ctx, uint32_t addr)`: Read 32-bit value from specified address
- `void sunxi_fel_write_memory(const struct sunxi_fel_ctx_t *ctx, uint32_t addr, const char *buf, size_t len)`: Write memory block
- `void sunxi_fel_read_memory(const struct sunxi_fel_ctx_t *ctx, uint32_t addr, const char *buf, size_t len)`: Read memory block

#### Execution Operations
- `void sunxi_fel_exec(const struct sunxi_fel_ctx_t *ctx, uint32_t addr)`: Execute code at specified address

#### Payload Operations
- `void sunxi_fel_payloads_init(enum sunxi_fel_payloads_arch arch)`: Initialize payload for specific architecture
- `struct payloads_ops *sunxi_fel_get_current_payload()`: Get current payload operations
- `uint32_t sunxi_fel_payloads_readl(const struct sunxi_fel_ctx_t *ctx, uint32_t addr)`: Read 32-bit value using current payload
- `void sunxi_fel_payloads_writel(const struct sunxi_fel_ctx_t *ctx, uint32_t value, uint32_t addr)`: Write 32-bit value using current payload

## Supported Architectures

- ARM32
- AARCH64 (ARM64)
- RISC-V32 E907

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request