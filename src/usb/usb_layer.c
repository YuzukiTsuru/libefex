#include <stdlib.h>
#include <string.h>

#include "usb_layer.h"
#include "efex-common.h"

static enum usb_backend_type current_backend = USB_BACKEND_AUTO;

extern const struct usb_backend_ops usb_libusb_ops;
extern const struct usb_backend_ops usb_winusb_ops;

static const struct usb_backend_ops *get_backend_ops(void) {
#ifdef _WIN32
	if (current_backend == USB_BACKEND_LIBUSB) {
		return &usb_libusb_ops;
	} else {
		return &usb_winusb_ops;
	}
#else
	return &usb_libusb_ops;
#endif
}

int sunxi_usb_bulk_send(void *handle, int ep, const char *buf, ssize_t len) {
	const struct usb_backend_ops *ops = get_backend_ops();
	if (!ops || !ops->bulk_send) {
		return EFEX_ERR_NOT_SUPPORT;
	}
	return ops->bulk_send(handle, ep, buf, len);
}

int sunxi_usb_bulk_recv(void *handle, int ep, char *buf, ssize_t len) {
	const struct usb_backend_ops *ops = get_backend_ops();
	if (!ops || !ops->bulk_recv) {
		return EFEX_ERR_NOT_SUPPORT;
	}
	return ops->bulk_recv(handle, ep, buf, len);
}

int sunxi_scan_usb_device(struct sunxi_efex_ctx_t *ctx) {
	const struct usb_backend_ops *ops = get_backend_ops();
	if (!ops || !ops->scan_device) {
		return EFEX_ERR_NOT_SUPPORT;
	}
	return ops->scan_device(ctx);
}

int sunxi_usb_init(struct sunxi_efex_ctx_t *ctx) {
	const struct usb_backend_ops *ops = get_backend_ops();
	if (!ops || !ops->init) {
		return EFEX_ERR_NOT_SUPPORT;
	}
	return ops->init(ctx);
}

int sunxi_usb_exit(struct sunxi_efex_ctx_t *ctx) {
	const struct usb_backend_ops *ops = get_backend_ops();
	if (!ops || !ops->exit) {
		return EFEX_ERR_NOT_SUPPORT;
	}
	return ops->exit(ctx);
}

int sunxi_efex_set_usb_backend(enum usb_backend_type backend) {
#ifdef _WIN32
	if (backend == USB_BACKEND_LIBUSB || backend == USB_BACKEND_WINUSB || backend == USB_BACKEND_AUTO) {
		current_backend = backend;
		return EFEX_ERR_SUCCESS;
	}
#else
	if (backend == USB_BACKEND_LIBUSB || backend == USB_BACKEND_AUTO) {
		current_backend = backend;
		return EFEX_ERR_SUCCESS;
	}
#endif
	return EFEX_ERR_INVALID_PARAM;
}

enum usb_backend_type sunxi_efex_get_usb_backend(void) {
	return current_backend;
}
