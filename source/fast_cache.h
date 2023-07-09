#ifndef APAC_FAST_CACHE_H
#define APAC_FAST_CACHE_H

#include <api.h>
i32 cache_dump_info(apac_ctx_t* apac_ctx);
u64 cache_reload(apac_ctx_t* apac_ctx);
i32 cache_init(apac_ctx_t* apac_ctx);
i32 cache_sync(apac_ctx_t* apac_ctx);
i32 cache_fetch(cache_entry_t* entries, u64 entries_count, u64 entry_size,
    cache_type_e retr_type, apac_ctx_t* apac_ctx);
i32 cache_update(cache_entry_t* entry, u64 entry_size, u64 epos,
    apac_ctx_t* apac_ctx);
i32 cache_deinit(apac_ctx_t* apac_ctx);

#endif
