# libefex

libefex is a cross-platform library for interacting with Allwinner chips in FEL mode. FEL is a low-level subroutine contained in the BootROM on Allwinner devices. It is used for initial programming and recovery of devices using USB.

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
git clone https://github.com/yourusername/libefex.git
cd libefex

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
#include "libefex.h"
#include "usb_layer.h"
#include "fel-protocol.h"

int main() {
    struct sunxi_efex_ctx_t ctx = {0};
    int ret = 0;

    // Scan for FEL devices
    ret = sunxi_scan_usb_device(&ctx);
    if (ret != EFEX_ERR_SUCCESS) {
        fprintf(stderr, "ERROR: %s\r\n", sunxi_efex_strerror(ret));
        return ret;
    }
    
    // Initialize USB
    ret = sunxi_usb_init(&ctx);
    if (ret != EFEX_ERR_SUCCESS) {
        fprintf(stderr, "ERROR: %s\r\n", sunxi_efex_strerror(ret));
        sunxi_usb_exit(&ctx);
        return ret;
    }
    
    // Initialize EFEX
    ret = sunxi_efex_init(&ctx);
    if (ret != EFEX_ERR_SUCCESS) {
        fprintf(stderr, "ERROR: %s\r\n", sunxi_efex_strerror(ret));
        sunxi_usb_exit(&ctx);
        return ret;
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
char buffer[1024] = "Hello, libefex!";
sunxi_fel_write_memory(&ctx, address, buffer, sizeof(buffer));

// Read memory block
char read_buffer[1024];
sunxi_fel_read_memory(&ctx, address, read_buffer, sizeof(read_buffer));
```




## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request