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

### Features

| Feature | Description |
|---------|-------------|
| (default) | Link against system libusb, fallback to vendored static build |
| `vendored` | Build libusb from source as static library |
| `vendored-shared` | Build libusb from source as shared library (LGPL compatible) |

### Usage

```toml
# In your Cargo.toml
[dependencies]
libefex-sys = { path = "path/to/libefex/rust/libefex-sys" }

# For LGPL compliance (dynamic linking)
libefex-sys = { path = "path/to/libefex/rust/libefex-sys", features = ["vendored-shared"] }
```

### Build

```bash
cd rust

# Default build (uses system libusb or static vendored)
cargo build

# Force static vendored build
cargo build --features vendored

# Build with shared library (LGPL compatible)
cargo build --features vendored-shared
```

### Windows Requirements

For `vendored-shared` on Windows, you need to build from **Visual Studio Developer Command Prompt** or ensure `link.exe` and `rc.exe` are in PATH.

### License Compliance

- **Static linking** (`vendored` or default fallback): Your application may be subject to LGPL requirements
- **Dynamic linking** (`vendored-shared`): LGPL compliant, the shared library can be replaced by users

## License

libefex is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

libusb is licensed under the GNU Lesser General Public License v2.1. See [lib/libusb-cmake/libusb/COPYING](lib/libusb-cmake/libusb/COPYING) for details.