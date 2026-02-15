add_subdirectory(arch)

add_library(efex STATIC
        efex-common.c
        efex-fel.c
        efex-fes.c
        efex-payloads.c
        efex-usb.c
        usb/usb_layer.c
        usb/usb_layer_libusb.c

        $<TARGET_OBJECTS:arch-obj>
)

if(WIN32)
    target_sources(efex PRIVATE
        usb/usb_layer_winusb.c
    )
    target_link_libraries(efex PRIVATE SetupAPI.lib)
    message(STATUS "Building with winusb and libusb support")
else()
    message(STATUS "Building with libusb support")
endif()
