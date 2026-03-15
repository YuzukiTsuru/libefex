#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libusb.h>

#include "compiler.h"
#include "efex-common.h"
#include "efex-protocol.h"
#include "efex-usb.h"
#include "ending.h"
#include "usb_layer.h"

static int libusb_bulk_send(void *handle, int ep, const char *buf, ssize_t len) {
	if (!handle || !buf || len <= 0) {
		return EFEX_ERR_NULL_PTR;
	}

	libusb_device_handle *hdl = (libusb_device_handle *) handle;
	const size_t max_chunk = 128 * 1024;
	int bytes;

	while (len > 0) {
		const size_t chunk = (size_t)len < max_chunk ? (size_t)len : max_chunk;

		sunxi_usb_hex_dump(buf, chunk, "SEND");

		const int r = libusb_bulk_transfer(hdl, ep, (void *) buf, (int) chunk, &bytes, DEFAULT_USB_TIMEOUT);
		if (r != 0) {
			return EFEX_ERR_USB_TRANSFER;
		}
		len -= bytes;
		buf += bytes;
	}
	return EFEX_ERR_SUCCESS;
}

static int libusb_bulk_recv(void *handle, int ep, char *buf, ssize_t len) {
	if (!handle || !buf || len <= 0) {
		return EFEX_ERR_NULL_PTR;
	}

	libusb_device_handle *hdl = (libusb_device_handle *) handle;
	int bytes;

	while (len > 0) {
		const int r = libusb_bulk_transfer(hdl, ep, (uint8_t *) buf, (int) len, &bytes, DEFAULT_USB_TIMEOUT);
		if (r != 0) {
			return EFEX_ERR_USB_TRANSFER;
		}

		sunxi_usb_hex_dump(buf, (size_t) bytes, "RECV");

		len -= bytes;
		buf += bytes;
	}
	return EFEX_ERR_SUCCESS;
}

static int libusb_scan_device(struct sunxi_efex_ctx_t *ctx) {
	if (!ctx) {
		return EFEX_ERR_NULL_PTR;
	}

	libusb_device **list = NULL;
	libusb_context *context = NULL;

	libusb_init(&context);
	ctx->usb_context = context;
	const size_t count = libusb_get_device_list(context, &list);
	for (size_t i = 0; i < count; i++) {
		libusb_device *device = list[i];
		struct libusb_device_descriptor desc;
		if (libusb_get_device_descriptor(device, &desc) != 0) {
			return EFEX_ERR_USB_DEVICE_NOT_FOUND;
		}
		if (desc.idVendor == SUNXI_USB_VENDOR && desc.idProduct == SUNXI_USB_PRODUCT) {
			libusb_device_handle *libusb_hdl = NULL;
			if (libusb_open(device, &libusb_hdl) != 0) {
				fprintf(stderr, "ERROR: Can't connect to device\r\n");
				return EFEX_ERR_USB_INIT;
			}
			ctx->hdl = libusb_hdl;
			return EFEX_ERR_SUCCESS;
		}
	}
	return EFEX_ERR_USB_DEVICE_NOT_FOUND;
}

static int libusb_scan_devices(struct sunxi_scanned_device_t **devices, size_t *count) {
	if (!devices || !count) {
		return EFEX_ERR_NULL_PTR;
	}

	*devices = NULL;
	*count = 0;

	libusb_device **list = NULL;
	libusb_context *context = NULL;

	int r = libusb_init(&context);
	if (r < 0) {
		return EFEX_ERR_USB_INIT;
	}

	const ssize_t device_count = libusb_get_device_list(context, &list);
	if (device_count < 0) {
		libusb_exit(context);
		return EFEX_ERR_USB_DEVICE_NOT_FOUND;
	}

	size_t found_count = 0;
	for (ssize_t i = 0; i < device_count; i++) {
		libusb_device *device = list[i];
		struct libusb_device_descriptor desc;
		if (libusb_get_device_descriptor(device, &desc) != 0) {
			continue;
		}
		if (desc.idVendor == SUNXI_USB_VENDOR && desc.idProduct == SUNXI_USB_PRODUCT) {
			found_count++;
		}
	}

	if (found_count == 0) {
		libusb_free_device_list(list, 1);
		libusb_exit(context);
		return EFEX_ERR_USB_DEVICE_NOT_FOUND;
	}

	struct sunxi_scanned_device_t *result = (struct sunxi_scanned_device_t *)malloc(
		sizeof(struct sunxi_scanned_device_t) * found_count);
	if (!result) {
		libusb_free_device_list(list, 1);
		libusb_exit(context);
		return EFEX_ERR_MEMORY;
	}

	size_t idx = 0;
	for (ssize_t i = 0; i < device_count && idx < found_count; i++) {
		libusb_device *device = list[i];
		struct libusb_device_descriptor desc;
		if (libusb_get_device_descriptor(device, &desc) != 0) {
			continue;
		}
		if (desc.idVendor == SUNXI_USB_VENDOR && desc.idProduct == SUNXI_USB_PRODUCT) {
			result[idx].bus = libusb_get_bus_number(device);
			result[idx].port = libusb_get_port_number(device);
			result[idx].vid = desc.idVendor;
			result[idx].pid = desc.idProduct;
			idx++;
		}
	}

	libusb_free_device_list(list, 1);
	libusb_exit(context);

	*devices = result;
	*count = found_count;
	return EFEX_ERR_SUCCESS;
}

static int libusb_scan_device_at(struct sunxi_efex_ctx_t *ctx, uint8_t bus, uint8_t port) {
	if (!ctx) {
		return EFEX_ERR_NULL_PTR;
	}

	libusb_device **list = NULL;
	libusb_context *context = NULL;

	libusb_init(&context);
	ctx->usb_context = context;
	const size_t count = libusb_get_device_list(context, &list);
	for (size_t i = 0; i < count; i++) {
		libusb_device *device = list[i];
		struct libusb_device_descriptor desc;
		if (libusb_get_device_descriptor(device, &desc) != 0) {
			continue;
		}
		if (desc.idVendor == SUNXI_USB_VENDOR && desc.idProduct == SUNXI_USB_PRODUCT) {
			uint8_t dev_bus = libusb_get_bus_number(device);
			uint8_t dev_port = libusb_get_port_number(device);
			if (dev_bus == bus && dev_port == port) {
				libusb_device_handle *libusb_hdl = NULL;
				if (libusb_open(device, &libusb_hdl) != 0) {
					fprintf(stderr, "ERROR: Can't connect to device at bus=%d port=%d\r\n", bus, port);
					libusb_free_device_list(list, 1);
					return EFEX_ERR_USB_INIT;
				}
				ctx->hdl = libusb_hdl;
				libusb_free_device_list(list, 1);
				return EFEX_ERR_SUCCESS;
			}
		}
	}
	libusb_free_device_list(list, 1);
	return EFEX_ERR_USB_DEVICE_NOT_FOUND;
}

static int libusb_backend_init(struct sunxi_efex_ctx_t *ctx) {
	if (ctx && ctx->hdl) {
		libusb_device_handle *libusb_hdl = (libusb_device_handle *) ctx->hdl;
		struct libusb_config_descriptor *config;

		if (libusb_kernel_driver_active(libusb_hdl, 0))
			libusb_detach_kernel_driver(libusb_hdl, 0);

		if (libusb_claim_interface(libusb_hdl, 0) == 0) {
			if (libusb_get_active_config_descriptor(libusb_get_device(libusb_hdl), &config) == 0) {
				for (int if_idx = 0; if_idx < config->bNumInterfaces; if_idx++) {
					const struct libusb_interface *iface = config->interface + if_idx;
					for (int set_idx = 0; set_idx < iface->num_altsetting; set_idx++) {
						const struct libusb_interface_descriptor *setting = iface->altsetting + set_idx;
						for (int ep_idx = 0; ep_idx < setting->bNumEndpoints; ep_idx++) {
							const struct libusb_endpoint_descriptor *ep = setting->endpoint + ep_idx;
							if ((ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) != LIBUSB_TRANSFER_TYPE_BULK)
								continue;
							if ((ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN)
								ctx->epin = ep->bEndpointAddress;
							else
								ctx->epout = ep->bEndpointAddress;
						}
					}
				}
				libusb_free_config_descriptor(config);
				return EFEX_ERR_SUCCESS;
			}
		}
	}
	return EFEX_ERR_USB_INIT;
}

static int libusb_backend_exit(struct sunxi_efex_ctx_t *ctx) {
	if (!ctx) {
		return EFEX_ERR_NULL_PTR;
	}

	if (ctx->hdl) {
		libusb_device_handle *libusb_hdl = (libusb_device_handle *) ctx->hdl;
		libusb_release_interface(libusb_hdl, 0);
		libusb_close(libusb_hdl);
		ctx->hdl = NULL;
	}

	if (ctx->usb_context) {
		libusb_exit((libusb_context *) ctx->usb_context);
		ctx->usb_context = NULL;
	}

	return EFEX_ERR_SUCCESS;
}

const struct usb_backend_ops usb_libusb_ops = {
	.bulk_send = libusb_bulk_send,
	.bulk_recv = libusb_bulk_recv,
	.scan_device = libusb_scan_device,
	.scan_device_at = libusb_scan_device_at,
	.scan_devices = libusb_scan_devices,
	.init = libusb_backend_init,
	.exit = libusb_backend_exit,
};
