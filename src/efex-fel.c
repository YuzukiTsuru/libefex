#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "ending.h"
#include "efex-usb.h"
#include "efex-common.h"
#include "efex-protocol.h"

static void sunxi_efex_write_wrapper(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr, const void *buf,
                                     const size_t len) {
    if (ctx->resp.mode != DEVICE_MODE_FEL) {
        fprintf(stderr, "Device is not in FEL mode, cannot perform FEL write\n");
        return;
    }
    sunxi_send_efex_request(ctx, EFEX_CMD_FEL_WRITE, addr, (uint32_t) len);
    sunxi_usb_write(ctx, buf, len);
    sunxi_read_efex_status(ctx);
}

static void sunxi_efex_fel_read_wrapper(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr, const void *buf,
                                        const size_t len) {
    if (ctx->resp.mode != DEVICE_MODE_FEL) {
        fprintf(stderr, "Device is not in FEL mode, cannot perform FEL read\n");
        return;
    }
    sunxi_send_efex_request(ctx, EFEX_CMD_FEL_READ, addr, (uint32_t) len);
    sunxi_usb_read(ctx, buf, len);
    sunxi_read_efex_status(ctx);
}

void sunxi_efex_fel_exec(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr) {
    if (ctx->resp.mode != DEVICE_MODE_FEL) {
        fprintf(stderr, "Device is not in FEL mode, cannot perform FEL exec\n");
        return;
    }
    sunxi_send_efex_request(ctx, EFEX_CMD_FEL_EXEC, addr, 0);
    sunxi_read_efex_status(ctx);
}

uint32_t sunxi_efex_fel_readl(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr) {
    static uint32_t val = 0;
    sunxi_efex_fel_read_wrapper(ctx, addr, &val, sizeof(uint32_t));
    return val;
}

void sunxi_efex_fel_writel(const struct sunxi_efex_ctx_t *ctx, const uint32_t val, const uint32_t addr) {
    sunxi_efex_write_wrapper(ctx, addr, &val, sizeof(uint32_t));
}

void sunxi_efex_fel_read_memory(const struct sunxi_efex_ctx_t *ctx, uint32_t addr, const char *buf, ssize_t len) {
    while (len > 0) {
        const uint32_t n = len > 65536 ? 65536 : (uint32_t) len;
        sunxi_efex_fel_read_wrapper(ctx, addr, buf, n);
        addr += n;
        buf += n;
        len -= n;
    }
}

void sunxi_efex_fel_write_memory(const struct sunxi_efex_ctx_t *ctx, uint32_t addr, const char *buf, ssize_t len) {
    while (len > 0) {
        const uint32_t n = len > 65536 ? 65536 : (uint32_t) len;
        sunxi_efex_write_wrapper(ctx, addr, buf, n);
        addr += n;
        buf += n;
        len -= n;
    }
}
