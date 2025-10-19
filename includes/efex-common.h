#ifndef LIBEFEX_EFEX_COMMON_H
#define LIBEFEX_EFEX_COMMON_H

#include "libefex.h"
#include "efex-protocol.h"

int sunxi_send_efex_request(const struct sunxi_efex_ctx_t *ctx, const enum sunxi_efex_cmd_t type,
                                   const uint32_t addr, const uint32_t length);

int sunxi_read_efex_status(const struct sunxi_efex_ctx_t *ctx);



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

#endif //LIBEFEX_EFEX_COMMON_H