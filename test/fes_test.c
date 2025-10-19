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
        printf("%02x ", (unsigned char) ctx.resp.reserved[i]);
    }
    printf("\n");

    uint32_t flash_type = 0;
    ret = sunxi_efex_fes_query_storage(&ctx, &flash_type);
    if (ret < 0) {
        fprintf(stderr, "ERROR: EFEX FES query storage failed\r\n");
        return -1;
    }
    printf("Storage Type: 0x%08x\n", flash_type);

    ret = sunxi_efex_fes_flash_set_onoff(&ctx, &flash_type, 0);
    if (ret != 0) {
        fprintf(stderr, "ERROR: EFEX FES flash set off failed\r\n");
        return -1;
    }
    printf("Flash Set Off\n");

    uint32_t flash_size = 0;
    ret = sunxi_efex_fes_probe_flash_size(&ctx, &flash_size);
    if (ret < 0) {
        fprintf(stderr, "ERROR: EFEX FES flash set off failed\r\n");
        return -1;
    }
    printf("Flash Size: 0x%08x\n", flash_size);

    ret = sunxi_efex_fes_flash_set_onoff(&ctx, &flash_type, 1);
    if (ret != 0) {
        fprintf(stderr, "ERROR: EFEX FES flash set on failed\r\n");
        return -1;
    }
    printf("Flash Set On\n");

    ret = sunxi_efex_fes_probe_flash_size(&ctx, &flash_size);
    if (ret < 0) {
        fprintf(stderr, "ERROR: EFEX FES flash set on failed\r\n");
        return -1;
    }
    printf("Flash Size: 0x%08x\n", flash_size);

    const char down_buf[16] = "Hello, EFEX FES\0";
    ret = sunxi_efex_fes_down(&ctx, down_buf, 16, 0x40000000, SUNXI_EFEX_DRAM_TAG);
    if (ret < 0) {
        fprintf(stderr, "ERROR: EFEX FES download failed\r\n");
        return -1;
    }
    printf("Download data: %s\n", down_buf);

    const char up_buf[16] = {0};
    ret = sunxi_efex_fes_up(&ctx, (char *) up_buf, 16, 0x40000000, SUNXI_EFEX_DRAM_TAG);
    if (ret < 0) {
        fprintf(stderr, "ERROR: EFEX FES upload failed\r\n");
        return -1;
    }
    printf("Upload Data: %s\n", up_buf);

    // maybe not supported
#if 0
    const char chip_id[129] = {0};
    ret = sunxi_efex_fes_get_chipid(&ctx, chip_id);
    if (ret < 0) {
        fprintf(stderr, "ERROR: EFEX FES get chipid failed\r\n");
        return -1;
    }
    printf("Chip ID: %s\n", chip_id);
#endif
}
