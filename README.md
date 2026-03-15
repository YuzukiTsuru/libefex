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
- Linux: `libusb-1.0-0-dev` package

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

libefex-sys uses the following strategy to find or build libusb:

1. **System libusb**: First tries to find libusb via pkg-config (Linux/macOS)
2. **Build from source**: If not found, builds libusb from source using CMake

| Platform | Library Source | Library Type |
|----------|----------------|--------------|
| Linux | System (pkg-config) or CMake | Shared (.so) |
| macOS | System (pkg-config) or CMake | Shared (.dylib) |
| Windows | CMake | Shared (.dll) |

### Supported Targets

| Platform | Target |
|----------|--------|
| Windows x86_64 | `x86_64-pc-windows-msvc` |
| Windows ARM64 | `aarch64-pc-windows-msvc` |
| macOS ARM64 | `aarch64-apple-darwin` |
| Linux x86_64 | `x86_64-unknown-linux-gnu` |
| Linux ARM64 | `aarch64-unknown-linux-gnu` |

### Usage

```toml
# In your Cargo.toml
[dependencies]
libefex-sys = { path = "path/to/libefex/rust/libefex-sys" }
```

### Build

```bash
cd rust
cargo build --release
```

Requirements:
- **CMake 3.16+**
- **Linux**: `libusb-1.0-0-dev` package (recommended)
- **macOS**: `libusb` via Homebrew (optional)

### License Compliance

libusb is linked as a **shared library** (LGPL compliant), users can replace the library.

## CI Matrix

| OS | Target | Tests |
|----|--------|-------|
| windows-latest | x86_64-pc-windows-msvc | ✓ |
| windows-latest | aarch64-pc-windows-msvc | ✗ |
| macos-latest | aarch64-apple-darwin | ✓ |
| ubuntu-latest | x86_64-unknown-linux-gnu | ✓ |
| ubuntu-24.04-arm | aarch64-unknown-linux-gnu | ✓ |

## License

libefex is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

libusb is licensed under the GNU Lesser General Public License v2.1. See [lib/libusb-cmake/libusb/COPYING](lib/libusb-cmake/libusb/COPYING) for details.
