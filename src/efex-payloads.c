#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "efex-protocol.h"
#include "efex-payloads.h"

extern struct payloads_ops riscv32_e907_ops;

static struct payloads_ops *payloads[] = {
        &riscv32_e907_ops,
};

static struct payloads_ops *current_payload;

void sunxi_efex_payloads_init(const enum sunxi_efex_payloads_arch arch) {
    for (size_t i = 0; i < sizeof(payloads) / sizeof(payloads[0]); ++i) {
        struct payloads_ops *p = payloads[i];
        if (p->arch == arch) {
            current_payload = p;
            return;
        }
    }
    fprintf(stderr, "Didn't find match %d payloads\n", arch);
}

struct payloads_ops *sunxi_efex_get_current_payload() {
    if (current_payload) {
        return current_payload;
    }
    return NULL;
}

uint32_t sunxi_efex_payloads_readl(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr) {
    if (ctx->resp.mode != DEVICE_MODE_FEL) {
        fprintf(stderr, "Device is not in FEL mode, cannot perform payload readl\n");
        return 0;
    }
    if (current_payload->readl) {
        return current_payload->readl(ctx, addr);
    }
    return 0;
}

void sunxi_efex_payloads_writel(const struct sunxi_efex_ctx_t *ctx, const uint32_t value, const uint32_t addr) {
    if (ctx->resp.mode != DEVICE_MODE_FEL) {
        fprintf(stderr, "Device is not in FEL mode, cannot perform payload writel\n");
        return;
    }
    if (current_payload->writel) {
        current_payload->writel(ctx, value, addr);
    }
}
