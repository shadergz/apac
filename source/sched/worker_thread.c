
#include <pthread.h>
#include <unistd.h>

#include <sched/gov.h>
#include <sched/spin_lock.h>

#include <echo/fmt.h>
#include <thread/sleep.h>

void
worker_killsig (i32 thrsig)
{
  echo_success (NULL, "Thread %lu is being killed...\n", pthread_self ());

  pthread_exit (NULL);
}

void *
worker_entry (void *apac_ptr)
{
  apac_ctx_t *apac_ctx = (apac_ctx_t *)apac_ptr;
  if (!apac_ctx)
    pthread_exit (NULL);

  schedthread_t *self = sched_find (0, apac_ctx);
  sched_configure (self, apac_ctx);

  // We can change the thread name more than once inside contexts
  sched_setname (self->thread_name, apac_ctx);

  schedgov_t *gov = apac_ctx->governor;
  spin_rlock (&gov->mutex);
  gov->threads_count++;

  echo_success (apac_ctx, "Thread (%s) with id %u was started *\n",
                self->thread_name, self->thread_id);
  spin_runlock (&gov->mutex);

  for (;;)
    thread_sleepby (100, THREAD_SLEEPCONV_SECONDS);
  return NULL;
}