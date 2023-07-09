#ifndef APAC_THREAD_CTX_NOW_H
#define APAC_THREAD_CTX_NOW_H

#include <pthread.h>

#include <api.h>

i32 thread_save(char* laststatus, u64 lsize, pthread_t thread_id,
    const char* save);

i32 thread_restore(pthread_t thread_id, const char* restore);

#endif
