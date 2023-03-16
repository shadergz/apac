#include <pthread.h>
#include <sched.h>

#include <sched/gov.h>

#include <echo/fmt.h>

typedef struct sched_param native_sched_parameters_t;

i32
affinity_setnice (i32 nice, pthread_t thid)
{

  native_sched_parameters_t core_params = {};
  if (!thid)
    thid = pthread_self ();

  i32 po_ticpu = 0;
  pthread_getschedparam (thid, &po_ticpu, &core_params);
  if (!core_params.sched_priority)
    {
      echo_info (NULL,
                 "Affinity: Nice of thread %lu is "
                 "under undefined yet!",
                 thid);
    }

  if (nice < 0 || nice > 19)
    {
      echo_error (NULL, "Affinity: Invalid nice attribute\n");
      return -1;
    }
  core_params.sched_priority = nice;
  pthread_setschedparam (thid, po_ticpu, &core_params);

  return 0;
}

i32
affinity_set (u8 core, pthread_t thread, apac_ctx_t *apac_ctx)
{

  if (!thread)
    thread = pthread_self ();
  if (!core)
    core = sched_getcpu ();
  if (core >= MAX_POSSIBLE_CORES)
    return -1;

  cpu_set_t core_set = {};
  CPU_SET (core, &core_set);

  schedthread_t *th = sched_find (thread, apac_ctx);
#if defined(__ANDROID__)
  sched_setaffinity (th->native_tid, sizeof core_set, &core_set);

#elif defined(__linux__)
  pthread_setaffinity_np (thread, sizeof core_set, &core_set);

#else
  i32 brain_idx = 0;

  for (; brain_idx < (8 * (i32)sizeof (core_set)); brain_idx++)
    {
      if (CPU_ISSET (brain_idx, &core_set))
        break;
    }
  ...
#endif

  th->core_owner = core;

  return 0;
}
