#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "ending.h"
#include "usb_layer.h"
#include "fel-payloads.h"
#include "fel-protocol.h"

// Function to read a 32-bit value from the specified address for RISC-V32 e907
static uint32_t payloads_riscv32_e907_readl(const struct sunxi_fel_ctx_t *ctx, const uint32_t addr) {
    // payload array containing RISC-V machine code instructions for reading a value from memory
    const uint32_t payload[] = {
        WARP_INST(0b00110111000000110100000000000000), /* lui t1,0x400 */ /* Load immediate value (1 << 22) into t1 */
        WARP_INST(0b01110011001000000000001101111100), /* csrs	mxstatus,t1 */
        /* Set the corresponding bit in csr mxstatus */
        WARP_INST(0b00001111000100000000000000000000), /* fence.i */
        /* Fence instruction to ensure the changes take effect */
        WARP_INST(0b01101111000000000100000000000000), /* jal pc+0x4 */ /* Jump to the main code */
        WARP_INST(0b10010111000000100000000000000000), /* auipc t0,0x0 */ /* Load zero to t0 */
        WARP_INST(0b10010011100000100000001000000010), /* addi	t0,t0,32 */ /* Load var_addr to t0 */
        WARP_INST(0b10000011101000100000001000000000), /* lw t0,0(t0) */ /* Load value at address stored in t0 */
        WARP_INST(0b10000011101000100000001000000000), /* lw t0,0(t0) */ /* Load again to avoid cache issue and align */
        WARP_INST(0b00010111000000110000000000000000), /* auipc t1,0x0 */ /* Load zero t1 */
        WARP_INST(0b00010011000000110100001100000001), /* addi	t1,t1,20 */ /* Load var_val to t0 */
        WARP_INST(0b00100011001000000101001100000000), /* sw t0,0(t1) */
        /* Store the value in t0 into the address stored in t1 */
        WARP_INST(0b01100111100000000000000000000000), /* ret */ /* Return from the function */
        // Placeholder comments for var_addr and var_value, will fill by sunxi_fel_write_memory
        /* WARP_INST(0b00000000000000000000000000000000), uint32_t var_addr */
        /* WARP_INST(0b11111111111111111111111111111111), uint32_t var_value */
    };

    // Convert address to little-endian format before writing to memory
    uint32_t addr_le32 = cpu_to_le32(addr);
    uint32_t val = 0;

    // Write the payload to the specified memory address in the context
    sunxi_fel_write_memory(ctx, ctx->resp.data_start_address, (void *) payload, sizeof(payload));
    // Write the address to be read from into memory
    sunxi_fel_write_memory(ctx, ctx->resp.data_start_address + sizeof(payload), (void *) &addr_le32, sizeof(addr_le32));
    // Execute the RISC-V32 instructions starting at the given memory address
    sunxi_fel_exec(ctx, ctx->resp.data_start_address);
    // Read the value from memory after execution
    sunxi_fel_read_memory(ctx, ctx->resp.data_start_address + sizeof(payload) + sizeof(addr_le32), (void *) &val,
                          sizeof(val));

    // Return the value read from memory after converting it to host byte order
    return le32_to_cpu(val);
}

// Function to write a 32-bit value to the specified address for RISC-V32 e907
static void payloads_riscv32_e907_writel(const struct sunxi_fel_ctx_t *ctx, const uint32_t value, const uint32_t addr) {
    // payload array containing RISC-V machine code instructions for writing a value to memory
    const uint32_t payload[] = {
        WARP_INST(0b00110111000000110100000000000000), /* lui t1,0x400 */ /* Load immediate value (1 << 22) into t1 */
        WARP_INST(0b01110011001000000000001101111100), /* csrs	mxstatus,t1 */ /* Set the corresponding bit in csr mxstatus */
        WARP_INST(0b00001111000100000000000000000000), /* fence.i */ /* Fence instruction to ensure the changes take effect */
        WARP_INST(0b01101111000000000100000000000000), /* jal pc+0x4 */ /* Jump to the main code */
        WARP_INST(0b10010111000000100000000000000000), /* auipc t0,0x0 */ /* Load zero to t0 */
        WARP_INST(0b10010011100000100000001000000010), /* addi	t0,t0,32 */ /* Load var_addr to t0 */
        WARP_INST(0b10000011101000100000001000000000), /* lw t0,0(t0) */ /* Load the value at the address stored in t0 */
        WARP_INST(0b00010111000000110000000000000000), /* auipc t1,0x0 */ /* Load zero to t1 */
        WARP_INST(0b00010011000000111000001100000001), /* addi	t1,t1,24 */ /* Load data into t1 */
        WARP_INST(0b00000011001000110000001100000000), /* lw t1,0(t1) */ /* Load the value at the address stored in t1 */
        WARP_INST(0b00100011101000000110001000000000), /* sw t1,0(t0) */ /* Store the value in t1 into the address stored in t0 */
        WARP_INST(0b01100111100000000000000000000000), /* ret */ /* Return from the function */
        // Placeholder comments for var_addr and var_value, will fill by sunxi_fel_write_memory
        /* WARP_INST(0b00000000000000000000000000000000), uint32_t var_addr */
        /* WARP_INST(0b11111111111111111111111111111111), uint32_t var_value */
    };

    // Convert address and value to little-endian format before writing to memory
    const uint32_t params[2] = {
        cpu_to_le32(addr),
        cpu_to_le32(value),
    };

    // Write the payload to the specified memory address in the context
    sunxi_fel_write_memory(ctx, ctx->resp.data_start_address, (void *) payload, sizeof(payload));
    // Write the parameters (address and value) to memory
    sunxi_fel_write_memory(ctx, ctx->resp.data_start_address + sizeof(payload), (void *) params, sizeof(params));
    // Execute the RISC-V32 instructions starting at the given memory address
    sunxi_fel_exec(ctx, ctx->resp.data_start_address);
}

// Structure defining the operations for the riscv32_e907 platform
struct payloads_ops riscv32_e907_ops = {
    .name = "riscv32_e907",
    .arch = PAYLOAD_ARCH_RISCV32_E907,
    .readl = payloads_riscv32_e907_readl,
    .writel = payloads_riscv32_e907_writel,
};
