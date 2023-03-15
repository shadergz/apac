
#include <sched/gov.h>
#include <sched/spin_lock.h>
#include <thread/sleep.h>

#include <echo/fmt.h>

#include <memctrlext.h>
#include <vec.h>

static i32
sched_dump_cpuinfo (apac_ctx_t *apac_ctx)
{
#define CPU_REAL_NAME 0x40
#define CPU_VENDOR_MODELSZ 0x40
#define CPU_FEATURESSZ 0x70

  char *cpu_realname = (char *)apmalloc (sizeof (char) * CPU_REAL_NAME);
  char *cpu_vendormodel
      = (char *)apmalloc (sizeof (char) * CPU_VENDOR_MODELSZ);
  char *cpu_features = (char *)apmalloc (sizeof (char) * CPU_FEATURESSZ);

  if (!cpu_realname || !cpu_vendormodel || !cpu_features)
    {
      echo_error (apac_ctx,
                  "Has occured a problem when attempt to "
                  "allocate 3 blocks for store hosts CPU data information\n");
      return -1;
    }

  scalar_cpuinfo (cpu_vendormodel, cpu_realname, cpu_features,
                  CPU_VENDOR_MODELSZ, CPU_REAL_NAME, CPU_FEATURESSZ, NULL,
                  NULL);
  if (!*cpu_realname)
    scalar_cpuname (cpu_realname, CPU_REAL_NAME);

  echo_success (apac_ctx,
                "Sched Thread Pool is taking controller of:\n"
                "\tName: %s\n"
                "\tVendor: %s\n"
                "\tCPU features: %s\n",
                cpu_realname, cpu_vendormodel, cpu_features);

  apfree (cpu_realname);
  apfree (cpu_vendormodel);
  apfree (cpu_features);

  return 0;
}

/* Spawn all physical threads as logical workers after fetcher the
 * real core count from the host CPU */
i32
sched_start (apac_ctx_t *apac_ctx)
{
  sched_dump_cpuinfo (apac_ctx);

  schedgov_t *scheduler = apac_ctx->governor;
  // Getting from the vector capacity value, cause we already preallocate it
  // somewhere else
  u8 cores_count = vec_capacity (scheduler->threads_info);

  i32 thread_created = 0;
  // Minus (1) cause we already have a thread being executed (us!, doooh)
  for (thread_created = 0; thread_created < cores_count - 1; thread_created++)
    {
      spin_rlock (&scheduler->mutex);
      schedthread_t *newth = vec_emplace (scheduler->threads_info);

      const i32 orret = orchestra_spawn (newth, apac_ctx);
      spin_runlock (&scheduler->mutex);

      echo_debug (apac_ctx, "New thread has created in %p\n", newth);

      if (orret != 0)
        {
          echo_error (apac_ctx, "Can't creates a new thread in %p\n", newth);
        }
    }

  // We can't have just one thread, this is an insane situation
  return thread_created != 0 ? 0 : -1;
}

i32
sched_stop (apac_ctx_t *apac_ctx)
{
  schedgov_t *gov = apac_ctx->governor;

  while (true)
    {
      thread_sleepby (10, THREAD_SLEEPCONV_MILI);
      spin_rlock (&gov->mutex);

      u8 threads = gov->threads_count;
      u8 cores = gov->cores;

      if (threads == cores)
        break;

      spin_runlock (&gov->mutex);
    }

  echo_success (apac_ctx, "Thereads created: %d, threads running %d\n",
                gov->threads_count, gov->cores);

  const u8 thread_count = vec_capacity (gov->threads_info);

  vec_reset (gov->threads_info);
  // We can't detach the main thread, just clean up it!
  schedthread_t *main_thread = vec_next (gov->threads_info);
  for (i32 thidx = 1; thidx < thread_count; thidx++)
    {
      schedthread_t *thXX = vec_next (gov->threads_info);
      orchestra_die (thXX, apac_ctx);
      /* This can be done outside the thread, we can kill the thread and after
       * remove it's data! */
      sched_cleanup (thXX, apac_ctx);
    }

  sched_cleanup (main_thread, apac_ctx);
  spin_runlock (&gov->mutex);

  return 0;
}
