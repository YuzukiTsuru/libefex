#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "ending.h"
#include "efex-usb.h"
#include "efex-common.h"
#include "efex-protocol.h"

static int sunxi_efex_write_wrapper(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr, const void *buf,
                                    const size_t len) {
    if (!buf) {
        return EFEX_ERR_NULL_PTR;
    }

    if (ctx->resp.mode != DEVICE_MODE_FEL) {
        return EFEX_ERR_INVALID_DEVICE_MODE;
    }

    int ret = sunxi_send_efex_request(ctx, EFEX_CMD_FEL_WRITE, addr, (uint32_t) len);
    if (ret != EFEX_ERR_SUCCESS) {
        return ret;
    }

    ret = sunxi_usb_write(ctx, buf, len);
    if (ret != EFEX_ERR_SUCCESS) {
        return ret;
    }

    ret = sunxi_read_efex_status(ctx);
    if (ret < 0) {
        return ret;
    }

    return EFEX_ERR_SUCCESS;
}

static int sunxi_efex_fel_read_wrapper(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr, const void *buf,
                                       const size_t len) {
    if (!buf) {
        return EFEX_ERR_NULL_PTR;
    }

    if (ctx->resp.mode != DEVICE_MODE_FEL) {
        return EFEX_ERR_INVALID_DEVICE_MODE;
    }

    int ret = sunxi_send_efex_request(ctx, EFEX_CMD_FEL_READ, addr, (uint32_t) len);
    if (ret != EFEX_ERR_SUCCESS) {
        return ret;
    }

    ret = sunxi_usb_read(ctx, buf, len);
    if (ret != EFEX_ERR_SUCCESS) {
        return ret;
    }

    ret = sunxi_read_efex_status(ctx);
    if (ret < 0) {
        return ret;
    }

    return EFEX_ERR_SUCCESS;
}

int sunxi_efex_fel_exec(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr) {
    if (!ctx) {
        return EFEX_ERR_NULL_PTR;
    }

    if (ctx->resp.mode != DEVICE_MODE_FEL) {
        return EFEX_ERR_INVALID_DEVICE_MODE;
    }

    int ret = sunxi_send_efex_request(ctx, EFEX_CMD_FEL_EXEC, addr, 0);
    if (ret != EFEX_ERR_SUCCESS) {
        return ret;
    }

    ret = sunxi_read_efex_status(ctx);
    if (ret < 0) {
        return ret;
    }

    return EFEX_ERR_SUCCESS;
}

int sunxi_efex_fel_readl(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr, uint32_t *val) {
    if (!ctx || !val) {
        return EFEX_ERR_NULL_PTR;
    }

    return sunxi_efex_fel_read_wrapper(ctx, addr, val, sizeof(uint32_t));
}

int sunxi_efex_fel_writel(const struct sunxi_efex_ctx_t *ctx, const uint32_t val, const uint32_t addr) {
    if (!ctx) {
        return EFEX_ERR_NULL_PTR;
    }

    return sunxi_efex_write_wrapper(ctx, addr, &val, sizeof(uint32_t));
}

int sunxi_efex_fel_read_memory(const struct sunxi_efex_ctx_t *ctx, uint32_t addr, const char *buf, ssize_t len) {
    if (!ctx || !buf) {
        return EFEX_ERR_NULL_PTR;
    }

    if (len <= 0) {
        return EFEX_ERR_INVALID_PARAM;
    }

    int ret = EFEX_ERR_SUCCESS;
    while (len > 0) {
        const uint32_t n = len > 65536 ? 65536 : (uint32_t) len;
        ret = sunxi_efex_fel_read_wrapper(ctx, addr, buf, n);
        if (ret != EFEX_ERR_SUCCESS) {
            break;
        }
        addr += n;
        buf += n;
        len -= n;
    }
    return ret;
}

int sunxi_efex_fel_write_memory(const struct sunxi_efex_ctx_t *ctx, uint32_t addr, const char *buf, ssize_t len) {
    if (!ctx || !buf) {
        return EFEX_ERR_NULL_PTR;
    }

    if (len <= 0) {
        return EFEX_ERR_INVALID_PARAM;
    }

    int ret = EFEX_ERR_SUCCESS;
    while (len > 0) {
        const uint32_t n = len > 65536 ? 65536 : (uint32_t) len;
        ret = sunxi_efex_write_wrapper(ctx, addr, buf, n);
        if (ret != EFEX_ERR_SUCCESS) {
            break;
        }
        addr += n;
        buf += n;
        len -= n;
    }
    return ret;
}
