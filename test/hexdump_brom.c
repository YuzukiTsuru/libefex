#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "libefex.h"
#include "usb_layer.h"
#include "efex-protocol.h"

void hexdump(const uint32_t addr, const void *buf, const size_t len) {
    const unsigned char *p = buf;
    size_t i;

    for (size_t j = 0; j < len; j += 16) {
        printf("%08x: ", (uint32_t) (addr + j));
        for (i = 0; i < 16; i++) {
            if (j + i < len)
                printf("%02x ", p[j + i]);
            else
                printf("   ");
        }
        putchar(' ');
        for (i = 0; i < 16; i++) {
            if (j + i >= len)
                putchar(' ');
            else
                putchar(isprint(p[j + i]) ? p[j + i] : '.');
        }
        printf("\r\n");
    }
}

int main() {
    struct sunxi_efex_ctx_t ctx = {0};
    int ret = 0;

    // Allocate memory for buf (assuming buf is a pointer to a byte array)
    char *buf = malloc(0x100); // Allocate 0x100 bytes
    memset(buf, 0, 0x100);

    if (buf == NULL) {
        fprintf(stderr, "ERROR: Memory allocation failed for buf\n");
        return -1;
    }

    ret = sunxi_scan_usb_device(&ctx);
    if (ret <= 0) {
        fprintf(stderr, "ERROR: Can't get valid EFEX device\r\n");
        free(buf); // Free the allocated memory before exiting
        return -1;
    }

    ret = sunxi_usb_init(&ctx);
    if (ret <= 0) {
        fprintf(stderr, "ERROR: EFEX device USB init failed\r\n");
        free(buf); // Free the allocated memory before exiting
        return -1;
    }

    ret = sunxi_efex_init(&ctx);
    if (ret < 0) {
        fprintf(stderr, "ERROR: EFEX device init failed\r\n");
        free(buf); // Free the allocated memory before exiting
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

    sunxi_efex_fel_write_memory(&ctx, 0x120000, "Hello, EFEX!", 12);

    // Read memory into buf
    sunxi_efex_fel_read_memory(&ctx, 0x120000, buf, 0x100);

    // Output the contents of buf in hex
    hexdump(0x0, buf, 0x100);

    // Free the allocated memory after use
    free(buf);

    return 0;
}
