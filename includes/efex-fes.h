#ifndef LIBEFEX_EFEX_FES_H
#define LIBEFEX_EFEX_FES_H

#include "efex-common.h"

int sunxi_efex_fes_query_storage(const struct sunxi_efex_ctx_t *ctx, uint32_t *storage_type);

int sunxi_efex_fes_probe_flash_size(const struct sunxi_efex_ctx_t *ctx, uint32_t *flash_size);

#endif //LIBEFEX_EFEX_FES_H