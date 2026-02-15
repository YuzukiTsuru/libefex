/**
 * @file usb_layer.h
 * @brief USB abstraction layer for libefex
 *
 * This file provides an abstraction layer for USB operations, supporting multiple backends
 * (libusb and winusb) on different platforms.
 */

#ifndef USB_LAYER_H
#define USB_LAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "efex-common.h"

/**
 * @brief USB backend type enumeration
 *
 * Defines the available USB backend implementations.
 */
enum usb_backend_type {
	USB_BACKEND_AUTO = 0,    /**< Auto-select backend (Windows: winusb, Linux/macOS: libusb) */
	USB_BACKEND_LIBUSB = 1,   /**< Force use libusb backend */
	USB_BACKEND_WINUSB = 2,   /**< Force use winusb backend (Windows only) */
};

/**
 * @brief USB backend operations structure
 *
 * Function pointers for USB backend operations. Each backend must implement
 * these functions to provide USB communication capabilities.
 */
struct usb_backend_ops {
	int (*bulk_send)(void *handle, int ep, const char *buf, ssize_t len);   /**< Send bulk data */
	int (*bulk_recv)(void *handle, int ep, char *buf, ssize_t len);   /**< Receive bulk data */
	int (*scan_device)(struct sunxi_efex_ctx_t *ctx);                 /**< Scan for USB device */
	int (*init)(struct sunxi_efex_ctx_t *ctx);                       /**< Initialize USB context */
	int (*exit)(struct sunxi_efex_ctx_t *ctx);                       /**< Cleanup USB context */
};

/**
 * @brief Send bulk data over USB
 *
 * Sends data to a USB device using the currently selected backend.
 *
 * @param handle USB device handle
 * @param ep Endpoint address
 * @param buf Buffer containing data to send
 * @param len Length of data to send
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_usb_bulk_send(void *handle, int ep, const char *buf, ssize_t len);

/**
 * @brief Receive bulk data over USB
 *
 * Receives data from a USB device using the currently selected backend.
 *
 * @param handle USB device handle
 * @param ep Endpoint address
 * @param buf Buffer to store received data
 * @param len Maximum length of data to receive
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_usb_bulk_recv(void *handle, int ep, char *buf, ssize_t len);

/**
 * @brief Scan for USB device
 *
 * Scans for a USB device matching the vendor and product IDs.
 *
 * @param ctx EFEX context structure
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_scan_usb_device(struct sunxi_efex_ctx_t *ctx);

/**
 * @brief Initialize USB context
 *
 * Initializes the USB context using the currently selected backend.
 *
 * @param ctx EFEX context structure
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_usb_init(struct sunxi_efex_ctx_t *ctx);

/**
 * @brief Cleanup USB context
 *
 * Releases USB resources using the currently selected backend.
 *
 * @param ctx EFEX context structure
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_usb_exit(struct sunxi_efex_ctx_t *ctx);

/**
 * @brief Set USB backend type
 *
 * Sets the USB backend to use for subsequent USB operations.
 * On Windows, both libusb and winusb are available.
 * On Linux/macOS, only libusb is available.
 *
 * @param backend USB backend type to use
 * @return EFEX_ERR_SUCCESS on success, or EFEX_ERR_INVALID_PARAM if backend is not supported
 */
int sunxi_efex_set_usb_backend(enum usb_backend_type backend);

/**
 * @brief Get current USB backend type
 *
 * Returns the currently selected USB backend type.
 *
 * @return Current USB backend type
 */
enum usb_backend_type sunxi_efex_get_usb_backend(void);

#ifdef __cplusplus
}
#endif

#endif // USB_LAYER_H
