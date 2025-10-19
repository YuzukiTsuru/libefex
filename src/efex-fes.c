#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ending.h"
#include "efex-usb.h"
#include "efex-protocol.h"

int sunxi_efex_fes_query_storage(const struct sunxi_efex_ctx_t *ctx, uint32_t *storage_type) {
    return sunxi_usb_fes_xfer(ctx, FES_XFER_RECV, EFEX_CMD_FES_QUERY_STORAGE, NULL, 0,
                              (char *) storage_type, sizeof(uint32_t));
}

int sunxi_efex_fes_query_secure(const struct sunxi_efex_ctx_t *ctx, uint32_t *secure_type) {
    return sunxi_usb_fes_xfer(ctx, FES_XFER_RECV, EFEX_CMD_FES_QUERY_SECURE, NULL, 0,
                              (char *) secure_type, sizeof(uint32_t));
}

int sunxi_efex_fes_probe_flash_size(const struct sunxi_efex_ctx_t *ctx, uint32_t *flash_size) {
    return sunxi_usb_fes_xfer(ctx, FES_XFER_RECV, EFEX_CMD_FES_FLASH_SIZE_PROBE, NULL, 0,
                              (char *) flash_size, sizeof(uint32_t));;
}

int sunxi_efex_fes_flash_set_onoff(const struct sunxi_efex_ctx_t *ctx, const uint32_t *storage_type,
                                   const uint32_t on_off) {
    const struct sunxi_fes_flash_t fes_flash = {
            .flash_type = cpu_to_le32(*storage_type),
    };
    const enum sunxi_efex_cmd_t cmd = on_off ? EFEX_CMD_FES_FLASH_SET_ON : EFEX_CMD_FES_FLASH_SET_OFF;
    return sunxi_usb_fes_xfer(ctx, FES_XFER_NONE, cmd, (const char *) &fes_flash, sizeof(fes_flash),
                              NULL, 0);
}

int sunxi_efex_fes_get_chipid(const struct sunxi_efex_ctx_t *ctx, const char *chip_id) {
    return sunxi_usb_fes_xfer(ctx, FES_XFER_RECV, EFEX_CMD_FES_GET_CHIPID, NULL, 0,
                              chip_id, 129);
}

static int sunxi_efex_fes_up_down(const struct sunxi_efex_ctx_t *ctx, const char *buf, const ssize_t len,
                                  const uint32_t addr, const enum sunxi_fes_data_type_t type,
                                  const enum sunxi_efex_cmd_t cmd) {
    int ret = 0;
    if (len <= 0) {
        fprintf(stderr, "Invalid length for FES download: %zd\n", len);
        return -1;
    }

    uint32_t remain_data = (uint32_t) len;
    const char *buff_ptr = (char *) buf;
    uint32_t addr_cur = addr;
    enum sunxi_fes_data_type_t current_type = type;
    const bool is_data_type = (type & SUNXI_EFEX_DATA_TYPE_MASK) != 0;

    while (remain_data > 0) {
        // Calculate current transfer length
        const uint32_t length = (remain_data > EFEX_CODE_MAX_SIZE) ? EFEX_CODE_MAX_SIZE : remain_data;
        remain_data -= length;

        // Add finish tag if this is the last data block
        if (remain_data == 0) {
            current_type |= SUNXI_EFEX_TRANS_FINISH_TAG;
        }

        // Prepare transfer structure
        const struct sunxi_fes_trans_t trans = {
                .addr = addr_cur,
                .len = length,
                .flags = current_type,
        };

        const enum sunxi_usb_fes_xfer_type_t xfer_type =
                (cmd == EFEX_CMD_FES_DOWN) ? FES_XFER_SEND : FES_XFER_RECV;
        // Perform USB transfer
        ret = sunxi_usb_fes_xfer(ctx, xfer_type, cmd,
                                 (const char *) &trans,
                                 sizeof(trans),
                                 buff_ptr, length);

        // Update address based on data type
        addr_cur += is_data_type ? length : (length / 512);
        buff_ptr += length;

        // Check for errors
        if (ret < 0) {
            fprintf(stderr, "Error while downloading FES data\n");
            return -1;
        }
    }

    return 0;
}

int sunxi_efex_fes_down(const struct sunxi_efex_ctx_t *ctx, const char *buf, const ssize_t len, const uint32_t addr,
                        const enum sunxi_fes_data_type_t type) {
    return sunxi_efex_fes_up_down(ctx, buf, len, addr, type, EFEX_CMD_FES_DOWN);
}

int sunxi_efex_fes_up(const struct sunxi_efex_ctx_t *ctx, const char *buf, const ssize_t len, const uint32_t addr,
                      const enum sunxi_fes_data_type_t type) {
    return sunxi_efex_fes_up_down(ctx, buf, len, addr, type, EFEX_CMD_FES_UP);
}

int sunxi_efex_fes_verify_value(const struct sunxi_efex_ctx_t *ctx, const uint32_t addr, const uint64_t size,
                                const struct sunxi_fes_verify_resp_t *buf) {
    const struct sunxi_fes_verify_value_t verify_value = {
            .addr = cpu_to_le32(addr),
            .size = cpu_to_le64(size),
    };
    return sunxi_usb_fes_xfer(ctx, FES_XFER_RECV, EFEX_CMD_FES_VERIFY_VALUE,
                              (const char *) &verify_value, sizeof(verify_value),
                              (const char *) buf, sizeof(struct sunxi_fes_verify_resp_t));
}

int sunxi_efex_fes_verify_status(const struct sunxi_efex_ctx_t *ctx, const uint32_t tag,
                                 const struct sunxi_fes_verify_resp_t *buf) {
    const struct sunxi_fes_verify_status_t verify_status = {
            .addr = 0x0,
            .size = 0x0,
            .tag = cpu_to_le32(tag),
    };
    return sunxi_usb_fes_xfer(ctx, FES_XFER_RECV, EFEX_CMD_FES_VERIFY_STATUS,
                              (const char *) &verify_status, sizeof(verify_status),
                              (const char *) buf, sizeof(struct sunxi_fes_verify_resp_t));
}

int sunxi_efex_fes_verify_uboot_blk(const struct sunxi_efex_ctx_t *ctx, uint32_t tag,
                                    const struct sunxi_fes_verify_resp_t *buf) {
    const struct sunxi_fes_verify_status_t verify_status = {
            .addr = 0x0,
            .size = 0x0,
            .tag = cpu_to_le32(tag),
    };
    return sunxi_usb_fes_xfer(ctx, FES_XFER_RECV, EFEX_CMD_FES_VERIFY_UBOOT_BLK,
                              (const char *) &verify_status, sizeof(verify_status),
                              (const char *) buf, sizeof(struct sunxi_fes_verify_resp_t));
}
