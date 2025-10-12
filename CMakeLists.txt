cmake_minimum_required(VERSION 3.11)
project(libefex C)

set(CMAKE_C_STANDARD 11)

include_directories(
        includes
        lib/libusb/include
)

add_subdirectory(src)

# tests for lib
include(cmake/add_test_case.cmake)
add_executable_with_libraries(scan_device_test test/scan_device.c)
add_executable_with_libraries(writel_readl test/writel_readl.c)
add_executable_with_libraries(hexdump_brom test/hexdump_brom.c)
add_executable_with_libraries(e907_payload_io test/e907_payload_io.c)
