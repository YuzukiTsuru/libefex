#include <fel-protocol.h>
#include <stdio.h>

#include "libFEx.h"
#include "usb_layer.h"
#include "fel-protocol.h"

int main() {
    struct sunxi_fel_ctx_t ctx = {0};
    int ret = 0;

    ret = sunxi_scan_usb_device(&ctx);
    if (ret <= 0) {
        fprintf(stderr, "ERROR: Can't get vaild FEL device\r\n");
        return -1;
    }
    ret = sunxi_usb_init(&ctx);
    if (ret <= 0) {
        fprintf(stderr, "ERROR: FEL device USB init failed\r\n");
        return -1;
    }
    ret = sunxi_fel_init(&ctx);
    if (ret < 0) {
        fprintf(stderr, "ERROR: FEL device init failed\r\n");
        return -1;
    }

    printf("Found FEL device\n");
    printf("Magic: %s\n", ctx.resp.magic);
    printf("ID: 0x%08x\n", ctx.resp.id);
    printf("Firmware: 0x%08x\n", ctx.resp.firmware);
    printf("Mode: 0x%04x\n", ctx.resp.mode);
    printf("Data Flag: 0x%02x\n", ctx.resp.data_flag);
    printf("Data Length: 0x%02x\n", ctx.resp.data_length);
    printf("Data Start Address: 0x%08x\n", ctx.resp.data_start_address);

    printf("Reserved: ");
    for (int i = 0; i < sizeof(ctx.resp.reserved); i++) {
        printf("%02x ", (unsigned char)ctx.resp.reserved[i]);
    }
    printf("\n");

    printf("Orig: 0x%08x\n", sunxi_fel_readl(&ctx, ctx.resp.data_start_address));
    sunxi_fel_writel(&ctx, 0x55AA55AA, ctx.resp.data_start_address);
    printf("New: 0x%08x\n", sunxi_fel_readl(&ctx, ctx.resp.data_start_address));
}
