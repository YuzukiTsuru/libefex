#ifndef LIBEFEX_EFEX_FEL_H
#define LIBEFEX_EFEX_FEL_H

#ifdef __cplusplus
extern "C" {

#endif

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
int sunxi_efex_fel_exec(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr);

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
int sunxi_efex_fel_read(const struct sunxi_efex_ctx_t *ctx, uint32_t addr, char *buf, ssize_t len);

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
int sunxi_efex_fel_write(const struct sunxi_efex_ctx_t *ctx, uint32_t addr, const char *buf, ssize_t len);

#ifdef __cplusplus
}
#endif

#endif // LIBEFEX_EFEX_FEL_H
