#ifndef EFEX_PROTOCOL_H
#define EFEX_PROTOCOL_H

#include <stdint.h>
#include "libefex.h"
#include "compiler.h"

enum sunxi_efex_cmd_t {
    /* Common Commands */
    EFEX_CMD_VERIFY_DEVICE = 0x0001,
    EFEX_CMD_SWITCH_ROLE = 0x0002,
    EFEX_CMD_IS_READY = 0x0003,
    EFEX_CMD_GET_CMD_SET_VER = 0x0004,
    EFEX_CMD_DISCONNECT = 0x0010,
    /* FEL Commands */
    EFEX_CMD_FEL_WRITE = 0x0101,
    EFEX_CMD_FEL_EXEC = 0x0102,
    EFEX_CMD_FEL_READ = 0x0103,
    /* FES Commands */
    EFEX_CMD_FES_TRANS = 0x0201,
    EFEX_CMD_FES_RUN = 0x0202,
    EFEX_CMD_FES_INFO = 0x0203,
    EFEX_CMD_FES_GET_MSG = 0x0204,
    EFEX_CMD_FES_UNREG_FED = 0x0205,
    EFEX_CMD_FES_DOWN = 0x0206,
    EFEX_CMD_FES_UP = 0x0207,
    EFEX_CMD_FES_VERIFY = 0x0208,
    EFEX_CMD_FES_QUERY_STORAGE = 0x0209,
    EFEX_CMD_FES_FLASH_SET_ON = 0x020A,
    EFEX_CMD_FES_FLASH_SET_OFF = 0x020B,
    EFEX_CMD_FES_VERIFY_VALUE = 0x020C,
    EFEX_CMD_FES_VERIFY_STATUS = 0x020D,
    EFEX_CMD_FES_FLASH_SIZE_PROBE = 0x020E,
    EFEX_CMD_FES_TOOL_MODE = 0x020F,
    EFEX_CMD_FES_VERIFY_UBOOT_BLK = 0x0214,
    EFEX_CMD_FES_FORCE_ERASE_FLASH = 0x0220,
    EFEX_CMD_FES_FORCE_ERASE_KEY = 0x0221,
    EFEX_CMD_FES_QUERY_SECURE = 0x0230,
    EFEX_CMD_FES_QUERY_INFO = 0x0231,
    EFEX_CMD_FES_GET_CHIPID = 0x0232
};

enum sunxi_verify_device_mode_t {
    DEVICE_MODE_NULL = 0x0,
    DEVICE_MODE_FEL = 0x1,
    DEVICE_MODE_SRV = 0x2,
    DEVICE_MODE_UPDATE_COOL = 0x3,
    DEVICE_MODE_UPDATE_HOT = 0x4,
};

EFEX_PACKED_BEGIN

struct sunxi_usb_request_t {
    union {
        char magic[4];
        uint32_t magics;
    };

    uint32_t tab;
    uint32_t data_length;
    uint16_t resvered1;
    uint8_t resvered2;
    uint8_t cmd_length;
    uint8_t cmd_package[16];
}
        EFEX_PACKED;

EFEX_PACKED_END

EFEX_PACKED_BEGIN

struct sunxi_usb_response_t {
    union {
        char magic[4];
        uint32_t magics;
    };

    uint32_t tag;
    uint32_t residue;
    uint8_t status;
}
        EFEX_PACKED;

EFEX_PACKED_END

EFEX_PACKED_BEGIN

struct sunxi_efex_request_t {
    uint16_t cmd;
    uint16_t tag;
    uint32_t address;
    uint32_t len;
    uint32_t flags;
}
        EFEX_PACKED;

EFEX_PACKED_END

EFEX_PACKED_BEGIN

struct sunxi_efex_response_t {
    uint16_t magic;
    uint16_t tag;
    uint8_t status;
    uint8_t reserve[3];
}
        EFEX_PACKED;

EFEX_PACKED_END

#define EFEX_CODE_MAX_SIZE (32 * 1024)

/**
 * @brief Scans for a USB device matching the specified vendor and product IDs.
 *
 * This function initializes the libusb context, retrieves the list of connected USB devices,
 * and checks each device's descriptor to identify a device with the predefined SUNXI_USB_VENDOR
 * and SUNXI_USB_PRODUCT IDs. If such a device is found, it attempts to open a connection to it
 * and stores the handle in the provided context.
 *
 * @param ctx A pointer to the sunxi_efex_ctx_t structure, which will hold the device handle if found.
 * @return 0 on success, or -1 if an error occurred (e.g., unable to retrieve device descriptor or
 *         open a connection to the device).
 *
 * @note This function uses the libusb library for USB device enumeration and connection.
 */
int sunxi_scan_usb_device(struct sunxi_efex_ctx_t *ctx);

/**
 * @brief Get the device mode from the EFEX context.
 *
 * This function retrieves the current device mode from the provided EFEX context.
 *
 * @param[in] ctx Pointer to the context structure.
 *
 * @return The current device mode as an enumeration value of type sunxi_verify_device_mode_t.
 */
enum sunxi_verify_device_mode_t sunxi_efex_get_device_mode(const struct sunxi_efex_ctx_t *ctx);

/**
 * @brief Get the device mode as a string representation.
 *
 * This function converts the device mode enumeration value to a human-readable string.
 *
 * @param[in] ctx Pointer to the context structure.
 *
 * @return A null-terminated string representing the device mode.
 */
const char *sunxi_efex_get_device_mode_str(const struct sunxi_efex_ctx_t *ctx);

/**
 * @brief Initialize the EFEX context.
 *
 * This function initializes the given context, setting up necessary
 * configurations and resources for future EFEX operations.
 *
 * @param[in] ctx Pointer to the context structure.
 *
 * @return Returns 0 on success, or a negative error code on failure.
 */
int sunxi_efex_init(struct sunxi_efex_ctx_t *ctx);

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

#endif //EFEX_PROTOCOL_H
