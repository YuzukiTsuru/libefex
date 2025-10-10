#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "ending.h"
#include "usb_layer.h"
#include "fel-protocol.h"

int sunxi_usb_bulk_send(libusb_device_handle *hdl, const int ep, const char *buf, size_t len) {
    const size_t max_chunk = 128 * 1024;
    int bytes;

    while (len > 0) {
        const size_t chunk = len < max_chunk ? len : max_chunk;
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

int sunxi_usb_bulk_recv(libusb_device_handle *hdl, const int ep, char *buf, size_t len) {
    int bytes;

    while (len > 0) {
        const int r = libusb_bulk_transfer(hdl, ep, (uint8_t *) buf, (int) len, &bytes, DEFAULT_USB_TIMEOUT);
        if (r != 0) {
            fprintf(stderr, "USB bulk receive failed with error code %d reason %s\n", r, libusb_error_name(r));
            return -1;
        }
        len -= bytes;
        buf += bytes;
    }
    return 0;
}

int sunxi_send_usb_request(const struct sunxi_fel_ctx_t *ctx, const enum sunxi_fel_usb_request_t type,
                           const size_t length) {
    struct sunxi_usb_request_t req = {
        .magic = SUNXI_USB_REQ_MAGIC,
        .tab = 0x0,
        .data_length = cpu_to_le32(length),
        .cmd_length = SUNXI_FEL_CMD_LEN,
        .cmd_package[0] = type
    };
    req.cmd_length = (uint8_t) req.data_length;

    const int ret = sunxi_usb_bulk_send(ctx->hdl, ctx->epout, (const char *) &req, sizeof(struct sunxi_usb_request_t));
    if (ret != 0) {
        fprintf(stderr, "Failed to send USB request\n");
        return -1;
    }
    return 0;
}

int sunxi_read_usb_response(const struct sunxi_fel_ctx_t *ctx) {
    struct sunxi_usb_response_t resp = {0};

    const int ret = sunxi_usb_bulk_recv(ctx->hdl, ctx->epin, (char *) &resp, sizeof(resp));
    if (ret != 0) {
        fprintf(stderr, "Failed to read USB response\n");
        return -1;
    }

    if (strncmp(resp.magic, SUNXI_USB_RSP_MAGIC, 4) != 0) {
        fprintf(stderr, "Unexpected USB response: 0x%08x -> %s\n", resp.magics, resp.magic);
        return -1;
    }

    return resp.status;
}

int sunxi_usb_write(const struct sunxi_fel_ctx_t *ctx, const void *buf, const size_t len) {
    int ret = sunxi_send_usb_request(ctx, AW_USB_WRITE, len);
    if (ret != 0) {
        fprintf(stderr, "Failed to send USB request for writing\n");
        return ret;
    }

    ret = sunxi_usb_bulk_send(ctx->hdl, ctx->epout, buf, len);
    if (ret != 0) {
        fprintf(stderr, "Failed to write USB data\n");
        return ret;
    }

    ret = sunxi_read_usb_response(ctx);
    if (ret != 0) {
        fprintf(stderr, "Failed to read USB response after writing\n");
        return ret;
    }
    return 0;
}

int sunxi_usb_read(const struct sunxi_fel_ctx_t *ctx, const void *data, const size_t len) {
    int ret = sunxi_send_usb_request(ctx, AW_USB_READ, len);
    if (ret != 0) {
        fprintf(stderr, "Failed to send USB request for reading\n");
        return ret;
    }

    ret = sunxi_usb_bulk_send(ctx->hdl, ctx->epin, data, len);
    if (ret != 0) {
        fprintf(stderr, "Failed to send USB data for reading\n");
        return ret;
    }

    ret = sunxi_read_usb_response(ctx);
    if (ret != 0) {
        fprintf(stderr, "Failed to read USB response after reading\n");
        return ret;
    }
    return 0;
}

int sunxi_scan_usb_device(struct sunxi_fel_ctx_t *ctx) {
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
            if (libusb_open(device, &ctx->hdl) != 0) {
                fprintf(stderr, "ERROR: Can't connect to device\r\n");
                return -1;
            }
            device_found = 1;
            break;
        }
    }
    return device_found;
}


int sunxi_usb_init(struct sunxi_fel_ctx_t *ctx) {
    if (ctx && ctx->hdl) {
        struct libusb_config_descriptor *config;

        if (libusb_kernel_driver_active(ctx->hdl, 0))
            libusb_detach_kernel_driver(ctx->hdl, 0);

        if (libusb_claim_interface(ctx->hdl, 0) == 0) {
            if (libusb_get_active_config_descriptor(libusb_get_device(ctx->hdl), &config) == 0) {
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
