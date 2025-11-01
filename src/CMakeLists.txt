add_subdirectory(arch)

add_library(efex STATIC
        efex-common.c
        efex-fel.c
        efex-fes.c
        efex-payloads.c
        efex-usb.c

        $<TARGET_OBJECTS:arch-obj>
)

if(WIN32 AND USE_WINUSB)
    target_sources(efex PRIVATE
        usb/winusb_layer.c
    )
    if(USE_WINUSB)
        target_link_libraries(efex PRIVATE SetupAPI.lib)
    endif()
    message(STATUS "Building with winusb support")
else()
    target_sources(efex PRIVATE
        usb/libusb_layer.c
    )
    message(STATUS "Building with libusb support")
endif()
