add_library(arch-obj OBJECT
        arch/riscv32_e907/efex-payloads.c
        ../includes/efex-common.h
        ../includes/efex-fel.h
        ../includes/efex-fes.h
)

add_library(efex STATIC
        efex-common.c
        efex-fel.c
        efex-fes.c
        efex-payloads.c
        usb_layer.c

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
