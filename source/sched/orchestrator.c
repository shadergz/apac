
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include <echo/fmt.h>
#include <sched/gov.h>
#include <sched/spin_lock.h>

#include <memctrlext.h>
#include <vec.h>

i32
orchestra_spawn (schedthread_t *thread, apac_ctx_t *apac_ctx)
{
  schedgov_t *gov = apac_ctx->governor;

  pthread_create (&thread->thread_handler, gov->thread_attrs, worker_entry,
                  (void *)apac_ctx);
  return gov != NULL ? 0 : -1;
}

i32
orchestra_die (schedthread_t *thread, apac_ctx_t *apac_ctx)
{
  /* Stopping everything what thread is doing, this is more insecure than a
   * checkpoint, but will garant that the thread will stop when we want! */
  const i32 retkill = pthread_kill (thread->thread_handler, SIGUSR1);
  if (retkill != 0)
    {
      echo_error (NULL, "Can't stop the thread %lu (%s)\n",
                  thread->thread_handler, thread->thread_name);
    }

  // Detaching all thread's resource, if there's one!
  pthread_detach (thread->thread_handler);

  schedgov_t *gov = apac_ctx->governor;

  spin_rlock (&gov->mutex);
  gov->threads_count--;

  echo_success (apac_ctx, "Thread %s with id %u has detached\n",
                thread->thread_name, thread->thread_id);
  spin_runlock (&gov->mutex);

  // This can be done outside the thread, we can kill the thread and after
  // remove it's data
  const i32 cc = sched_cleanups (thread, apac_ctx);

  return cc;
}

i32
orchestra_migrate (schedthread_t *from, schedthread_t *to)
{
  return 0;
}

i32
orchestra_exchange (schedthread_t *from, schedthread_t *to)
{
  return 0;
}

i32
orchestra_movelow (schedthread_t *thi)
{
  return 0;
}

i32
orchestra_movetop (schedthread_t *thi)
{
  return 0;
}

i32
orchestra_maxpriority (schedthread_t *thi)
{
  return 0;
}

i32
orchestra_coordinate (apac_ctx_t *apac_ctx)
{
  return 0;
}
