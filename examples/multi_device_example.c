#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libefex.h>

void print_device_info(struct sunxi_efex_multi_ctx_t *multi_ctx) {
    printf("Device status:\n");
    size_t total = sunxi_efex_multi_get_count(multi_ctx);
    size_t connected = sunxi_efex_multi_count_by_state(multi_ctx, EFEX_DEVICE_STATE_CONNECTED);
    size_t initialized = sunxi_efex_multi_count_by_state(multi_ctx, EFEX_DEVICE_STATE_INITIALIZED);
    size_t flashed = sunxi_efex_multi_count_by_state(multi_ctx, EFEX_DEVICE_STATE_FLASHED);
    size_t errors = sunxi_efex_multi_count_by_state(multi_ctx, EFEX_DEVICE_STATE_ERROR);

    printf("  Total: %zu\n", total);
    printf("  Connected: %zu\n", connected);
    printf("  Initialized: %zu\n", initialized);
    printf("  Flashed: %zu\n", flashed);
    printf("  Errors: %zu\n\n", errors);
}

int main(int argc, char *argv[]) {
    const char *firmware_file = (argc > 1) ? argv[1] : "firmware.fex";

    FILE *fp = fopen(firmware_file, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open firmware file: %s\n", firmware_file);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *firmware_data = malloc(file_size);
    if (!firmware_data) {
        fclose(fp);
        fprintf(stderr, "Failed to allocate memory\n");
        return -1;
    }

    fread(firmware_data, 1, file_size, fp);
    fclose(fp);

    printf("Multi-device example\nFirmware: %s (%ld bytes)\n\n", firmware_file, file_size);

    struct sunxi_efex_multi_ctx_t *multi_ctx = NULL;

    if (sunxi_efex_multi_create(0, &multi_ctx) != 0) {
        fprintf(stderr, "Failed to create multi-device context\n");
        free(firmware_data);
        return -1;
    }

    printf("Scanning for devices (REPLACE mode)...\n");
    int count = sunxi_efex_multi_scan_devices(multi_ctx, EFEX_SCAN_MODE_REPLACE);
    if (count <= 0) {
        fprintf(stderr, "No devices found\n");
        sunxi_efex_multi_destroy(&multi_ctx);
        free(firmware_data);
        return -1;
    }

    printf("Found %d devices\n", count);
    print_device_info(multi_ctx);

    printf("Initializing all devices...\n");
    int init_count = sunxi_efex_multi_init_all(multi_ctx, 1);
    printf("Initialized: %d/%d\n", init_count, count);
    print_device_info(multi_ctx);

    printf("\nSequential flashing:\n");
    int seq_success = sunxi_efex_multi_flash_all(multi_ctx, firmware_data, file_size,
                                                  0, SUNXI_EFEX_FLASH_TAG);
    printf("Sequential: %d/%d\n", seq_success, count);
    print_device_info(multi_ctx);

    printf("\nRebooting all devices...\n");
    int reboot_count = sunxi_efex_multi_reboot_all(multi_ctx, TOOL_MODE_REBOOT, TOOL_MODE_NORMAL);
    printf("Rebooted: %d/%d\n", reboot_count, count);

    printf("\nWaiting 2 seconds for devices to reconnect...\n");
    sleep(2);

    printf("Rescanning for devices (APPEND mode)...\n");
    int new_count = sunxi_efex_multi_scan_devices(multi_ctx, EFEX_SCAN_MODE_APPEND);
    printf("New devices found: %d\n", new_count);
    print_device_info(multi_ctx);

    sunxi_efex_multi_destroy(&multi_ctx);
    free(firmware_data);

    return 0;
}
