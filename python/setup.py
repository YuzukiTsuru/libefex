#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import platform
from setuptools import setup, find_packages, Extension
from wheel.bdist_wheel import bdist_wheel


class bdist_wheel_abi3(bdist_wheel):
    def get_tag(self):
        python, abi, plat = super().get_tag()

        if python.startswith("cp"):
            # on CPython, our wheels are abi3 and compatible back to 3.6
            return "cp36", "abi3", plat

        return python, abi, plat


# Determine paths for header files and libraries
def get_include_dirs():
    include_dirs = [os.path.abspath(os.path.join('..', 'includes'))]
    # Add libusb header path from the project (correct path is lib/libusb/include)
    libusb_include_path = os.path.abspath(os.path.join('..', 'lib', 'libusb', 'include'))
    if os.path.exists(libusb_include_path):
        include_dirs.append(libusb_include_path)
    
    # Add possible system include paths
    if platform.system() != 'Windows':
        # Possible include paths for Linux/Mac platforms
        for path in ['/usr/include', '/usr/local/include']:
            if os.path.exists(os.path.join(path, 'libusb-1.0')):
                include_dirs.append(os.path.join(path, 'libusb-1.0'))
    
    print(f"Include directories: {include_dirs}")
    return include_dirs


def get_libraries():
    if platform.system() == 'Windows':
        # On Windows, we know there's libusb-1.0.lib in the VS2022 directory
        return ['libusb-1.0']
    else:
        return ['usb-1.0']


def get_library_dirs():
    library_dirs = []
    
    if platform.system() == 'Windows':
        # On Windows, use the 64-bit static library from VS2022
        libusb_lib_path = os.path.abspath(os.path.join('..', 'lib', 'libusb', 'VS2022', 'MS64', 'static'))
        if os.path.exists(libusb_lib_path):
            library_dirs.append(libusb_lib_path)
            print(f"Added Windows library path: {libusb_lib_path}")
    else:
        # Add libusb library path from the project
        libusb_lib_path = os.path.abspath(os.path.join('..', 'lib', 'libusb'))
        if os.path.exists(libusb_lib_path):
            library_dirs.append(libusb_lib_path)
        
        # Possible library paths for Linux/Mac platforms
        for path in ['/usr/lib', '/usr/local/lib']:
            if os.path.exists(path):
                library_dirs.append(path)
    
    print(f"Library directories: {library_dirs}")
    return library_dirs


# Create extension module
ext = Extension(
    name='libfex',
    sources=[
        'libfex-binding.c',
        os.path.abspath(os.path.join('..', 'src', 'fel-protocol.c')),
        os.path.abspath(os.path.join('..', 'src', 'usb_layer.c')),
        os.path.abspath(os.path.join('..', 'src', 'fel-payloads.c')),
        os.path.abspath(os.path.join('..', 'src', 'arch', 'riscv32_e907', 'fel-payloads.c'))
    ],
    include_dirs=get_include_dirs(),
    libraries=get_libraries(),
    library_dirs=get_library_dirs(),
    py_limited_api=True,
    # Remove PY_SSIZE_T_CLEAN macro definition as it's already defined in libfex-binding.c
    define_macros=[]
)


# Project metadata
setup_args = dict(
    name='libfex',
    version='0.1.0',
    description='Python bindings for libfex - Allwinner FEL mode interaction library',
    long_description='Python bindings for libfex, a cross-platform library for interacting with Allwinner chips in FEL mode.',
    author='libfex developers',
    url='https://github.com/yourusername/libfex',
    packages=[],  # No Python packages, just the C extension
    ext_modules=[ext],
    cmdclass={"bdist_wheel": bdist_wheel_abi3},
    install_requires=[],
    python_requires='>=3.6',
    license='MIT',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Topic :: Software Development :: Libraries',
        'Topic :: System :: Hardware',
    ],
    options={
        'build_ext': {
            'include_dirs': get_include_dirs(),
        }
    }
)


# Execute installation
if __name__ == '__main__':
    setup(**setup_args)
