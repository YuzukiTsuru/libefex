#ifndef LIBEFEX_EFEX_FES_H
#define LIBEFEX_EFEX_FES_H

#include "efex-common.h"

enum sunxi_fes_data_type_t {
    SUNXI_EFEX_TAG_NONE = 0x0,
    /* Data type tag */
    SUNXI_EFEX_DRAM_TAG = 0x7f00,
    SUNXI_EFEX_MBR_TAG = 0x7f01,
    SUNXI_EFEX_BOOT1_TAG = 0x7f02,
    SUNXI_EFEX_BOOT0_TAG = 0x7f03,
    SUNXI_EFEX_ERASE_TAG = 0x7f04,
    SUNXI_EFEX_FULLIMG_SIZE_TAG = 0x7f10,
    SUNXI_EFEX_EXT4_UBIFS_TAG = 0x7ff0,
    SUNXI_EFEX_FLASH_TAG = 0x8000,
    /* Data type mask */
    SUNXI_EFEX_DATA_TYPE_MASK = 0x7fff,

    /* Transfer tag */
    SUNXI_EFEX_TRANS_START_TAG = 0x20000,
    SUNXI_EFEX_TRANS_FINISH_TAG = 0x10000,

    /* Transfer mask */
    SUNXI_EFEX_TRANS_MASK = 0x30000,
};

int sunxi_efex_fes_query_storage(const struct sunxi_efex_ctx_t *ctx, uint32_t *storage_type);

int sunxi_efex_fes_probe_flash_size(const struct sunxi_efex_ctx_t *ctx, uint32_t *flash_size);

int sunxi_efex_fes_flash_set_onoff(const struct sunxi_efex_ctx_t *ctx, const uint32_t *storage_type, uint32_t on_off);

int sunxi_efex_fes_get_chipid(const struct sunxi_efex_ctx_t *ctx, const char *chip_id);

int sunxi_efex_fes_down(const struct sunxi_efex_ctx_t *ctx, const char *buf, const ssize_t len, uint32_t addr,
                        enum sunxi_fes_data_type_t type);

int sunxi_efex_fes_up(const struct sunxi_efex_ctx_t *ctx, const char *buf, const ssize_t len, uint32_t addr,
                      enum sunxi_fes_data_type_t type);

#endif //LIBEFEX_EFEX_FES_H
