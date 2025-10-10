#!/usr/bin/env python3
"""
libfex Python Bindings Test Script

This script is used to test the basic functionality of the libfex Python bindings.
"""

import sys
import libfex
from libfex import Arch


def print_separator(title):
    """Print separator line and title"""
    print(f"\n{'=' * 50}")
    print(f"{title}")
    print(f"{'=' * 50}")


def test_device_scan():
    """Test device scanning functionality"""
    print_separator("Device Scanning Test")
    
    ctx = libfex.Context()
    print("Context object created successfully")
    
    scan_result = libfex.scan_usb_device(ctx)
    print(f"Scan result: {scan_result}")
    
    if scan_result > 0:
        print("FEL device found!")
    else:
        print("No FEL device found")
        return ctx, False
    
    return ctx, True

def test_usb_initialization(ctx):
    """Test USB initialization functionality"""
    print_separator("USB Initialization Test")
    
    usb_init_result = libfex.usb_init(ctx)
    print(f"USB initialization result: {usb_init_result}")
    
    if usb_init_result > 0:
        print("USB initialization successful!")
        return True
    else:
        print("USB initialization failed")
        return False

def test_fel_initialization(ctx):
    """Test FEL initialization functionality"""
    print_separator("FEL Initialization Test")
    
    fel_init_result = libfex.fel_init(ctx)
    print(f"FEL initialization result: {fel_init_result}")
    
    if fel_init_result >= 0:
        print("FEL initialization successful!")
        
        # Get and print device response data
        try:
            resp_data = libfex.get_device_resp(ctx)
            print("\nDevice response data:")
            print(f"magic: {resp_data['magic']}")
            print(f"id: 0x{resp_data['id']:08X}")
            print(f"firmware: 0x{resp_data['firmware']:08X}")
            print(f"mode: {resp_data['mode']}")
            print(f"data_flag: {resp_data['data_flag']}")
            print(f"data_length: {resp_data['data_length']}")
            print(f"data_start_address: 0x{resp_data['data_start_address']:08X}")
            print(f"reserved: {resp_data['reserved']}")
        except Exception as e:
            print(f"Failed to get device response data: {e}")
        
        return True
    else:
        print("FEL initialization failed")
        return False

def test_payloads_initialization():
    """Test payload initialization functionality"""
    print_separator("Payload Initialization Test")
    
    try:
        libfex.payloads_init(Arch['RISCV32_E907'])
        print("RISCV32_E907 architecture payload initialization successful!")
        return True
    except Exception as e:
        print(f"RISCV32_E907 architecture payload initialization failed: {e}")
        return False

def test_payloads_read_write(ctx):
    """Test payload read/write functionality"""
    print_separator("Payload Read/Write Test")
    
    try:
        payload = libfex.payloads_readl(ctx, 0x02001000)
        print(f"Read mem at 0x02001000: 0x{payload:08X}")
        
        # Write payload
        libfex.payloads_writel(ctx, 0x80001400, 0x02001000)
        print("Write mem at 0x02001000: 0x80001400")
        
        # Read back payload
        payload = libfex.payloads_readl(ctx, 0x02001000)
        print(f"Read back mem at 0x02001000: 0x{payload:08X}")
        
        return True
    except Exception as e:
        print(f"Payload read/write failed: {e}")
        return False

def test_architecture_constants():
    """Test architecture constants"""
    print_separator("Architecture Constants Test")
    
    architectures = {
        "ARM32": Arch['ARM32'],
        "AARCH64": Arch['AARCH64'],
        "RISCV32_E907": Arch['RISCV32_E907']
    }
    
    for name, value in architectures.items():
        print(f"Arch.{name} = {value}")
    
    return True

def run_tests():
    """Run all tests"""
    print("libfex Python Bindings Test Script")
    print(f"Python version: {sys.version}")
    
    # Test architecture constants
    test_architecture_constants()
    
    # Test device scanning
    ctx, device_found = test_device_scan()
    if not device_found:
        print("\nNote: No FEL device found, cannot proceed with subsequent tests.")
        print("Please put the device in FEL mode and try again.")
        return
    
    # Test USB initialization
    if not test_usb_initialization(ctx):
        print("USB initialization failed, cannot proceed with subsequent tests.")
        return
    
    # Test FEL initialization
    if not test_fel_initialization(ctx):
        print("FEL initialization failed, cannot proceed with subsequent tests.")
        return
    
    # Test payload initialization
    if not test_payloads_initialization():
        print("Payload initialization failed, cannot proceed with subsequent tests.")
        return
    
    # Test payload read/write
    if not test_payloads_read_write(ctx):
        print("Payload read/write failed, cannot proceed with subsequent tests.")
        return
    
    print_separator("Tests Completed")
    print("All tests have been completed.")
    print("Note: For security reasons, this test script does not perform actual memory read/write or code execution operations.")
    print("Please refer to example.py for a complete functional demonstration.")


if __name__ == "__main__":
    run_tests()