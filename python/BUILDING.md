# Building libefex Python Bindings

This document provides detailed instructions for building the Python bindings for libefex.

## Prerequisites

Before building the Python bindings, ensure you have the following dependencies installed:
- Python 3.6 or higher
- pip
- libusb development library
- C compiler (GCC, Clang, MSVC, etc.)

## Building on Windows

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/libefex.git
    cd libefex/python
   ```

2. Install the Python package:
   ```bash
   pip install .
   ```

   Or install in development mode:
   ```bash
   pip install -e .
   ```

## Building on Linux/macOS

1. Clone the repository:
   ```bash
   git clone https://github.com/YuzukiTsuru/libefex.git
   cd libefex/python
   ```

2. Install system dependencies (example for Ubuntu/Debian):
   ```bash
   sudo apt-get install libusb-1.0-0-dev
   ```

3. Install the Python package:
   ```bash
   pip install .
   ```

   Or install in development mode:
   ```bash
   pip install -e .
   ```

## Troubleshooting

### libusb not found

If the build process cannot find the libusb library, you may need to specify the paths manually:

```bash
# On Windows
export LIBUSB_DIR="C:\path\to\libusb"

# On Linux/macOS
export LIBUSB_INCLUDE_DIR="/path/to/libusb/include"
export LIBUSB_LIBRARY_DIR="/path/to/libusb/lib"

# Then try installing again
pip install .
```

### Compiler errors

Make sure you have a compatible C compiler installed and properly configured in your system PATH.

For Windows, you may need to install Visual Studio Build Tools.

For Linux, install build-essential:
```bash
sudo apt-get install build-essential
```

### Python version issues

Ensure you're using Python 3.6 or newer. You can check your Python version with:
```bash
python --version
```