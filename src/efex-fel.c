#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "efex-common.h"
#include "efex-protocol.h"
#include "efex-usb.h"
#include "ending.h"

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

int sunxi_efex_fel_read(const struct sunxi_efex_ctx_t *ctx, uint32_t addr, char *buf, ssize_t len) {
	if (!ctx || !buf) {
		return EFEX_ERR_NULL_PTR;
	}

	if (ctx->resp.mode != DEVICE_MODE_FEL) {
		return EFEX_ERR_INVALID_DEVICE_MODE;
	}

	if (len <= 0) {
		return EFEX_ERR_INVALID_PARAM;
	}

	int ret = EFEX_ERR_SUCCESS;
	while (len > 0) {
		const uint32_t n = len > EFEX_CODE_MAX_SIZE ? EFEX_CODE_MAX_SIZE : (uint32_t) len;

		ret = sunxi_send_efex_request(ctx, EFEX_CMD_FEL_READ, addr, n);
		if (ret != EFEX_ERR_SUCCESS) {
			return ret;
		}

		ret = sunxi_usb_read(ctx, (void *) buf, n);
		if (ret != EFEX_ERR_SUCCESS) {
			return ret;
		}

		ret = sunxi_read_efex_status(ctx);
		if (ret < 0) {
			return ret;
		}

		addr += n;
		buf += n;
		len -= n;
	}
	return ret;
}

int sunxi_efex_fel_write(const struct sunxi_efex_ctx_t *ctx, uint32_t addr, const char *buf, ssize_t len) {
	if (!ctx || !buf) {
		return EFEX_ERR_NULL_PTR;
	}

	if (ctx->resp.mode != DEVICE_MODE_FEL) {
		return EFEX_ERR_INVALID_DEVICE_MODE;
	}

	if (len <= 0) {
		return EFEX_ERR_INVALID_PARAM;
	}

	int ret = EFEX_ERR_SUCCESS;
	while (len > 0) {
		const uint32_t n = len > EFEX_CODE_MAX_SIZE ? EFEX_CODE_MAX_SIZE : (uint32_t) len;

		ret = sunxi_send_efex_request(ctx, EFEX_CMD_FEL_WRITE, addr, n);
		if (ret != EFEX_ERR_SUCCESS) {
			return ret;
		}

		ret = sunxi_usb_write(ctx, buf, n);
		if (ret != EFEX_ERR_SUCCESS) {
			return ret;
		}

		ret = sunxi_read_efex_status(ctx);
		if (ret < 0) {
			return ret;
		}

		addr += n;
		buf += n;
		len -= n;
	}
	return ret;
}
