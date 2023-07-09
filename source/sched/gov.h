#ifndef APAC_SCHED_GOV_H
#define APAC_SCHED_GOV_H

#include <api.h>

#include <default.h>

#define MAX_POSSIBLE_CORES CONFIG_DEFAULT_MAX_THREAD_MAX

i32 sched_init(apac_ctx_t* apac_ctx);
i32 sched_deinit(apac_ctx_t* apac_ctx);

u8 sched_getcount(const apac_ctx_t* apac_ctx);
schedthread_t* sched_find(pthread_t thread, apac_ctx_t* apac_ctx);
i32 sched_configure(schedthread_t* thinfo, apac_ctx_t* apac_ctx);
i32 sched_cleanup(schedthread_t* thread, apac_ctx_t* apac_ctx);

u8 super_getcores();
i32 scalar_cpuinfo(char* cpu_vendor, char* cpu_name, char* cpu_features,
    u64 vesz, u64 namesz, u64 featuresz, u8* cores_count,
    u8* threads_count);

u64 scalar_cpuname(char* cpu_nb, u64 cpu_nsz);

i32 sched_start(apac_ctx_t* apac_ctx);
i32 sched_stop(apac_ctx_t* apac_ctx);

i32 orchestra_spawn(schedthread_t* thread, apac_ctx_t* apac_ctx);
i32 orchestra_die(schedthread_t* thread, apac_ctx_t* apac_ctx);

void* worker_entry(void* apac_ptr);
void worker_killsig(i32 thrsig);

i32 sched_setname(const char* message, apac_ctx_t* apac_ctx);
i32 sched_unsetname(apac_ctx_t* apac_ctx);

#endif
