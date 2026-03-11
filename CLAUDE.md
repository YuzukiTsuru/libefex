# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

libefex is a cross-platform C library for interacting with Allwinner Sunxi chips in FEL and FES modes. FEL is a low-level BootROM subroutine for initial programming and recovery. FES mode is used for advanced flash operations.

## Build Commands

```bash
# Create build directory and configure
mkdir build && cd build
cmake ..

# Build the project
cmake --build .

# Run tests via ctest
ctest
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `LIBEFEX_USE_SHARED_LIBUSB` | ON | Build libusb as shared library (LGPL compatible). Set to OFF for static linking. |
| `USE_WINUSB` | ON (Windows) | Enable WinUSB backend on Windows |

### Platform-Specific Notes

- **Linux**: Requires `libudev-dev` package for USB hotplug support
- **Windows**: libusb is built from source via `lib/libusb-cmake`. Both MSVC and MinGW are supported.

### MinGW Example

```bash
cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=D:/SDK/mingw64/bin/gcc.exe ..
cmake --build .
```

### MSVC Example

```bash
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

## Architecture

### Core Components

- **FEL Mode** (`src/efex-fel.c`, `includes/efex-fel.h`): Memory read/write, code execution
- **FES Mode** (`src/efex-fes.c`, `includes/efex-fes.h`): Flash operations, device queries, verification
- **Protocol Layer** (`includes/efex-protocol.h`): Command definitions, packed structures, context `sunxi_efex_ctx_t`
- **USB Layer** (`src/usb/`): Abstraction over libusb and winusb backends

### USB Abstraction

The USB layer has two implementations:
- `usb_layer_libusb.c` - Cross-platform using libusb
- `usb_layer_winusb.c` - Windows-native WinUSB API

The active backend is selected at compile time. Windows builds can use both simultaneously.

### Architecture Support

`src/arch/` contains processor-specific payload code:
- `arm.c` - ARM32
- `aarch64.c` - AArch64
- `riscv.c` - RISC-V32 E907

The payload operations are registered via `sunxi_efex_fel_payloads_init()`.

### Context Structure

All operations use `sunxi_efex_ctx_t` which holds:
- USB device handle and context
- Endpoint addresses (epin/epout)
- Device response info (`sunxi_efex_device_resp_t`)

Typical initialization flow:
1. `sunxi_scan_usb_device()` - Find and open device
2. `sunxi_usb_init()` - Initialize USB layer
3. `sunxi_efex_init()` - Query device info

## Rust Bindings

Located in `rust/` as a Cargo workspace with three crates:
- `libefex-sys` - FFI bindings
- `libefex` - Safe Rust wrapper
- `efex-cli` - CLI tool

Build with `cargo build --manifest-path rust/Cargo.toml`.

## Code Style

Uses clang-format with LLVM base style. Key settings from `.clang-format`:
- Tabs (4 width) for indentation
- 120 column limit
- Custom brace wrapping (same line for functions/control)

Format code with: `clang-format -i <file>`