#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "efex-multi.h"
#include "efex-common.h"
#include "efex-fes.h"
#include "usb_layer.h"
#include "compiler.h"
#include "ending.h"
#include "usb_layer.h"

int sunxi_efex_multi_create(size_t capacity, struct sunxi_efex_multi_ctx_t **multi_ctx) {
    if (!multi_ctx) {
        return EFEX_ERR_NULL_PTR;
    }

    if (capacity == 0) {
        capacity = EFEX_MAX_DEVICES;
    }

    if (capacity > EFEX_MAX_DEVICES) {
        return EFEX_ERR_INVALID_PARAM;
    }

    struct sunxi_efex_multi_ctx_t *ctx = calloc(1, sizeof(struct sunxi_efex_multi_ctx_t));
    if (!ctx) {
        return EFEX_ERR_MEMORY;
    }

    ctx->devices = calloc(capacity, sizeof(struct sunxi_efex_ctx_t));
    if (!ctx->devices) {
        free(ctx);
        return EFEX_ERR_MEMORY;
    }

    ctx->info = calloc(capacity, sizeof(struct sunxi_efex_device_info_t));
    if (!ctx->info) {
        free(ctx->devices);
        free(ctx);
        return EFEX_ERR_MEMORY;
    }

    ctx->capacity = capacity;
    ctx->count = 0;

    *multi_ctx = ctx;
    return EFEX_ERR_SUCCESS;
}

int sunxi_efex_multi_destroy(struct sunxi_efex_multi_ctx_t **multi_ctx) {
    if (!multi_ctx || !*multi_ctx) {
        return EFEX_ERR_NULL_PTR;
    }

    struct sunxi_efex_multi_ctx_t *ctx = *multi_ctx;

    if (ctx->devices) {
        free(ctx->devices);
    }

    if (ctx->info) {
        free(ctx->info);
    }

    free(ctx);
    *multi_ctx = NULL;

    return EFEX_ERR_SUCCESS;
}

size_t sunxi_efex_multi_get_count(const struct sunxi_efex_multi_ctx_t *multi_ctx) {
    if (!multi_ctx) {
        return 0;
    }
    return multi_ctx->count;
}

struct sunxi_efex_ctx_t *sunxi_efex_multi_get_device(struct sunxi_efex_multi_ctx_t *multi_ctx, size_t index) {
    if (!multi_ctx || !multi_ctx->devices) {
        return NULL;
    }

    if (index >= multi_ctx->count) {
        return NULL;
    }

    return &multi_ctx->devices[index];
}

struct sunxi_efex_device_info_t *sunxi_efex_multi_get_info(struct sunxi_efex_multi_ctx_t *multi_ctx, size_t index) {
    if (!multi_ctx || !multi_ctx->info) {
        return NULL;
    }

    if (index >= multi_ctx->count) {
        return NULL;
    }

    return &multi_ctx->info[index];
}

static int device_exists(struct sunxi_efex_multi_ctx_t *multi_ctx, uint8_t bus, uint8_t port, size_t *index) {
    if (!multi_ctx) {
        return 0;
    }

    for (size_t i = 0; i < multi_ctx->count; i++) {
        if (multi_ctx->info[i].bus == bus && multi_ctx->info[i].port == port) {
            if (index) {
                *index = i;
            }
            return 1;
        }
    }
    return 0;
}

int sunxi_efex_multi_scan_devices(struct sunxi_efex_multi_ctx_t *multi_ctx,
                                   enum sunxi_efex_scan_mode_t mode) {
    if (!multi_ctx || !multi_ctx->devices) {
        return EFEX_ERR_NULL_PTR;
    }

    if (sunxi_usb_init(NULL) != EFEX_ERR_SUCCESS) {
        return EFEX_ERR_USB_INIT;
    }

    struct sunxi_scanned_device_t *scanned_devices = NULL;
    size_t scanned_count = 0;
    int ret = sunxi_scan_usb_devices(&scanned_devices, &scanned_count);

    if (ret != EFEX_ERR_SUCCESS || scanned_count == 0) {
        free(scanned_devices);
        sunxi_usb_exit(NULL);

        if (mode == EFEX_SCAN_MODE_REPLACE && multi_ctx->count > 0) {
            multi_ctx->count = 0;
        }

        return (scanned_count == 0) ? 0 : ret;
    }

    int added_count = 0;

    if (mode == EFEX_SCAN_MODE_REPLACE) {
        sunxi_efex_multi_exit_all(multi_ctx);
        multi_ctx->count = 0;
    }

    for (size_t i = 0; i < scanned_count; i++) {
        uint8_t bus = scanned_devices[i].bus;
        uint8_t port = scanned_devices[i].port;

        if (mode == EFEX_SCAN_MODE_REPLACE || !device_exists(multi_ctx, bus, port, NULL)) {
            if (multi_ctx->count >= multi_ctx->capacity) {
                break;
            }

            size_t idx = multi_ctx->count;
            struct sunxi_efex_ctx_t *ctx = &multi_ctx->devices[idx];
            struct sunxi_efex_device_info_t *info = &multi_ctx->info[idx];

            memset(ctx, 0, sizeof(struct sunxi_efex_ctx_t));
            memset(info, 0, sizeof(struct sunxi_efex_device_info_t));

            if (sunxi_scan_usb_device_at(ctx, bus, port) == EFEX_ERR_SUCCESS) {
                if (sunxi_usb_init(ctx) == EFEX_ERR_SUCCESS) {
                    info->bus = bus;
                    info->port = port;
                    info->vid = scanned_devices[i].vid;
                    info->pid = scanned_devices[i].pid;
                    info->state = EFEX_DEVICE_STATE_CONNECTED;
                    info->error_code = 0;
                    multi_ctx->count++;
                    added_count++;
                } else {
                    sunxi_usb_exit(ctx);
                }
            }
        }
    }

    free(scanned_devices);

    if (multi_ctx->count == 0) {
        sunxi_usb_exit(NULL);
        return EFEX_ERR_USB_DEVICE_NOT_FOUND;
    }

    return added_count;
}

int sunxi_efex_multi_init_all(struct sunxi_efex_multi_ctx_t *multi_ctx, int skip_errors) {
    if (!multi_ctx || !multi_ctx->devices) {
        return EFEX_ERR_NULL_PTR;
    }

    int success = 0;
    for (size_t i = 0; i < multi_ctx->count; i++) {
        struct sunxi_efex_ctx_t *ctx = &multi_ctx->devices[i];
        if (multi_ctx->info[i].state != EFEX_DEVICE_STATE_CONNECTED) {
            continue;
        }

        int ret = sunxi_efex_init(ctx);
        if (ret == EFEX_ERR_SUCCESS) {
            multi_ctx->info[i].state = EFEX_DEVICE_STATE_INITIALIZED;
            multi_ctx->info[i].error_code = 0;
            success++;
        } else {
            multi_ctx->info[i].state = EFEX_DEVICE_STATE_ERROR;
            multi_ctx->info[i].error_code = ret;
            if (!skip_errors) {
                return ret;
            }
        }
    }

    return success;
}

int sunxi_efex_multi_exit_all(struct sunxi_efex_multi_ctx_t *multi_ctx) {
    if (!multi_ctx || !multi_ctx->devices) {
        return EFEX_ERR_NULL_PTR;
    }

    int success = 0;
    for (size_t i = 0; i < multi_ctx->count; i++) {
        struct sunxi_efex_ctx_t *ctx = &multi_ctx->devices[i];
        if (multi_ctx->info[i].state == EFEX_DEVICE_STATE_INITIALIZED ||
            multi_ctx->info[i].state == EFEX_DEVICE_STATE_FLASHED) {
            if (sunxi_usb_exit(ctx) == EFEX_ERR_SUCCESS) {
                multi_ctx->info[i].state = EFEX_DEVICE_STATE_CONNECTED;
                success++;
            } else {
                multi_ctx->info[i].state = EFEX_DEVICE_STATE_ERROR;
            }
        }
    }

    sunxi_usb_exit(NULL);
    return success;
}

int sunxi_efex_multi_foreach(struct sunxi_efex_multi_ctx_t *multi_ctx,
                             sunxi_efex_multi_callback_t callback,
                             void *user_data) {
    if (!multi_ctx || !callback) {
        return EFEX_ERR_NULL_PTR;
    }

    int ret = EFEX_ERR_SUCCESS;
    for (size_t i = 0; i < multi_ctx->count; i++) {
        struct sunxi_efex_ctx_t *ctx = &multi_ctx->devices[i];
        int result = callback(ctx, i, user_data);
        if (result != 0 && ret == EFEX_ERR_SUCCESS) {
            ret = result;
        }
    }

    return ret;
}

struct thread_task_arg {
    struct sunxi_efex_ctx_t *ctx;
    sunxi_efex_multi_callback_t callback;
    void *user_data;
    size_t index;
    int result;
};

static void *thread_task_wrapper(void *arg) {
    struct thread_task_arg *task = (struct thread_task_arg *)arg;
    task->result = task->callback(task->ctx, task->index, task->user_data);
    return NULL;
}

int sunxi_efex_multi_foreach_parallel(struct sunxi_efex_multi_ctx_t *multi_ctx,
                                      sunxi_efex_multi_callback_t callback,
                                      void *user_data,
                                      size_t max_threads) {
    if (!multi_ctx || !callback) {
        return EFEX_ERR_NULL_PTR;
    }

    if (multi_ctx->count == 0) {
        return EFEX_ERR_SUCCESS;
    }

    if (max_threads == 0 || max_threads > multi_ctx->count) {
        max_threads = multi_ctx->count;
    }

    pthread_t *threads = calloc(max_threads, sizeof(pthread_t));
    if (!threads) {
        return EFEX_ERR_MEMORY;
    }

    struct thread_task_arg *args = calloc(multi_ctx->count, sizeof(struct thread_task_arg));
    if (!args) {
        free(threads);
        return EFEX_ERR_MEMORY;
    }

    size_t thread_idx = 0;
    for (size_t i = 0; i < multi_ctx->count; i++) {
        args[i] = (struct thread_task_arg){
            .ctx = &multi_ctx->devices[i],
            .callback = callback,
            .user_data = user_data,
            .index = i,
            .result = -1
        };

        if (pthread_create(&threads[thread_idx], NULL, thread_task_wrapper, &args[i]) != 0) {
            args[i].result = EFEX_ERR_OPERATION_FAILED;
        } else {
            thread_idx++;
            if (thread_idx >= max_threads) {
                for (size_t j = 0; j < thread_idx; j++) {
                    pthread_join(threads[j], NULL);
                }
                thread_idx = 0;
            }
        }
    }

    for (size_t j = 0; j < thread_idx; j++) {
        pthread_join(threads[j], NULL);
    }

    int ret = EFEX_ERR_SUCCESS;
    for (size_t i = 0; i < multi_ctx->count; i++) {
        if (args[i].result != 0 && ret == EFEX_ERR_SUCCESS) {
            ret = args[i].result;
        }
    }

    free(threads);
    free(args);
    return ret;
}

struct flash_task_arg {
    struct sunxi_efex_ctx_t *ctx;
    const char *firmware_data;
    size_t firmware_size;
    uint32_t addr;
    enum sunxi_fes_data_type_t type;
    size_t index;
    int result;
};

static int flash_callback(struct sunxi_efex_ctx_t *ctx, size_t index, void *user_data) {
    struct flash_task_arg *arg = (struct flash_task_arg *)user_data;
    return sunxi_efex_fes_down(ctx, arg->firmware_data, arg->firmware_size,
                               arg->addr, arg->type);
}

int sunxi_efex_multi_flash_all(struct sunxi_efex_multi_ctx_t *multi_ctx,
                               const char *firmware_data,
                               size_t firmware_size,
                               uint32_t addr,
                               enum sunxi_fes_data_type_t type) {
    if (!multi_ctx || !firmware_data || firmware_size == 0) {
        return EFEX_ERR_NULL_PTR;
    }

    struct flash_task_arg arg = {
        .firmware_data = firmware_data,
        .firmware_size = firmware_size,
        .addr = addr,
        .type = type
    };

    int success = 0;
    for (size_t i = 0; i < multi_ctx->count; i++) {
        if (multi_ctx->info[i].state == EFEX_DEVICE_STATE_INITIALIZED) {
            int ret = flash_callback(&multi_ctx->devices[i], i, &arg);
            if (ret == EFEX_ERR_SUCCESS) {
                multi_ctx->info[i].state = EFEX_DEVICE_STATE_FLASHED;
                success++;
            } else {
                multi_ctx->info[i].state = EFEX_DEVICE_STATE_ERROR;
                multi_ctx->info[i].error_code = ret;
            }
        }
    }

    return success;
}

static void *flash_thread_wrapper(void *arg) {
    struct flash_task_arg *task = (struct flash_task_arg *)arg;
    task->result = sunxi_efex_fes_down(task->ctx, task->firmware_data,
                                      task->firmware_size, task->addr, task->type);
    return NULL;
}

int sunxi_efex_multi_flash_all_parallel(struct sunxi_efex_multi_ctx_t *multi_ctx,
                                        const char *firmware_data,
                                        size_t firmware_size,
                                        uint32_t addr,
                                        enum sunxi_fes_data_type_t type,
                                        size_t max_threads) {
    if (!multi_ctx || !firmware_data || firmware_size == 0) {
        return EFEX_ERR_NULL_PTR;
    }

    if (multi_ctx->count == 0) {
        return 0;
    }

    if (max_threads == 0 || max_threads > multi_ctx->count) {
        max_threads = multi_ctx->count;
    }

    pthread_t *threads = calloc(max_threads, sizeof(pthread_t));
    if (!threads) {
        return EFEX_ERR_MEMORY;
    }

    struct flash_task_arg *args = calloc(multi_ctx->count, sizeof(struct flash_task_arg));
    if (!args) {
        free(threads);
        return EFEX_ERR_MEMORY;
    }

    size_t success = 0;
    size_t thread_idx = 0;
    for (size_t i = 0; i < multi_ctx->count; i++) {
        if (multi_ctx->info[i].state != EFEX_DEVICE_STATE_INITIALIZED) {
            args[i].result = EFEX_ERR_OPERATION_FAILED;
            continue;
        }

        args[i] = (struct flash_task_arg){
            .ctx = &multi_ctx->devices[i],
            .firmware_data = firmware_data,
            .firmware_size = firmware_size,
            .addr = addr,
            .type = type,
            .index = i,
            .result = -1
        };

        if (pthread_create(&threads[thread_idx], NULL, flash_thread_wrapper, &args[i]) != 0) {
            args[i].result = EFEX_ERR_OPERATION_FAILED;
            multi_ctx->info[i].state = EFEX_DEVICE_STATE_ERROR;
        } else {
            thread_idx++;
            if (thread_idx >= max_threads) {
                for (size_t j = 0; j < thread_idx; j++) {
                    pthread_join(threads[j], NULL);
                    if (args[j].result == 0) {
                        multi_ctx->info[j].state = EFEX_DEVICE_STATE_FLASHED;
                        success++;
                    } else {
                        multi_ctx->info[j].state = EFEX_DEVICE_STATE_ERROR;
                        multi_ctx->info[j].error_code = args[j].result;
                    }
                }
                thread_idx = 0;
            }
        }
    }

    for (size_t j = 0; j < thread_idx; j++) {
        pthread_join(threads[j], NULL);
        if (args[j].result == 0) {
            multi_ctx->info[args[j].index].state = EFEX_DEVICE_STATE_FLASHED;
            success++;
        } else {
            multi_ctx->info[args[j].index].state = EFEX_DEVICE_STATE_ERROR;
            multi_ctx->info[args[j].index].error_code = args[j].result;
        }
    }

    free(threads);
    free(args);
    return (int)success;
}

int sunxi_efex_multi_reboot_all(struct sunxi_efex_multi_ctx_t *multi_ctx,
                                enum sunxi_fes_tool_mode_t tool_mode,
                                enum sunxi_fes_tool_mode_t next_mode) {
    if (!multi_ctx) {
        return EFEX_ERR_NULL_PTR;
    }

    int success = 0;
    for (size_t i = 0; i < multi_ctx->count; i++) {
        struct sunxi_efex_ctx_t *ctx = &multi_ctx->devices[i];
        if (multi_ctx->info[i].state == EFEX_DEVICE_STATE_INITIALIZED ||
            multi_ctx->info[i].state == EFEX_DEVICE_STATE_FLASHED) {
            if (sunxi_efex_fes_tool_mode(ctx, tool_mode, next_mode) == 0) {
                success++;
            }
        }
    }

    return success;
}

int sunxi_efex_multi_add_device(struct sunxi_efex_multi_ctx_t *multi_ctx,
                                 uint8_t bus, uint8_t port) {
    if (!multi_ctx || !multi_ctx->devices) {
        return EFEX_ERR_NULL_PTR;
    }

    if (multi_ctx->count >= multi_ctx->capacity) {
        return EFEX_ERR_OUT_OF_RANGE;
    }

    size_t existing_index;
    if (device_exists(multi_ctx, bus, port, &existing_index)) {
        return existing_index;
    }

    if (sunxi_usb_init(NULL) != EFEX_ERR_SUCCESS) {
        return EFEX_ERR_USB_INIT;
    }

    size_t idx = multi_ctx->count;
    struct sunxi_efex_ctx_t *ctx = &multi_ctx->devices[idx];
    struct sunxi_efex_device_info_t *info = &multi_ctx->info[idx];

    memset(ctx, 0, sizeof(struct sunxi_efex_ctx_t));
    memset(info, 0, sizeof(struct sunxi_efex_device_info_t));

    if (sunxi_scan_usb_device_at(ctx, bus, port) != EFEX_ERR_SUCCESS) {
        sunxi_usb_exit(NULL);
        return EFEX_ERR_USB_DEVICE_NOT_FOUND;
    }

    if (sunxi_usb_init(ctx) != EFEX_ERR_SUCCESS) {
        sunxi_usb_exit(ctx);
        sunxi_usb_exit(NULL);
        return EFEX_ERR_USB_INIT;
    }

    info->bus = bus;
    info->port = port;
    info->vid = ctx->usb_dev->descriptor.idVendor;
    info->pid = ctx->usb_dev->descriptor.idProduct;
    info->state = EFEX_DEVICE_STATE_CONNECTED;
    info->error_code = 0;
    multi_ctx->count++;

    return (int)idx;
}

int sunxi_efex_multi_remove_device(struct sunxi_efex_multi_ctx_t *multi_ctx, size_t index) {
    if (!multi_ctx || !multi_ctx->devices) {
        return EFEX_ERR_NULL_PTR;
    }

    if (index >= multi_ctx->count) {
        return EFEX_ERR_OUT_OF_RANGE;
    }

    if (multi_ctx->info[index].state == EFEX_DEVICE_STATE_INITIALIZED ||
        multi_ctx->info[index].state == EFEX_DEVICE_STATE_FLASHED) {
        sunxi_usb_exit(&multi_ctx->devices[index]);
    }

    size_t remaining = multi_ctx->count - index - 1;
    if (remaining > 0) {
        memmove(&multi_ctx->devices[index], &multi_ctx->devices[index + 1],
                remaining * sizeof(struct sunxi_efex_ctx_t));
        memmove(&multi_ctx->info[index], &multi_ctx->info[index + 1],
                remaining * sizeof(struct sunxi_efex_device_info_t));
    }

    multi_ctx->count--;
    return EFEX_ERR_SUCCESS;
}

int sunxi_efex_multi_refresh_devices(struct sunxi_efex_multi_ctx_t *multi_ctx) {
    if (!multi_ctx || !multi_ctx->devices) {
        return EFEX_ERR_NULL_PTR;
    }

    int refreshed = 0;
    for (size_t i = 0; i < multi_ctx->count; i++) {
        struct sunxi_efex_device_info_t *info = &multi_ctx->info[i];
        if (info->state == EFEX_DEVICE_STATE_DISCONNECTED ||
            info->state == EFEX_DEVICE_STATE_ERROR) {
            if (sunxi_usb_init(NULL) == EFEX_ERR_SUCCESS) {
                struct sunxi_efex_ctx_t *ctx = &multi_ctx->devices[i];
                memset(ctx, 0, sizeof(struct sunxi_efex_ctx_t));

                if (sunxi_scan_usb_device_at(ctx, info->bus, info->port) == EFEX_ERR_SUCCESS) {
                    if (sunxi_usb_init(ctx) == EFEX_ERR_SUCCESS) {
                        info->state = EFEX_DEVICE_STATE_CONNECTED;
                        info->error_code = 0;
                        refreshed++;
                    }
                } else {
                    sunxi_usb_exit(NULL);
                }
            }
        }
    }

    return refreshed;
}

size_t sunxi_efex_multi_count_by_state(struct sunxi_efex_multi_ctx_t *multi_ctx,
                                       enum sunxi_efex_device_state_t state) {
    if (!multi_ctx) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < multi_ctx->count; i++) {
        if (multi_ctx->info[i].state == state) {
            count++;
        }
    }

    return count;
}
