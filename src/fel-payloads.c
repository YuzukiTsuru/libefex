#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "ending.h"
#include "usb_layer.h"
#include "fel-payloads.h"
#include "fel-protocol.h"

extern struct payloads_ops riscv32_e907_ops;

static struct payloads_ops *payloads[] = {
    &riscv32_e907_ops,
};

static struct payloads_ops *current_payload;

void sunxi_fel_payloads_init(const enum sunxi_fel_payloads_arch arch) {
    for (size_t i = 0; i < sizeof(payloads) / sizeof(payloads[0]); ++i) {
        struct payloads_ops *p = payloads[i];
        if (p->arch == arch) {
            current_payload = p;
            return;
        }
    }
    fprintf(stderr, "Didn't find match %d payloads\n", arch);
}

struct payloads_ops *sunxi_fel_get_current_payload() {
    if (current_payload) {
        return current_payload;
    }
    return NULL;
}

uint32_t sunxi_fel_payloads_readl(const struct sunxi_fel_ctx_t *ctx, const uint32_t addr) {
    if (current_payload->readl) {
        return current_payload->readl(ctx, addr);
    }
    return 0;
}

void sunxi_fel_payloads_writel(const struct sunxi_fel_ctx_t *ctx, const uint32_t value, const uint32_t addr) {
    if (current_payload->writel) {
        current_payload->writel(ctx, value, addr);
    }
}
