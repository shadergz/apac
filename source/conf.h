#ifndef APAC_CONF_H
#define APAC_CONF_H

#include <api.h>

i32 conf_init(apac_ctx_t* apac_ctx);
i32 conf_load(apac_ctx_t* apac_ctx);

i32 conf_save(apac_ctx_t* apac_ctx);

i32 conf_deinit(apac_ctx_t* apac_ctx);

#endif
