#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "ending.h"
#include "usb_layer.h"
#include "fel-protocol.h"
#include "compiler.h"

static int sunxi_send_fel_request(const struct sunxi_fel_ctx_t *ctx, const enum sunxi_fel_cmd_t type,
                                  const uint32_t addr, const uint32_t length) {
    const struct sunxi_fel_request_t req = {
        .cmd = cpu_to_le16(type),
        .tag = 0x0,
        .address = cpu_to_le32(addr),
        .len = cpu_to_le32(length)
    };
    return sunxi_usb_write(ctx, &req, sizeof(struct sunxi_fel_request_t));
}

static int sunxi_read_fel_status(const struct sunxi_fel_ctx_t *ctx) {
    const struct sunxi_fel_response_t resp = {0};
    const int ret = sunxi_usb_read(ctx, &resp, sizeof(resp));
    if (ret != 0) {
        fprintf(stderr, "Failed to read USB data for chip discovery\n");
        return -1;
    }
    return resp.status;
}

int sunxi_fel_init(struct sunxi_fel_ctx_t *ctx) {
    // Send request to find chips
    sunxi_send_fel_request(ctx, FEL_CMD_VERIFY_DEVICE, 0, 0);

    // Read response
    const int ret = sunxi_usb_read(ctx, &ctx->resp, sizeof(ctx->resp));
    if (ret != 0) {
        fprintf(stderr, "Failed to read USB data for chip discovery\n");
        return -1;
    }

    // Read status
    sunxi_read_fel_status(ctx);

    // Process the chip data
    ctx->resp.id = le32_to_cpu(ctx->resp.id);
    ctx->resp.firmware = le32_to_cpu(ctx->resp.firmware);
    ctx->resp.mode = le16_to_cpu(ctx->resp.mode);
    ctx->resp.data_start_address = le32_to_cpu(ctx->resp.data_start_address);
    ctx->resp.data_length = ctx->resp.data_length;
    ctx->resp.data_flag = ctx->resp.data_flag;

    return 0;
}

void sunxi_fel_exec(const struct sunxi_fel_ctx_t *ctx, const uint32_t addr) {
    sunxi_send_fel_request(ctx, FEL_CMD_EXEC, addr, 0);
    sunxi_read_fel_status(ctx);
}

static void sunxi_fel_write_wrapper(const struct sunxi_fel_ctx_t *ctx, const uint32_t addr, const void *buf,
                                    const size_t len) {
    sunxi_send_fel_request(ctx, FEL_CMD_WRITE, addr, (uint32_t) len);
    sunxi_usb_write(ctx, buf, len);
    sunxi_read_fel_status(ctx);
}

static void sunxi_fel_read_wrapper(const struct sunxi_fel_ctx_t *ctx, const uint32_t addr, const void *buf,
                                   const size_t len) {
    sunxi_send_fel_request(ctx, FEL_CMD_READ, addr, (uint32_t) len);
    sunxi_usb_read(ctx, buf, len);
    sunxi_read_fel_status(ctx);
}

uint32_t sunxi_fel_readl(const struct sunxi_fel_ctx_t *ctx, const uint32_t addr) {
    static uint32_t val = 0;
    sunxi_fel_read_wrapper(ctx, addr, &val, sizeof(uint32_t));
    return val;
}

void sunxi_fel_writel(const struct sunxi_fel_ctx_t *ctx, const uint32_t val, const uint32_t addr) {
    sunxi_fel_write_wrapper(ctx, addr, &val, sizeof(uint32_t));
}

void sunxi_fel_read_memory(const struct sunxi_fel_ctx_t *ctx, uint32_t addr, const char *buf, size_t len) {
    while (len > 0) {
        const uint32_t n = len > 65536 ? 65536 : (uint32_t) len;
        sunxi_fel_read_wrapper(ctx, addr, buf, n);
        addr += n;
        buf += n;
        len -= n;
    }
}

void sunxi_fel_write_memory(const struct sunxi_fel_ctx_t *ctx, uint32_t addr, const char *buf, size_t len) {
    while (len > 0) {
        const uint32_t n = len > 65536 ? 65536 : (uint32_t) len;
        sunxi_fel_write_wrapper(ctx, addr, buf, n);
        addr += n;
        buf += n;
        len -= n;
    }
}
