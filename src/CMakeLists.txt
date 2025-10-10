add_library(arch-obj OBJECT
        arch/riscv32_e907/fel-payloads.c
)

add_library(fex STATIC
        fel-protocol.c
        fes-protocol.c
        usb_layer.c
        fel-payloads.c

        $<TARGET_OBJECTS:arch-obj>
)
