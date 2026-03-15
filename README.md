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

libefex-sys automatically detects the build environment and chooses the appropriate linking strategy:

| Platform | Build Type | Build Tool | Library Type |
|----------|------------|------------|--------------|
| Windows | Native / Cross | CMake | Shared (.dll) |
| macOS | Native | CMake | Shared (.dylib) |
| Linux | Native | CMake | Shared (.so) |
| Linux | Cross | cc | Static |

### Supported Targets

| Platform | Target | Build Type |
|----------|--------|------------|
| Windows x86_64 | `x86_64-pc-windows-msvc` | Native |
| Windows ARM64 | `aarch64-pc-windows-msvc` | Cross (Shared) |
| macOS ARM64 | `aarch64-apple-darwin` | Native |
| Linux x86_64 | `x86_64-unknown-linux-gnu` | Native |
| Linux ARM64 | `aarch64-unknown-linux-gnu` | Native / Cross |

### Usage

```toml
# In your Cargo.toml
[dependencies]
libefex-sys = { path = "path/to/libefex/rust/libefex-sys" }
```

### Build

#### Native Build

```bash
cd rust
cargo build --release
```

Requirements:
- **CMake 3.16+**
- **Linux**: `libudev-dev` package (optional, for USB hotplug support)

#### Cross-compilation

Linux 交叉编译时使用静态链接，无需配置 CMake 交叉编译工具链。

```bash
# 安装交叉编译工具链
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# 设置环境变量
export CC_aarch64_unknown_linux_gnu=aarch64-linux-gnu-gcc
export CXX_aarch64_unknown_linux_gnu=aarch64-linux-gnu-g++
export AR_aarch64_unknown_linux_gnu=aarch64-linux-gnu-ar
export CARGO_TARGET_AARCH64_UNKNOWN_LINUX_GNU_LINKER=aarch64-linux-gnu-gcc

# 编译
cd rust
cargo build --release --target aarch64-unknown-linux-gnu
```

Windows 交叉编译（如 x86_64 交叉编译 aarch64）使用 CMake 动态链接，无需额外配置：

```bash
cargo build --release --target aarch64-pc-windows-msvc
```

### How It Works

The build system automatically detects cross-compilation by comparing `HOST` and `TARGET`:

1. **Windows**: Always uses CMake to build libusb as a shared library (.dll), regardless of native or cross-compilation.

2. **macOS**: Uses CMake to build libusb as a shared library (.dylib).

3. **Linux Native**: Uses CMake to build libusb as a shared library (.so).

4. **Linux Cross-compilation**: Uses `cc` crate to compile libusb from source as a static library, avoiding the need for cross-compiled CMake toolchain setup.

### License Compliance

- **Windows / macOS / Linux Native**: libusb is built as a **shared library** (LGPL compliant), users can replace the library.
- **Linux Cross-compilation**: libusb is statically linked (still LGPL compliant if you provide object files or source code).

## CI Matrix

| OS | Target | Type | Library | Tests |
|----|--------|------|---------|-------|
| windows-latest | x86_64-pc-windows-msvc | Native | Shared | ✓ |
| windows-latest | aarch64-pc-windows-msvc | Cross | Shared | ✗ |
| macos-latest | aarch64-apple-darwin | Native | Shared | ✓ |
| ubuntu-latest | x86_64-unknown-linux-gnu | Native | Shared | ✓ |
| ubuntu-latest | aarch64-unknown-linux-gnu | Cross | Static | ✗ |
| ubuntu-24.04-arm | aarch64-unknown-linux-gnu | Native | Shared | ✓ |

## License

libefex is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

libusb is licensed under the GNU Lesser General Public License v2.1. See [lib/libusb-cmake/libusb/COPYING](lib/libusb-cmake/libusb/COPYING) for details.
