#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "ending.h"
#include "efex-usb.h"
#include "efex-common.h"
#include "compiler.h"

int sunxi_send_efex_request(const struct sunxi_efex_ctx_t *ctx, const enum sunxi_efex_cmd_t type,
                                   const uint32_t addr, const uint32_t length) {
    const struct sunxi_efex_request_t req = {
            .cmd = cpu_to_le16(type),
            .tag = 0x0,
            .address = cpu_to_le32(addr),
            .len = cpu_to_le32(length)
    };
    return sunxi_usb_write(ctx, &req, sizeof(struct sunxi_efex_request_t));
}

int sunxi_read_efex_status(const struct sunxi_efex_ctx_t *ctx) {
    const struct sunxi_efex_response_t resp = {0};
    const int ret = sunxi_usb_read(ctx, &resp, sizeof(resp));
    if (ret != 0) {
        fprintf(stderr, "Failed to read USB data for chip discovery\n");
        return -1;
    }
    return resp.status;
}

enum sunxi_verify_device_mode_t sunxi_efex_get_device_mode(const struct sunxi_efex_ctx_t *ctx) {
    return ctx->resp.mode;
}

const char *sunxi_efex_get_device_mode_str(const struct sunxi_efex_ctx_t *ctx) {
    const enum sunxi_verify_device_mode_t mode = sunxi_efex_get_device_mode(ctx);

    switch (mode) {
        case DEVICE_MODE_NULL:
            return "DEVICE_MODE_NULL";
        case DEVICE_MODE_FEL:
            return "DEVICE_MODE_FEL";
        case DEVICE_MODE_SRV:
            return "DEVICE_MODE_SRV";
        case DEVICE_MODE_UPDATE_COOL:
            return "DEVICE_MODE_UPDATE_COOL";
        case DEVICE_MODE_UPDATE_HOT:
            return "DEVICE_MODE_UPDATE_HOT";
        default:
            return "UNKNOWN_DEVICE_MODE";
    }
}

int sunxi_efex_init(struct sunxi_efex_ctx_t *ctx) {
    // Send request to find chips
    sunxi_send_efex_request(ctx, EFEX_CMD_VERIFY_DEVICE, 0, 0);

    // Read response
    const int ret = sunxi_usb_read(ctx, &ctx->resp, sizeof(ctx->resp));
    if (ret != 0) {
        fprintf(stderr, "Failed to read USB data for chip discovery\n");
        return -1;
    }

    // Read status
    sunxi_read_efex_status(ctx);

    // Process the chip data
    ctx->resp.id = le32_to_cpu(ctx->resp.id);
    ctx->resp.firmware = le32_to_cpu(ctx->resp.firmware);
    ctx->resp.mode = le16_to_cpu(ctx->resp.mode);
    ctx->resp.data_start_address = le32_to_cpu(ctx->resp.data_start_address);
    ctx->resp.data_length = ctx->resp.data_length;
    ctx->resp.data_flag = ctx->resp.data_flag;

    return 0;
}