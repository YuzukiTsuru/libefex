cmake_minimum_required(VERSION 3.16)
project(libefex C)

set(CMAKE_C_STANDARD 11)

if(WIN32)
    option(USE_WINUSB "Use winusb for USB communication on Windows" ON)
    message("-- Enable winusb support: ${USE_WINUSB}")
endif()

# libusb build options (default shared for LGPL compliance)
option(LIBEFEX_USE_SHARED_LIBUSB "Build libusb as shared library (LGPL compatible)" ON)
if(LIBEFEX_USE_SHARED_LIBUSB)
    set(LIBUSB_BUILD_SHARED_LIBS ON CACHE BOOL "Build libusb as shared library" FORCE)
else()
    set(LIBUSB_BUILD_SHARED_LIBS OFF CACHE BOOL "Build libusb as static library" FORCE)
endif()

add_subdirectory(lib/libusb-cmake)

include_directories(
        includes
)

add_subdirectory(src)

# app CLI
include(cmake/add_test_case.cmake)
add_executable_with_libraries(efex-cli app/efex-cli.c)

# tests for lib
add_executable_with_libraries(scan_device test/scan_device.c)
add_executable_with_libraries(fel_test test/fel_test.c)
add_executable_with_libraries(fes_test test/fes_test.c)
add_executable_with_libraries(fes_flash test/fes_flash.c)
