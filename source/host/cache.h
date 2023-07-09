#ifndef APAC_HOST_CACHE_H
#define APAC_HOST_CACHE_H

#include <api.h>

typedef enum cache_prefetch_level {
    CACHE_PREFETCH_LOW_READ,
    CACHE_PREFETCH_LOW_WRITE,
} cache_prefetch_level_e;

static inline void
cache_prefetch_low(void* mem_data, u64 hint)
{
    __builtin_prefetch((void*)((u8*)mem_data + hint), 0, 1);
}
static inline void
cache_prefetch_low_write(void* mem_data, u64 hint)
{
    __builtin_prefetch((void*)((u8*)mem_data + hint), 1, 1);
}

static inline void
cache_prefetch(void* mem_data, u64 hint, cache_prefetch_level_e level)
{
    switch (level) {
    case CACHE_PREFETCH_LOW_READ:
        cache_prefetch_low(mem_data, hint);
        break;
    case CACHE_PREFETCH_LOW_WRITE:
        cache_prefetch_low_write(mem_data, hint);
        break;
    }
}
#endif
