#ifndef EFEX_LIBEFEX_H
#define EFEX_LIBEFEX_H

#include <stdint.h>

#if defined(_WIN32) && defined(USE_WINUSB)
#include <windows.h>
#else
#include <libusb.h>
#endif

struct sunxi_efex_device_resp_t {
    char magic[8];
    uint32_t id;
    uint32_t firmware;
    uint16_t mode;
    uint8_t data_flag;
    uint8_t data_length;
    uint32_t data_start_address;
    uint8_t reserved[8];
};

struct sunxi_efex_ctx_t {
    void *hdl;
    char *dev_name;
    int epout;
    int epin;
    struct sunxi_efex_device_resp_t resp;
};

#endif //EFEX_LIBEFEX_H