#ifndef APAC_BACKEND_SP_H
#define APAC_BACKEND_SP_H

#include <api.h>

i32 back_init (apac_ctx_t *apac_ctx);
i32 back_deinit (apac_ctx_t *apac_ctx);

i32 back_select_devices (apac_ctx_t *apac_ctx);

#endif
