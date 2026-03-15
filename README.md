# libefex

libefex is a cross-platform library for interacting with Sunxi chips in both FEL and FES modes. FEL is a low-level subroutine contained in the BootROM on Sunxi devices used for initial programming and recovery. FES mode is another BootROM subroutine used for more advanced flash operations.

## Features

- Scan and connect to Sunxi devices in FEL and FES modes
- Read and write device memory
- Execute code in device memory
- Flash programming and management
- Support for multiple processor architectures (ARM32, AARCH64, RISC-V32 E907)
- C language API interface
- Python bindings - WIP
- Rust bindings

## Build Guide

### Prerequisites

- CMake 3.16 or higher
- C compiler (GCC, Clang, MSVC, etc.)
- Linux: `libudev-dev` package (optional, for USB hotplug support)

### Build Steps

```bash
# Clone the repository with submodules
git clone --recursive https://github.com/YuzukiTsuru/libefex.git
cd libefex

# Create build directory
mkdir build && cd build

# Configure the project
cmake ..

# Build the project
cmake --build .
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `LIBEFEX_USE_SHARED_LIBUSB` | ON | Build libusb as shared library (LGPL compatible) |
| `USE_WINUSB` | ON (Windows) | Enable WinUSB backend on Windows |

To build with static libusb:

```bash
cmake -DLIBEFEX_USE_SHARED_LIBUSB=OFF ..
```

## Rust Bindings

Rust bindings are available in the `rust/` directory.

### Build System

libefex-sys uses **CMake** to build libusb as a shared library (LGPL compatible).

| Target | Build Tool | Library Type |
|--------|------------|--------------|
| Windows | CMake | Shared (.dll) |
| macOS | CMake | Shared (.dylib) |
| Linux | CMake | Shared (.so) |

### Usage

```toml
# In your Cargo.toml
[dependencies]
libefex-sys = { path = "path/to/libefex/rust/libefex-sys" }
```

### Build

```bash
cd rust

# Build (uses CMake to compile libusb as shared library)
cargo build --release
```

### Requirements

- **CMake 3.16+** is required
- **Linux**: `libudev-dev` package (optional, for USB hotplug support)

### License Compliance

- libusb is built as a **shared library** (LGPL compliant)
- The shared library can be replaced by users

## License

libefex is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

libusb is licensed under the GNU Lesser General Public License v2.1. See [lib/libusb-cmake/libusb/COPYING](lib/libusb-cmake/libusb/COPYING) for details.
