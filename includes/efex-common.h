/**
 * @file efex-common.h
 * @brief Common EFEX functions and utilities
 *
 * This file provides common EFEX functions including USB device scanning,
 * device mode queries, and USB backend management.
 */

#ifndef LIBEFEX_EFEX_COMMON_H
#define LIBEFEX_EFEX_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "efex-protocol.h"
#include "libefex.h"

/**
 * @brief Send an EFEX request to the device
 *
 * Sends a request to the EFEX device with the specified command type,
 * address, and length parameters.
 *
 * @param ctx Pointer to the EFEX context structure
 * @param type Command type to send
 * @param addr Address parameter for the command
 * @param length Length parameter for the command
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_send_efex_request(const struct sunxi_efex_ctx_t *ctx, enum sunxi_efex_cmd_t type, uint32_t addr,
                            uint32_t length);

/**
 * @brief Read the EFEX status from the device
 *
 * Reads the current status from the EFEX device.
 *
 * @param ctx Pointer to the EFEX context structure
 * @return Status value on success, or an error code on failure
 */
int sunxi_read_efex_status(const struct sunxi_efex_ctx_t *ctx);

/**
 * @brief Scan for a USB device matching the specified vendor and product IDs
 *
 * Initializes the USB context, retrieves the list of connected USB devices,
 * and checks each device's descriptor to identify a device with the predefined
 * SUNXI_USB_VENDOR and SUNXI_USB_PRODUCT IDs. If such a device is found,
 * it attempts to open a connection to it and stores the handle in the provided context.
 *
 * @param ctx Pointer to the EFEX context structure
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_scan_usb_device(struct sunxi_efex_ctx_t *ctx);

/**
 * @brief Get the device mode from the EFEX context
 *
 * Retrieves the current device mode from the provided EFEX context.
 *
 * @param ctx Pointer to the EFEX context structure
 * @return The current device mode as an enumeration value
 */
enum sunxi_verify_device_mode_t sunxi_efex_get_device_mode(const struct sunxi_efex_ctx_t *ctx);

/**
 * @brief Get the device mode as a string representation
 *
 * Converts the device mode enumeration value to a human-readable string.
 *
 * @param ctx Pointer to the EFEX context structure
 * @return A null-terminated string representing the device mode
 */
const char *sunxi_efex_get_device_mode_str(const struct sunxi_efex_ctx_t *ctx);

/**
 * @brief Initialize the EFEX context
 *
 * Initializes the given context, setting up necessary configurations
 * and resources for future EFEX operations.
 *
 * @param ctx Pointer to the EFEX context structure
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_efex_init(struct sunxi_efex_ctx_t *ctx);

/**
 * @brief Get error message string for a given error code
 *
 * Returns a human-readable string description for the specified error code.
 *
 * @param error_code The error code to get the description for
 * @return A null-terminated string describing the error
 */
const char *sunxi_efex_strerror(int error_code);

#ifdef __cplusplus
}
#endif

#endif // LIBEFEX_EFEX_COMMON_H
