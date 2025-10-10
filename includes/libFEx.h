#ifndef FEX_LIBFEX_H
#define FEX_LIBFEX_H

#include <stdint.h>
#include <libusb.h>

struct sunxi_fel_device_resp_t {
    char magic[8];
    uint32_t id;
    uint32_t firmware;
    uint16_t mode;
    uint8_t data_flag;
    uint8_t data_length;
    uint32_t data_start_address;
    uint8_t reserved[8];
};

struct sunxi_fel_ctx_t {
    libusb_device_handle *hdl;
    int epout;
    int epin;
    struct sunxi_fel_device_resp_t resp;
};

#endif //FEX_LIBFEX_H