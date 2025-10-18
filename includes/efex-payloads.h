#ifndef EFEX_PAYLOADS_H
#define EFEX_PAYLOADS_H

#include <stdint.h>
#include "libefex.h"
#include "compiler.h"

enum sunxi_efex_fel_payloads_arch {
    ARCH_ARM32,
    ARCH_AARCH64,
    ARCH_RISCV32_E907,
};

/**
 * @struct payloads_ops
 * @brief Structure that defines the operations for reading and writing payload data.
 *
 * This structure encapsulates the necessary operations for interacting with payloads,
 * including the architecture type and read/write functions. These operations are
 * typically used in contexts like payload processing and device management.
 */
struct payloads_ops {
    /**
     * @brief Name of the payload operations.
     *
     * This field holds the name of the payload operation in a string format.
     */
    const char name[32];

    /**
     * @brief Architecture type of the payload.
     *
     * This field specifies the architecture type of the payload.
     * It uses an enumeration type, sunxi_efex_payloads_arch, to define different supported architectures.
     */
    enum sunxi_efex_fel_payloads_arch arch;

    /**
     * @brief Function to read a 32-bit value from the given address.
     *
     * This function pointer is used to read a 32-bit value from a specific memory address
     * in the context of the provided sunxi_efex_ctx_t. It is typically used to retrieve
     * data or register values from the payload.
     *
     * @param ctx Pointer to the context structure containing relevant state.
     * @param addr Memory address from which the value is to be read.
     * @return The 32-bit value read from the specified address.
     */
    uint32_t (*readl)(const struct sunxi_efex_ctx_t *ctx, uint32_t addr);

    /**
     * @brief Function to write a 32-bit value to the given address.
     *
     * This function pointer is used to write a 32-bit value to a specific memory address
     * in the context of the provided sunxi_efex_ctx_t. It is typically used to set or modify
     * data or registers in the payload.
     *
     * @param ctx Pointer to the context structure containing relevant state.
     * @param value The 32-bit value to be written.
     * @param addr Memory address where the value should be written.
     */
    void (*writel)(const struct sunxi_efex_ctx_t *ctx, uint32_t value, uint32_t addr);
};

/**
 * @macro WARP_INST(x)
 * @brief Converts the input value to its byte-swapped representation.
 *
 * This macro is used to convert a 32-bit value into its byte-swapped format.
 * The byte-swapping is useful when dealing with different endianness between the
 * host system and the target system.
 *
 * @param x The value to be byte-swapped.
 * @return The byte-swapped 32-bit value.
 */
#define WARP_INST(x) (SWAB32((uint32_t)(x)))

/**
 * @brief Initializes the payloads for the given architecture.
 *
 * This function sets up the necessary payloads based on the provided architecture type.
 * It will configure the correct operations to interact with the hardware, such as reading
 * or writing to memory based on the chosen architecture.
 *
 * @param arch The architecture type to initialize payloads for.
 *             This can be an enum value representing a specific architecture (e.g., ARM, RISC-V).
 */
void sunxi_efex_fel_payloads_init(enum sunxi_efex_fel_payloads_arch arch);

/**
 * @brief Retrieves the current payload operations.
 *
 * This function returns a pointer to the current payload operations, which includes the
 * functions for reading and writing memory specific to the active architecture.
 *
 * @return A pointer to a structure containing the current payload operations.
 */
struct payloads_ops *sunxi_efex_fel_get_current_payload();

/**
 * @brief Reads a 32-bit value from the specified address.
 *
 * This function uses the current payloads to read a 32-bit value from the specified memory address.
 * It interacts with the hardware through a low-level interface to fetch the value stored at the address.
 *
 * @param ctx The context structure that holds the necessary information for the operation.
 * @param addr The address from which to read the 32-bit value.
 *
 * @return The 32-bit value read from the specified address.
 */
uint32_t sunxi_efex_fel_payloads_readl(const struct sunxi_efex_ctx_t *ctx, uint32_t addr);

/**
 * @brief Writes a 32-bit value to the specified address.
 *
 * This function writes a 32-bit value to the specified memory address using the current payloads.
 * It interacts with the hardware to store the value at the given address.
 *
 * @param ctx The context structure that holds the necessary information for the operation.
 * @param value The 32-bit value to write to memory.
 * @param addr The address to which the value will be written.
 */
void sunxi_efex_fel_payloads_writel(const struct sunxi_efex_ctx_t *ctx, uint32_t value, uint32_t addr);

#endif //EFEX_PAYLOADS_H
