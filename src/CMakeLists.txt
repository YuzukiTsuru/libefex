add_library(arch-obj OBJECT
        arch/riscv32_e907/fel-payloads.c
)

add_library(efex STATIC
        fel-protocol.c
        fes-protocol.c
        usb/libusb_layer.c
        fel-payloads.c

        $<TARGET_OBJECTS:arch-obj>
        usb/winusb_layer.c
)
