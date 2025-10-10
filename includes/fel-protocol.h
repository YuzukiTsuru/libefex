#ifndef LIBFEL_LIBFEL_H
#define LIBFEL_LIBFEL_H

#include <stdint.h>
#include "libFEx.h"
#include "compiler.h"

enum sunxi_fel_cmd_t {
    FEL_CMD_VERIFY_DEVICE = 0x1,
    FEL_CMD_SWITCH_ROLE = 0x2,
    FEL_CMD_IS_READY = 0x3,
    FEL_CMD_GET_CMD_SET_VER = 0x4,
    FEL_CMD_DISCONNECT = 0x10,
    FEL_CMD_WRITE = 0x101,
    FEL_CMD_EXEC = 0x102,
    FEL_CMD_READ = 0x103,
};

enum sunxi_verify_device_mode_t {
    AW_DEVICE_MODE_NULL = 0x0,
    AW_DEVICE_MODE_FEL = 0x1,
    AW_DEVICE_MODE_SRV = 0x2,
    AW_DEVICE_MODE_UPDATE_COOL = 0x3,
    AW_DEVICE_MODE_UPDATE_HOT = 0x4,
};

FEX_PACKED_BEGIN
struct sunxi_usb_request_t {
    char magic[4];
    uint32_t tab;
    uint32_t data_length;
    uint16_t resvered1;
    uint8_t resvered2;
    uint8_t cmd_length;
    uint8_t cmd_package[16];
} FEX_PACKED;
FEX_PACKED_END

FEX_PACKED_BEGIN
struct sunxi_usb_response_t {
    union {
        char magic[4];
        uint32_t magics;
    };
    uint32_t tag;
    uint32_t residue;
    uint8_t status;
} FEX_PACKED;
FEX_PACKED_END

FEX_PACKED_BEGIN
struct sunxi_fel_request_t {
    uint16_t cmd;
    uint16_t tag;
    uint32_t address;
    uint32_t len;
    uint32_t flags;
} FEX_PACKED;
FEX_PACKED_END

FEX_PACKED_BEGIN
struct sunxi_fel_response_t {
    uint16_t magic;
    uint16_t tag;
    uint8_t status;
    uint8_t reserve[3];
} FEX_PACKED;
FEX_PACKED_END

/**
 * @brief Scans for a USB device matching the specified vendor and product IDs.
 *
 * This function initializes the libusb context, retrieves the list of connected USB devices,
 * and checks each device's descriptor to identify a device with the predefined SUNXI_USB_VENDOR
 * and SUNXI_USB_PRODUCT IDs. If such a device is found, it attempts to open a connection to it
 * and stores the handle in the provided context.
 *
 * @param ctx A pointer to the sunxi_fel_ctx_t structure, which will hold the device handle if found.
 * @return 0 on success, or -1 if an error occurred (e.g., unable to retrieve device descriptor or
 *         open a connection to the device).
 *
 * @note This function uses the libusb library for USB device enumeration and connection.
 */
int sunxi_scan_usb_device(struct sunxi_fel_ctx_t *ctx);

/**
 * @brief Initialize the FEL context.
 *
 * This function initializes the given context, setting up necessary
 * configurations and resources for future FEL operations.
 *
 * @param[in] ctx Pointer to the context structure.
 *
 * @return Returns 0 on success, or a negative error code on failure.
 */
int sunxi_fel_init(struct sunxi_fel_ctx_t *ctx);

/**
 * @brief Execute a command at the given address.
 *
 * This function executes the specified command at the given memory address.
 * The execution will depend on the chip and context configurations.
 *
 * @param[in] ctx Pointer to the context structure.
 * @param[in] addr Address where the command will be executed.
 */
void sunxi_fel_exec(const struct sunxi_fel_ctx_t *ctx, uint32_t addr);

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
uint32_t sunxi_fel_readl(const struct sunxi_fel_ctx_t *ctx, uint32_t addr);

/**
 * @brief Write a 32-bit value to the specified memory address.
 *
 * This function writes a 32-bit value to the specified memory address.
 *
 * @param[in] ctx Pointer to the context structure.
 * @param[in] val The 32-bit value to write to the memory address.
 * @param[in] addr The memory address where the value will be written.
 */
void sunxi_fel_writel(const struct sunxi_fel_ctx_t *ctx, uint32_t val, uint32_t addr);

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
void sunxi_fel_read_memory(const struct sunxi_fel_ctx_t *ctx, uint32_t addr, const char *buf, size_t len);

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
void sunxi_fel_write_memory(const struct sunxi_fel_ctx_t *ctx, uint32_t addr, const char *buf, size_t len);

#endif //LIBFEL_LIBFEL_H
