cmake_minimum_required(VERSION 3.11)
project(libefex C)

set(CMAKE_C_STANDARD 11)

if(WIN32)
    option(USE_WINUSB "Use winusb for USB communication on Windows" OFF)
endif()

include_directories(
        includes
        lib/libusb/include
)

add_subdirectory(src)

# app CLI
include(cmake/add_test_case.cmake)
add_executable_with_libraries(efex-cli app/efex-cli.c)

# tests for lib
add_executable_with_libraries(scan_device_test test/scan_device.c)
add_executable_with_libraries(writel_readl test/writel_readl.c)
add_executable_with_libraries(hexdump_brom test/hexdump_brom.c)
add_executable_with_libraries(e907_payload_io test/e907_payload_io.c)
