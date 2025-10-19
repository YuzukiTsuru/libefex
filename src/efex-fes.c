#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "efex-usb.h"
#include "efex-protocol.h"

int sunxi_efex_fes_query_storage(const struct sunxi_efex_ctx_t *ctx, uint32_t *storage_type) {
    return sunxi_usb_fes_xfer(ctx, FES_XFER_RECV, EFEX_CMD_FES_QUERY_STORAGE, NULL, 0,
                              (char *) storage_type, sizeof(uint32_t));
}

int sunxi_efex_fes_probe_flash_size(const struct sunxi_efex_ctx_t *ctx, uint32_t *flash_size) {
    return sunxi_usb_fes_xfer(ctx, FES_XFER_RECV, EFEX_CMD_FES_FLASH_SIZE_PROBE, NULL, 0,
                              (char *) flash_size, sizeof(uint32_t));;
}
