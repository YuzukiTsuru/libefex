#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#include "efex-common.h"
#include "efex-payloads.h"
#include "efex-protocol.h"


extern struct payloads_ops riscv32_e907_ops;

static struct payloads_ops *payloads[] = {
		&riscv32_e907_ops,
};

static struct payloads_ops *current_payload;

int sunxi_efex_fel_payloads_init(const enum sunxi_efex_fel_payloads_arch arch) {
	for (size_t i = 0; i < sizeof(payloads) / sizeof(payloads[0]); ++i) {
		struct payloads_ops *p = payloads[i];
		if (p->arch == arch) {
			current_payload = p;
			return EFEX_ERR_SUCCESS;
		}
	}
	return EFEX_ERR_INVALID_PARAM;
}

struct payloads_ops *sunxi_efex_fel_get_current_payload() { return current_payload; }

int sunxi_efex_fel_payloads_readl(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr, uint32_t *val) {
	if (!ctx || !val) {
		return EFEX_ERR_NULL_PTR;
	}
	if (ctx->resp.mode != DEVICE_MODE_FEL) {
		return EFEX_ERR_INVALID_DEVICE_MODE;
	}
	if (!current_payload || !current_payload->readl) {
		return EFEX_ERR_NOT_SUPPORT;
	}
	return current_payload->readl(ctx, addr, val);
}

int sunxi_efex_fel_payloads_writel(const struct sunxi_efex_ctx_t *ctx, const uint32_t value, const uint32_t addr) {
	if (!ctx) {
		return EFEX_ERR_NULL_PTR;
	}
	if (ctx->resp.mode != DEVICE_MODE_FEL) {
		return EFEX_ERR_INVALID_DEVICE_MODE;
	}
	if (!current_payload || !current_payload->writel) {
		return EFEX_ERR_NOT_SUPPORT;
	}
	return current_payload->writel(ctx, value, addr);
}
