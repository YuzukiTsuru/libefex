#ifndef EFEX_USB_LAYER_H
#define EFEX_USB_LAYER_H

#include "libefex.h"

#define DEFAULT_USB_TIMEOUT (10000)

enum sunxi_usb_ids {
    SUNXI_USB_VENDOR = 0x1f3a,
    SUNXI_USB_PRODUCT = 0xefe8,
};

#define SUNXI_FEL_CMD_LEN (0xc)
#define SUNXI_USB_REQ_MAGIC "AWUC"
#define SUNXI_USB_RSP_MAGIC "AWUS"
#define SUNXI_VERIFY_RSP_MAGIC "AWUSBFEX"

enum sunxi_fel_usb_request_t {
    AW_USB_READ = 0x11,
    AW_USB_WRITE = 0x12,
};

/**
 * @brief Sends data over a bulk USB endpoint.
 *
 * This function sends data to a specified bulk endpoint of a USB device. It uses the provided
 * device handle, endpoint number, and data buffer to send the data.
 *
 * @param hdl A pointer to the libusb device handle representing the open USB device.
 * @param ep The endpoint address to send the data to. Should be a valid bulk endpoint.
 * @param buf A pointer to the buffer containing the data to be sent.
 * @param len The length of the data to be sent, in bytes.
 * @return 0 on success, or a negative error code on failure.
 *
 * @note The function uses libusb_bulk_transfer to perform the bulk transfer.
 */
int sunxi_usb_bulk_send(libusb_device_handle *hdl, int ep, const char *buf, size_t len);

/**
 * @brief Receives data from a bulk USB endpoint.
 *
 * This function receives data from a specified bulk endpoint of a USB device. It uses the provided
 * device handle, endpoint number, and buffer to receive the data.
 *
 * @param hdl A pointer to the libusb device handle representing the open USB device.
 * @param ep The endpoint address to receive data from. Should be a valid bulk endpoint.
 * @param buf A pointer to the buffer where received data will be stored.
 * @param len The maximum number of bytes to receive.
 * @return The number of bytes actually received on success, or a negative error code on failure.
 *
 * @note The function uses libusb_bulk_transfer to perform the bulk transfer.
 */
int sunxi_usb_bulk_recv(libusb_device_handle *hdl, int ep, char *buf, size_t len);

/**
 * @brief Sends a USB request to the device.
 *
 * This function sends a USB request to the device specified by the given context. The request
 * type and length of the request are also specified.
 *
 * @param ctx A pointer to the sunxi_fel_ctx_t structure that contains the device context.
 * @param type The type of the USB request to be sent.
 * @param length The length of the data to be sent as part of the request.
 * @return 0 on success, or a negative error code on failure.
 *
 * @note This function uses libusb control transfer for sending requests.
 */
int sunxi_send_usb_request(const struct sunxi_fel_ctx_t *ctx, enum sunxi_fel_usb_request_t type,
                           size_t length);

/**
 * @brief Reads a USB response from the device.
 *
 * This function reads a response from the USB device after sending a request. The response
 * data will be processed or returned as needed.
 *
 * @param ctx A pointer to the sunxi_fel_ctx_t structure that contains the device context.
 * @return 0 on success, or a negative error code on failure.
 *
 * @note The function uses libusb control transfer to read the response.
 */
int sunxi_read_usb_response(const struct sunxi_fel_ctx_t *ctx);

/**
 * @brief Writes data to the USB device.
 *
 * This function writes the specified data buffer to the USB device using the provided context.
 *
 * @param ctx A pointer to the sunxi_fel_ctx_t structure that contains the device context.
 * @param buf A pointer to the buffer containing the data to be written.
 * @param len The length of the data to be written, in bytes.
 * @return 0 on success, or a negative error code on failure.
 *
 * @note This function uses libusb_bulk_transfer to perform the write operation.
 */
int sunxi_usb_write(const struct sunxi_fel_ctx_t *ctx, const void *buf, size_t len);

/**
 * @brief Reads data from the USB device.
 *
 * This function reads data from the USB device into the provided buffer. It uses the context
 * to identify the device and endpoint from which to read.
 *
 * @param ctx A pointer to the sunxi_fel_ctx_t structure that contains the device context.
 * @param data A pointer to the buffer where the data will be stored.
 * @param len The maximum number of bytes to read.
 * @return The number of bytes read on success, or a negative error code on failure.
 *
 * @note This function uses libusb_bulk_transfer to perform the read operation.
 */
int sunxi_usb_read(const struct sunxi_fel_ctx_t *ctx, const void *data, size_t len);

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
 * @brief Initializes the USB context for interacting with USB devices.
 *
 * This function sets up the necessary context for interacting with USB devices. It allocates
 * the required resources and prepares the given context structure for subsequent USB operations
 * (such as scanning devices or sending/receiving data).
 *
 * @param ctx A pointer to the sunxi_fel_ctx_t structure, which will be initialized.
 * @return 0 on success, or a negative error code on failure (e.g., unable to initialize the context).
 *
 * @note This function requires libusb to be properly installed and initialized before use.
 */
int sunxi_usb_init(struct sunxi_fel_ctx_t *ctx);

#endif //EFEX_USB_LAYER_H
