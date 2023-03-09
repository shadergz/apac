#ifndef APAC_SCHED_GOV_H
#define APAC_SCHED_GOV_H

#include <api.h>

#define MAX_POSSIBLE_CORES 32
#define SOLVE_CORE_ID MAX_POSSIBLE_CORES

i32 sched_init (apac_ctx_t *apac_ctx);
i32 sched_deinit (apac_ctx_t *apac_ctx);

u8 sched_getcount (const apac_ctx_t *apac_ctx);
schedthread_t *sched_find (u32 thread, apac_ctx_t *apac_ctx);
schedthread_t *sched_configure (u32 thread, apac_ctx_t *apac_ctx);

u8 super_getcores ();
i32 scalar_cpuinfo (char *cpu_vendor, char *cpu_name, char *cpu_features,
                    u64 vesz, u64 namesz, u64 featuresz, u8 *cores_count,
                    u8 *threads_count);

u64 scalar_cpuname (char *cpu_nb, u64 cpu_nsz);

i32 sched_start (apac_ctx_t *apac_ctx);
i32 sched_stop (apac_ctx_t *apac_ctx);

#endif
