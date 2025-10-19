#include <stdio.h>

#include "libefex.h"

int main() {
    struct sunxi_efex_ctx_t ctx = {0};
    int ret = 0;

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
        printf("%02x ", (unsigned char)ctx.resp.reserved[i]);
    }
    printf("\n");

    uint32_t data = 0;
    sunxi_efex_fes_query_storage(&ctx, &data);
    printf("Storage Type: 0x%08x\n", data);

    sunxi_efex_fes_probe_flash_size(&ctx, &data);
    printf("Flash Size: 0x%08x\n", data);
}
