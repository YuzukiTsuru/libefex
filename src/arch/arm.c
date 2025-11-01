#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "efex-fel.h"
#include "efex-payloads.h"
#include "efex-usb.h"
#include "ending.h"


// Function to read a 32-bit value from the specified address for ARMv7
static int payloads_readl(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr, uint32_t *val) {
	// payload array containing ARMv7 machine code instructions for reading a value from memory
	/* Note: Do NOT declare this array as 'static'. Some Windows drivers cannot access payload symbols marked static;
	 * keep it non-static to ensure data visibility when writing to device memory via FEL.
	 */
	const uint32_t payload[] = {
			WARP_INST(0b11100011101000000000000000000000), /* mov r0, #0 */
			WARP_INST(0b11101110000010000000111100010111), /* mcr 15, 0, r0, cr8, cr7, {0} */
			WARP_INST(0b11101110000001110000111100010101), /* mcr 15, 0, r0, cr7, cr5, {0} */
			WARP_INST(0b11101110000001110000111111010101), /* mcr 15, 0, r0, cr7, cr5, {6} */
			WARP_INST(0b11101110000001110000111110011010), /* mcr 15, 0, r0, cr7, cr10, {4} */
			WARP_INST(0b11101110000001110000111110010101), /* mcr 15, 0, r0, cr7, cr5, {4} */
			WARP_INST(0b11101010111111111111111111111111), /* b 0x4 */
			WARP_INST(0b11100101100111110000000000001100), /* ldr r0, [pc, #12] */
			WARP_INST(0b11100010100011110001000000001100), /* add r1, pc, #12 */
			WARP_INST(0b11100101100100000010000000000000), /* ldr r2, [r0] */
			WARP_INST(0b11100101100000010010000000000000), /* str r2, [r1] */
			WARP_INST(0b11100001001011111111111100011110), /* bx lr */
			// Placeholder comments for var_addr and var_value, will fill by sunxi_fel_write_memory
			/* WARP_INST(0b00000000000000000000000000000000), uint32_t var_addr */
			/* WARP_INST(0b11111111111111111111111111111111), uint32_t var_value */
	};

	// Check if val pointer is valid
	if (!val) {
		return EFEX_ERR_NULL_PTR;
	}

	// Convert address to little-endian format before writing to memory
	uint32_t addr_le32 = cpu_to_le32(addr);
	uint32_t tmp_val = 0;
	int ret = EFEX_ERR_SUCCESS;

	// Write the payload to the specified memory address in the context
	ret = sunxi_efex_fel_write(ctx, ctx->resp.data_start_address, (void *) payload, sizeof(payload));
	if (ret != EFEX_ERR_SUCCESS) {
		return ret;
	}

	// Write the address to be read from into memory
	ret = sunxi_efex_fel_write(ctx, ctx->resp.data_start_address + sizeof(payload), (void *) &addr_le32,
							   sizeof(addr_le32));
	if (ret != EFEX_ERR_SUCCESS) {
		return ret;
	}

	// Execute the ARMv732 instructions starting at the given memory address
	ret = sunxi_efex_fel_exec(ctx, ctx->resp.data_start_address);
	if (ret != EFEX_ERR_SUCCESS) {
		return ret;
	}

	// Read the value from memory after execution
	ret = sunxi_efex_fel_read(ctx, ctx->resp.data_start_address + sizeof(payload) + sizeof(addr_le32),
							  (void *) &tmp_val, sizeof(tmp_val));
	if (ret != EFEX_ERR_SUCCESS) {
		return ret;
	}

	// Store the value read from memory after converting it to host byte order
	*val = le32_to_cpu(tmp_val);

	// Return success
	return EFEX_ERR_SUCCESS;
}

// Function to write a 32-bit value to the specified address for ARMv7
static int payloads_writel(const struct sunxi_efex_ctx_t *ctx, const uint32_t value, const uint32_t addr) {
	// payload array containing ARMv7 machine code instructions for writing a value to memory
	/* Note: Do NOT declare this array as 'static'. Some Windows drivers cannot access payload symbols marked static;
	 * keep it non-static to ensure data visibility when writing to device memory via FEL.
	 */
	const uint32_t payload[] = {
			WARP_INST(0b11100011101000000000000000000000), /* mov r0, #0 */
			WARP_INST(0b11101110000010000000111100010111), /* mcr 15, 0, r0, cr8, cr7, {0} */
			WARP_INST(0b11101110000001110000111100010101), /* mcr 15, 0, r0, cr7, cr5, {0} */
			WARP_INST(0b11101110000001110000111111010101), /* mcr 15, 0, r0, cr7, cr5, {6} */
			WARP_INST(0b11101110000001110000111110011010), /* mcr 15, 0, r0, cr7, cr10, {4} */
			WARP_INST(0b11101110000001110000111110010101), /* mcr 15, 0, r0, cr7, cr5, {4} */
			WARP_INST(0b11101010111111111111111111111111), /* b 0x4 */
			WARP_INST(0b11100101100111110000000000001000), /* ldr r0, [pc, #8] */
			WARP_INST(0b11100101100111110001000000001000), /* ldr r1, [pc, #8] */
			WARP_INST(0b11100101100000000001000000000000), /* str r1, [r0] */
			WARP_INST(0b11100001001011111111111100011110), /* bx lr */
			// Placeholder comments for var_addr and var_value, will fill by sunxi_fel_write_memory
			/* WARP_INST(0b00000000000000000000000000000000), uint32_t var_addr */
			/* WARP_INST(0b11111111111111111111111111111111), uint32_t var_value */
	};

	// Convert address and value to little-endian format before writing to memory
	const uint32_t params[2] = {
			cpu_to_le32(addr),
			cpu_to_le32(value),
	};

	int ret = EFEX_ERR_SUCCESS;

	// Write the payload to the specified memory address in the context
	ret = sunxi_efex_fel_write(ctx, ctx->resp.data_start_address, (void *) payload, sizeof(payload));
	if (ret != EFEX_ERR_SUCCESS) {
		return ret;
	}

	// Write the parameters (address and value) to memory
	ret = sunxi_efex_fel_write(ctx, ctx->resp.data_start_address + sizeof(payload), (void *) params, sizeof(params));
	if (ret != EFEX_ERR_SUCCESS) {
		return ret;
	}

	// Execute the ARMv732 instructions starting at the given memory address
	ret = sunxi_efex_fel_exec(ctx, ctx->resp.data_start_address);
	if (ret != EFEX_ERR_SUCCESS) {
		return ret;
	}

	return EFEX_ERR_SUCCESS;
}

// Structure defining the operations for the riscv_ops platform
struct payloads_ops arm_ops = {
		.name = "arm32",
		.arch = ARCH_ARM32,
		.readl = payloads_readl,
		.writel = payloads_writel,
};
