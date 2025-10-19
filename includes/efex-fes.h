#ifndef LIBEFEX_EFEX_FES_H
#define LIBEFEX_EFEX_FES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "efex-common.h"

/**
 * @brief FES transfer data type enum
 * 
 * Defines various data type tags and transfer control flags for FES mode
 */
enum sunxi_fes_data_type_t {
    SUNXI_EFEX_TAG_NONE = 0x0, /**< No tag */
    /* Data type tag */
    SUNXI_EFEX_DRAM_TAG = 0x7f00, /**< DRAM configuration data tag */
    SUNXI_EFEX_MBR_TAG = 0x7f01, /**< MBR partition table tag */
    SUNXI_EFEX_BOOT1_TAG = 0x7f02, /**< BOOT1 tag */
    SUNXI_EFEX_BOOT0_TAG = 0x7f03, /**< BOOT0 tag */
    SUNXI_EFEX_ERASE_TAG = 0x7f04, /**< Erase command tag */
    SUNXI_EFEX_FULLIMG_SIZE_TAG = 0x7f10, /**< Full image size tag */
    SUNXI_EFEX_EXT4_UBIFS_TAG = 0x7ff0, /**< EXT4/UBIFS file system tag */
    SUNXI_EFEX_FLASH_TAG = 0x8000, /**< FLASH operation tag */
    /* Data type mask */
    SUNXI_EFEX_DATA_TYPE_MASK = 0x7fff, /**< Data type mask */

    /* Transfer tag */
    SUNXI_EFEX_TRANS_START_TAG = 0x20000, /**< Transfer start tag */
    SUNXI_EFEX_TRANS_FINISH_TAG = 0x10000, /**< Transfer finish tag */

    /* Transfer mask */
    SUNXI_EFEX_TRANS_MASK = 0x30000, /**< Transfer control mask */
};

/**
 * @brief Query storage device type
 * 
 * @param ctx Context pointer
 * @param storage_type Output parameter, storage device type
 * @return 0 on success, negative value on failure
 */
int sunxi_efex_fes_query_storage(const struct sunxi_efex_ctx_t *ctx, uint32_t *storage_type);

/**
 * @brief Query secure mode type
 * 
 * @param ctx Context pointer
 * @param secure_type Output parameter, secure mode type
 * @return 0 on success, negative value on failure
 */
int sunxi_efex_fes_query_secure(const struct sunxi_efex_ctx_t *ctx, uint32_t *secure_type);

/**
 * @brief Probe flash size
 * 
 * @param ctx Context pointer
 * @param flash_size Output parameter, flash size
 * @return 0 on success, negative value on failure
 */
int sunxi_efex_fes_probe_flash_size(const struct sunxi_efex_ctx_t *ctx, uint32_t *flash_size);

/**
 * @brief Set flash on/off status
 * 
 * @param ctx Context pointer
 * @param storage_type Storage device type
 * @param on_off On/off status, 1 for on, 0 for off
 * @return 0 on success, negative value on failure
 */
int sunxi_efex_fes_flash_set_onoff(const struct sunxi_efex_ctx_t *ctx, const uint32_t *storage_type, uint32_t on_off);

/**
 * @brief Get chip ID
 * 
 * @param ctx Context pointer
 * @param chip_id Output parameter, chip ID string buffer
 * @return 0 on success, negative value on failure
 */
int sunxi_efex_fes_get_chipid(const struct sunxi_efex_ctx_t *ctx, const char *chip_id);

/**
 * @brief Send data to FES (download)
 * 
 * @param ctx Context pointer
 * @param buf Data buffer
 * @param len Data length
 * @param addr Target address
 * @param type Data type
 * @return 0 on success, negative value on failure
 */
int sunxi_efex_fes_down(const struct sunxi_efex_ctx_t *ctx, const char *buf, const ssize_t len, uint32_t addr,
                        enum sunxi_fes_data_type_t type);

/**
 * @brief Receive data from FES (upload)
 * 
 * @param ctx Context pointer
 * @param buf Data buffer
 * @param len Data length
 * @param addr Source address
 * @param type Data type
 * @return 0 on success, negative value on failure
 */
int sunxi_efex_fes_up(const struct sunxi_efex_ctx_t *ctx, const char *buf, const ssize_t len, uint32_t addr,
                      enum sunxi_fes_data_type_t type);

/**
 * @brief Verify content at specified address and size
 * 
 * @param ctx Context pointer
 * @param addr Verify address
 * @param size Verify size
 * @param buf Output parameter, verification response result
 * @return 0 on success, negative value on failure
 */
int sunxi_efex_fes_verify_value(const struct sunxi_efex_ctx_t *ctx, uint32_t addr, uint64_t size,
                                const struct sunxi_fes_verify_resp_t *buf);

/**
 * @brief Verify status of specified tag
 * 
 * @param ctx Context pointer
 * @param tag Tag value
 * @param buf Output parameter, verification response result
 * @return 0 on success, negative value on failure
 */
int sunxi_efex_fes_verify_status(const struct sunxi_efex_ctx_t *ctx, uint32_t tag,
                                 const struct sunxi_fes_verify_resp_t *buf);

/**
 * @brief Verify UBOOT block
 * 
 * @param ctx Context pointer
 * @param tag Tag value
 * @param buf Output parameter, verification response result
 * @return 0 on success, negative value on failure
 */
int sunxi_efex_fes_verify_uboot_blk(const struct sunxi_efex_ctx_t *ctx, uint32_t tag,
                                    const struct sunxi_fes_verify_resp_t *buf);

/**
 * @brief Set tool mode
 * 
 * @param ctx Context pointer
 * @param tool_mode Tool mode value
 * @param next_mode Next mode value
 * @return 0 on success, negative value on failure
 */
int sunxi_efex_fes_tool_mode(const struct sunxi_efex_ctx_t *ctx, uint32_t tool_mode, uint32_t next_mode);


#ifdef __cplusplus
}
#endif

#endif //LIBEFEX_EFEX_FES_H
