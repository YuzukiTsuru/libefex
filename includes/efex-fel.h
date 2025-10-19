#ifndef LIBEFEX_EFEX_FEL_H
#define LIBEFEX_EFEX_FEL_H

#include "efex-protocol.h"

/**
 * @brief Execute a command at the given address.
 *
 * This function executes the specified command at the given memory address.
 * The execution will depend on the chip and context configurations.
 *
 * @param[in] ctx Pointer to the context structure.
 * @param[in] addr Address where the command will be executed.
 */
void sunxi_efex_fel_exec(const struct sunxi_efex_ctx_t *ctx, uint32_t addr);

/**
 * @brief Read a 32-bit value from the specified memory address.
 *
 * This function reads a 32-bit value from the memory at the given address.
 *
 * @param[in] ctx Pointer to the context structure.
 * @param[in] addr The memory address from which the value will be read.
 *
 * @return The 32-bit value read from the memory address.
 */
uint32_t sunxi_efex_fel_readl(const struct sunxi_efex_ctx_t *ctx, uint32_t addr);

/**
 * @brief Write a 32-bit value to the specified memory address.
 *
 * This function writes a 32-bit value to the specified memory address.
 *
 * @param[in] ctx Pointer to the context structure.
 * @param[in] val The 32-bit value to write to the memory address.
 * @param[in] addr The memory address where the value will be written.
 */
void sunxi_efex_fel_writel(const struct sunxi_efex_ctx_t *ctx, uint32_t val, uint32_t addr);

/**
 * @brief Read a block of memory from the specified address.
 *
 * This function reads a block of memory from the specified address into the provided buffer.
 *
 * @param[in] ctx Pointer to the context structure.
 * @param[in] addr The memory address from which data will be read.
 * @param[out] buf Pointer to the buffer where the data will be stored.
 * @param[in] len The number of bytes to read from the memory.
 */
void sunxi_efex_fel_read_memory(const struct sunxi_efex_ctx_t *ctx, uint32_t addr, const char *buf, ssize_t len);

/**
 * @brief Write a block of memory to the specified address.
 *
 * This function writes a block of memory to the specified address from the provided buffer.
 *
 * @param[in] ctx Pointer to the context structure.
 * @param[in] addr The memory address to which data will be written.
 * @param[in] buf Pointer to the buffer containing the data to be written.
 * @param[in] len The number of bytes to write to the memory.
 */
void sunxi_efex_fel_write_memory(const struct sunxi_efex_ctx_t *ctx, uint32_t addr, const char *buf, ssize_t len);

#endif //LIBEFEX_EFEX_FEL_H