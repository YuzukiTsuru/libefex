#include <stdio.h>

#include "libefex.h"
#include "efex-common.h"

int main() {
    struct sunxi_efex_ctx_t ctx = {0};
    int ret = EFEX_ERR_SUCCESS;

    ret = sunxi_scan_usb_device(&ctx);
    if (ret != EFEX_ERR_SUCCESS) {
        fprintf(stderr, "ERROR: %s\r\n", sunxi_efex_strerror(ret));
        return ret;
    }
    ret = sunxi_usb_init(&ctx);
    if (ret != EFEX_ERR_SUCCESS) {
        fprintf(stderr, "ERROR: %s\r\n", sunxi_efex_strerror(ret));
        return ret;
    }
    ret = sunxi_efex_init(&ctx);
    if (ret != EFEX_ERR_SUCCESS) {
        fprintf(stderr, "ERROR: %s\r\n", sunxi_efex_strerror(ret));
        sunxi_usb_exit(&ctx);
        return ret;
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
    
    sunxi_usb_exit(&ctx);
    return 0;
}
