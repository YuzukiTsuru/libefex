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
 * @brief Scanned device information
 *
 * Contains information about a scanned USB device.
 */
struct sunxi_scanned_device_t {
	uint8_t bus;           /**< USB bus number */
	uint8_t port;          /**< USB port number */
	uint16_t vid;          /**< Vendor ID */
	uint16_t pid;          /**< Product ID */
};

/**
 * @brief Hotplug snapshot device information
 *
 * Contains backend-native device identity data used to diff hotplug state.
 * `device_path` may be NULL when the active backend does not expose a stable path.
 * `port` may be 0 when the backend cannot determine a stable port number.
 */
struct sunxi_hotplug_device_t {
	uint16_t vid;          /**< Vendor ID */
	uint16_t pid;          /**< Product ID */
	uint32_t bus_id;       /**< Backend/native USB bus identifier */
	uint32_t usb_device_id;/**< Backend/native USB device/address identifier */
	uint8_t port;          /**< Backend-native port number, 0 if unknown */
	char *device_path;     /**< Stable backend device path, caller must free through snapshot free API */
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
	int (*scan_device_at)(struct sunxi_efex_ctx_t *ctx, uint8_t bus, uint8_t port); /**< Scan for USB device at specific bus/port */
	int (*scan_devices)(struct sunxi_scanned_device_t **devices, size_t *count); /**< Scan for all USB devices */
	int (*hotplug_snapshot)(struct sunxi_hotplug_device_t **devices, size_t *count); /**< Read current hotplug device snapshot */
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
 * @brief Scan for USB device at specific bus/port
 *
 * Scans for a USB device at the specified bus and port.
 *
 * @param ctx EFEX context structure
 * @param bus USB bus number
 * @param port USB port number
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_scan_usb_device_at(struct sunxi_efex_ctx_t *ctx, uint8_t bus, uint8_t port);

/**
 * @brief Scan for all USB devices
 *
 * Scans for all USB devices matching the vendor and product IDs.
 * The caller is responsible for freeing the returned devices array.
 *
 * @param devices Pointer to receive array of scanned devices (caller must free)
 * @param count Pointer to receive number of devices found
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_scan_usb_devices(struct sunxi_scanned_device_t **devices, size_t *count);

/**
 * @brief Read the current backend hotplug snapshot
 *
 * Returns all currently visible SUNXI USB devices for the active backend. This is intended
 * for caller-side diffing to derive arrived/left events.
 *
 * @param devices Pointer to receive array of devices (caller must free with sunxi_hotplug_free_snapshot)
 * @param count Pointer to receive number of devices found
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_hotplug_snapshot(struct sunxi_hotplug_device_t **devices, size_t *count);

/**
 * @brief Free a hotplug snapshot returned by sunxi_hotplug_snapshot
 *
 * @param devices Snapshot array pointer
 * @param count Number of entries in the array
 */
void sunxi_hotplug_free_snapshot(struct sunxi_hotplug_device_t *devices, size_t count);

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
