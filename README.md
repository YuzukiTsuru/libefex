# libefex

libefex is a cross-platform library for interacting with Allwinner chips in both FEL and FES modes. FEL is a low-level subroutine contained in the BootROM on Allwinner devices used for initial programming and recovery. FES mode is another BootROM subroutine used for more advanced flash operations.

## Features

- Scan and connect to Allwinner devices in FEL and FES modes
- Read and write device memory
- Execute code in device memory
- Flash programming and management
- Support for multiple processor architectures (ARM32, AARCH64, RISC-V32 E907)
- C language API interface
- Python bindings - WIP
- Rust bindings - WIP

## Build Guide

### Prerequisites

- CMake 3.11 or higher
- C compiler (GCC, Clang, MSVC, etc.)
- libusb development library

### Build Steps

```bash
# Clone the repository
git clone https://github.com/YuzukiTsuru/libefex.git
cd libefex

# Create build directory
mkdir build
cd build

# Configure the project
cmake ..

# Build the project
cmake --build .
```

## License

libefex is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.