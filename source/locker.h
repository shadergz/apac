#ifndef APAC_LOCKER_H
#define APAC_LOCKER_H

#include <api.h>

i32 locker_acquire(apac_ctx_t* apac_ctx);
i32 locker_release(apac_ctx_t* apac_ctx);

i32 locker_init(apac_ctx_t* apac_ctx);
i32 locker_deinit(apac_ctx_t* apac_ctx);

#endif


