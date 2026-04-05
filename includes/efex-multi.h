/**
 * @file efex-multi.h
 * @brief Multi-device support for EFEX operations
 *
 * This file provides multi-device management functionality including device scanning,
 * batch operations, and parallel processing capabilities.
 */

#ifndef LIBEFEX_EFEX_MULTI_H
#define LIBEFEX_EFEX_MULTI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "efex-protocol.h"
#include "efex-fes.h"
#include <stddef.h>

#define EFEX_MAX_DEVICES 16

/**
 * @brief Device state enumeration
 *
 * Defines the state of a device in the multi-device context.
 */
enum sunxi_efex_device_state_t {
    EFEX_DEVICE_STATE_DISCONNECTED = 0,  /**< Device not connected */
    EFEX_DEVICE_STATE_CONNECTED,          /**< Device connected but not initialized */
    EFEX_DEVICE_STATE_INITIALIZED,        /**< Device initialized and ready */
    EFEX_DEVICE_STATE_FLASHED,            /**< Device flashed successfully */
    EFEX_DEVICE_STATE_ERROR,              /**< Device in error state */
};

/**
 * @brief Scan mode enumeration
 *
 * Defines how devices should be scanned.
 */
enum sunxi_efex_scan_mode_t {
    EFEX_SCAN_MODE_REPLACE = 0,   /**< Replace all devices with newly scanned ones */
    EFEX_SCAN_MODE_APPEND,        /**< Append new devices to existing list */
    EFEX_SCAN_MODE_UPDATE,       /**< Update existing devices and append new ones */
};

struct sunxi_efex_device_info_t {
    uint8_t bus;
    uint8_t port;
    uint16_t vid;
    uint16_t pid;
    enum sunxi_efex_device_state_t state;
    int error_code;
};

struct sunxi_efex_multi_ctx_t {
    struct sunxi_efex_ctx_t *devices;
    struct sunxi_efex_device_info_t *info;
    size_t count;
    size_t capacity;
};

typedef int (*sunxi_efex_multi_callback_t)(struct sunxi_efex_ctx_t *ctx, size_t index, void *user_data);

/**
 * @brief Create multi-device context
 *
 * @param capacity Maximum number of devices (0 for default EFEX_MAX_DEVICES)
 * @param multi_ctx Pointer to store multi-device context
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_efex_multi_create(size_t capacity, struct sunxi_efex_multi_ctx_t **multi_ctx);

/**
 * @brief Destroy multi-device context and release resources
 *
 * @param multi_ctx Pointer to multi-device context pointer
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_efex_multi_destroy(struct sunxi_efex_multi_ctx_t **multi_ctx);

/**
 * @brief Get device count
 *
 * @param multi_ctx Multi-device context
 * @return Number of devices
 */
size_t sunxi_efex_multi_get_count(const struct sunxi_efex_multi_ctx_t *multi_ctx);

/**
 * @brief Get device at specific index
 *
 * @param multi_ctx Multi-device context
 * @param index Device index
 * @return Pointer to device context, or NULL on invalid index
 */
struct sunxi_efex_ctx_t *sunxi_efex_multi_get_device(struct sunxi_efex_multi_ctx_t *multi_ctx, size_t index);

/**
 * @brief Get device info at specific index
 *
 * @param multi_ctx Multi-device context
 * @param index Device index
 * @return Pointer to device info, or NULL on invalid index
 */
struct sunxi_efex_device_info_t *sunxi_efex_multi_get_info(struct sunxi_efex_multi_ctx_t *multi_ctx, size_t index);

/**
 * @brief Scan and connect to all available devices
 *
 * @param multi_ctx Multi-device context
 * @param mode Scan mode (replace, append, or update)
 * @return Number of devices found, or negative error code on failure
 */
int sunxi_efex_multi_scan_devices(struct sunxi_efex_multi_ctx_t *multi_ctx,
                                   enum sunxi_efex_scan_mode_t mode);

/**
 * @brief Initialize all devices
 *
 * @param multi_ctx Multi-device context
 * @param skip_errors Skip devices with errors and continue with others
 * @return Number of devices successfully initialized, or negative error code on failure
 */
int sunxi_efex_multi_init_all(struct sunxi_efex_multi_ctx_t *multi_ctx, int skip_errors);

/**
 * @brief Exit and cleanup all devices
 *
 * @param multi_ctx Multi-device context
 * @return Number of devices successfully exited, or negative error code on failure
 */
int sunxi_efex_multi_exit_all(struct sunxi_efex_multi_ctx_t *multi_ctx);

/**
 * @brief Execute callback for each device sequentially
 *
 * @param multi_ctx Multi-device context
 * @param callback Callback function to execute for each device
 * @param user_data User data passed to callback
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_efex_multi_foreach(struct sunxi_efex_multi_ctx_t *multi_ctx,
                             sunxi_efex_multi_callback_t callback,
                             void *user_data);

/**
 * @brief Execute callback for each device in parallel
 *
 * @param multi_ctx Multi-device context
 * @param callback Callback function to execute for each device
 * @param user_data User data passed to callback
 * @param max_threads Maximum number of parallel threads (0 for device count)
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_efex_multi_foreach_parallel(struct sunxi_efex_multi_ctx_t *multi_ctx,
                                      sunxi_efex_multi_callback_t callback,
                                      void *user_data,
                                      size_t max_threads);

/**
 * @brief Flash firmware to all devices sequentially
 *
 * @param multi_ctx Multi-device context
 * @param firmware_data Firmware data buffer
 * @param firmware_size Firmware data size
 * @param addr Flash address
 * @param type Data type
 * @return Number of successful flashes, or negative error code on failure
 */
int sunxi_efex_multi_flash_all(struct sunxi_efex_multi_ctx_t *multi_ctx,
                               const char *firmware_data,
                               size_t firmware_size,
                               uint32_t addr,
                               enum sunxi_fes_data_type_t type);

/**
 * @brief Flash firmware to all devices in parallel
 *
 * @param multi_ctx Multi-device context
 * @param firmware_data Firmware data buffer
 * @param firmware_size Firmware data size
 * @param addr Flash address
 * @param type Data type
 * @param max_threads Maximum number of parallel threads (0 for device count)
 * @return Number of successful flashes, or negative error code on failure
 */
int sunxi_efex_multi_flash_all_parallel(struct sunxi_efex_multi_ctx_t *multi_ctx,
                                        const char *firmware_data,
                                        size_t firmware_size,
                                        uint32_t addr,
                                        enum sunxi_fes_data_type_t type,
                                        size_t max_threads);

/**
 * @brief Reboot all devices
 *
 * @param multi_ctx Multi-device context
 * @param tool_mode Tool mode to set
 * @param next_mode Next mode after reboot
 * @return Number of successful reboots, or negative error code on failure
 */
int sunxi_efex_multi_reboot_all(struct sunxi_efex_multi_ctx_t *multi_ctx,
                                enum sunxi_fes_tool_mode_t tool_mode,
                                enum sunxi_fes_tool_mode_t next_mode);

/**
 * @brief Add a device at specific bus and port
 *
 * @param multi_ctx Multi-device context
 * @param bus USB bus number
 * @param port USB port number
 * @return Device index on success, or negative error code on failure
 */
int sunxi_efex_multi_add_device(struct sunxi_efex_multi_ctx_t *multi_ctx,
                                 uint8_t bus, uint8_t port);

/**
 * @brief Remove a device at specific index
 *
 * @param multi_ctx Multi-device context
 * @param index Device index
 * @return EFEX_ERR_SUCCESS on success, or an error code on failure
 */
int sunxi_efex_multi_remove_device(struct sunxi_efex_multi_ctx_t *multi_ctx, size_t index);

/**
 * @brief Refresh device connections (reconnect disconnected devices)
 *
 * @param multi_ctx Multi-device context
 * @return Number of successfully refreshed devices, or negative error code on failure
 */
int sunxi_efex_multi_refresh_devices(struct sunxi_efex_multi_ctx_t *multi_ctx);

/**
 * @brief Get number of devices in specific state
 *
 * @param multi_ctx Multi-device context
 * @param state Device state to count
 * @return Number of devices in the specified state
 */
size_t sunxi_efex_multi_count_by_state(struct sunxi_efex_multi_ctx_t *multi_ctx,
                                       enum sunxi_efex_device_state_t state);

#ifdef __cplusplus
}
#endif

#endif // LIBEFEX_EFEX_MULTI_H
