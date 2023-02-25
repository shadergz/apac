#ifndef APAC_POOL_GOV_H
#define APAC_POOL_GOV_H

#include <api.h>

i32 sched_init(apac_ctx_t* apac_ctx);
i32 sched_deinit(apac_ctx_t* apac_ctx);

u8 sched_getcount(const apac_ctx_t* apac_ctx); 
schedthread_t* sched_find(u32 thread, apac_ctx_t* apac_ctx); 
schedthread_t* sched_configure(u32 thread, apac_ctx_t* apac_ctx); 


#endif


