#include <stdio.h>

#include "libefex.h"

int main() {
    struct sunxi_efex_ctx_t ctx = {0};
    int ret = 0;

    // Test common efex init
    printf("Starting efex common tests\n");
    ret = sunxi_scan_usb_device(&ctx);
    if (ret <= 0) {
        fprintf(stderr, "ERROR: Can't get vaild EFEX device\r\n");
        return -1;
    }
    ret = sunxi_usb_init(&ctx);
    if (ret <= 0) {
        fprintf(stderr, "ERROR: EFEX device USB init failed\r\n");
        return -1;
    }
    ret = sunxi_efex_init(&ctx);
    if (ret < 0) {
        fprintf(stderr, "ERROR: EFEX device init failed\r\n");
        return -1;
    }

    printf("Found EFEX device\n");
    printf("Magic: %s\n", ctx.resp.magic);
    printf("ID: 0x%08x\n", ctx.resp.id);
    printf("Firmware: 0x%08x\n", ctx.resp.firmware);
    printf("Mode: 0x%04x\n", ctx.resp.mode);
    printf("Data Flag: 0x%02x\n", ctx.resp.data_flag);
    printf("Data Length: 0x%02x\n", ctx.resp.data_length);
    printf("Data Start Address: 0x%08x\n", ctx.resp.data_start_address);

    printf("Reserved: ");
    for (int i = 0; i < sizeof(ctx.resp.reserved); i++) {
        printf("%02x ", (unsigned char) ctx.resp.reserved[i]);
    }
    printf("\n");

    printf("efex common tests done\n");

    // Test writel/readl
    printf("Orig: 0x%08x\n", sunxi_efex_fel_readl(&ctx, ctx.resp.data_start_address));
    sunxi_efex_fel_writel(&ctx, 0x55AA55AA, ctx.resp.data_start_address);
    printf("New: 0x%08x\n", sunxi_efex_fel_readl(&ctx, ctx.resp.data_start_address));

    // Test payload readl/writel
    sunxi_efex_fel_payloads_init(ARCH_RISCV32_E907);

    uint32_t id[4];
    id[0] = sunxi_efex_fel_payloads_readl(&ctx, 0x03006200 + 0x0);
    id[1] = sunxi_efex_fel_payloads_readl(&ctx, 0x03006200 + 0x4);
    id[2] = sunxi_efex_fel_payloads_readl(&ctx, 0x03006200 + 0x8);
    id[3] = sunxi_efex_fel_payloads_readl(&ctx, 0x03006200 + 0xc);
    printf("sid: %08x%08x%08x%08x\n", id[0], id[1], id[2], id[3]);

    uint32_t reg_val = sunxi_efex_fel_payloads_readl(&ctx, 0x02001000);
    printf("reg_val: 0x%08x\n", reg_val);
    reg_val |= (1 << 31);
    printf("reg_val: 0x%08x\n", reg_val);
    sunxi_efex_fel_payloads_writel(&ctx, reg_val, 0x02001000);

    sunxi_efex_fel_payloads_writel(&ctx, (0x16aa << 16) | (0x1 << 0), 0x06012000 + 0x08);
    reg_val = sunxi_efex_fel_payloads_readl(&ctx, 0x06012000);
    printf("reg_val: 0x%08x\n", reg_val);
}
