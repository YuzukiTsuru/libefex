#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include <libusb.h>

#include "ending.h"
#include "usb_layer.h"
#include "efex-protocol.h"
#include "compiler.h"

int sunxi_usb_bulk_send(void *handle, const int ep, const char *buf, ssize_t len) {
    libusb_device_handle *hdl = (libusb_device_handle *) handle;
    const size_t max_chunk = 128 * 1024;
    int bytes;

    while (len > 0) {
        const size_t chunk = len < max_chunk ? len : max_chunk;

        sunxi_usb_hex_dump(buf, chunk, "SEND");

        const int r = libusb_bulk_transfer(hdl, ep, (void *) buf, (int) chunk, &bytes, DEFAULT_USB_TIMEOUT);
        if (r != 0) {
            fprintf(stderr, "USB bulk send failed with error code %d reason %s\n", r, libusb_error_name(r));
            return -1;
        }
        len -= bytes;
        buf += bytes;
    }
    return 0;
}

int sunxi_usb_bulk_recv(void *handle, const int ep, char *buf, ssize_t len) {
    libusb_device_handle *hdl = (libusb_device_handle *) handle;
    int bytes;

    while (len > 0) {
        const int r = libusb_bulk_transfer(hdl, ep, (uint8_t *)buf, (int) len, &bytes, DEFAULT_USB_TIMEOUT);
        if (r != 0) {
            fprintf(stderr, "USB bulk receive failed with error code %d reason %s\n", r, libusb_error_name(r));
            return -1;
        }

        sunxi_usb_hex_dump(buf, (size_t)bytes, "RECV");

        len -= bytes;
        buf += bytes;
    }
    return 0;
}

int sunxi_scan_usb_device(struct sunxi_efex_ctx_t *ctx) {
    libusb_device **list = NULL;
    libusb_context *context = NULL;
    int device_found = 0;

    libusb_init(&context);
    const size_t count = libusb_get_device_list(context, &list);
    for (size_t i = 0; i < count; i++) {
        libusb_device *device = list[i];
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(device, &desc) != 0) {
            fprintf(stderr, "ERROR: Can't get device list\r\n");
            return -1;
        }
        if (desc.idVendor == SUNXI_USB_VENDOR && desc.idProduct == SUNXI_USB_PRODUCT) {
            // Cast to correct type for libusb_open
            libusb_device_handle *libusb_hdl = NULL;
            if (libusb_open(device, &libusb_hdl) != 0) {
                fprintf(stderr, "ERROR: Can't connect to device\r\n");
                return -1;
            }
            ctx->hdl = libusb_hdl; // Assign to void* after successful open
            device_found = 1;
            break;
        }
    }
    return device_found;
}


int sunxi_usb_init(struct sunxi_efex_ctx_t *ctx) {
    if (ctx && ctx->hdl) {
        libusb_device_handle *libusb_hdl = (libusb_device_handle *)ctx->hdl;
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
                return 1;
            }
        }
    }
    return -1;
}

int sunxi_usb_exit(struct sunxi_efex_ctx_t *ctx) {
    if (ctx && ctx->hdl) {
        libusb_device_handle *libusb_hdl = (libusb_device_handle *)ctx->hdl;
        libusb_close(libusb_hdl);
        ctx->hdl = NULL;
        return 0;
    }
    return -1;
}
