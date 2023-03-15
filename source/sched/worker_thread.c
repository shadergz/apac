
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

  schedgov_t *gov = apac_ctx->governor;

  spin_rlock (&gov->mutex);

  schedthread_t *self = sched_find (0, apac_ctx);
  sched_configure (self, apac_ctx);

  echo_assert (apac_ctx, self->executing == 0, "Thread is already executing!");
  self->executing = true;

  // We can change the thread name more than once inside contexts
  sched_setname (self->thread_name, apac_ctx);

  gov->threads_count++;
  spin_runlock (&gov->mutex);

  echo_success (apac_ctx,
                "Thread (%s) with id %lu was started\t[\e[0;32mON\e[0m]\n",
                self->thread_name, self->thread_handler);

  for (;;)
    thread_sleepby (100, THREAD_SLEEPCONV_SECONDS);
  return NULL;
}