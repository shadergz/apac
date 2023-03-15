
#include <stddef.h>
#include <string.h>

#include <pthread.h>
#include <signal.h>

#include <pthread.h>
#include <sys/prctl.h>

#include <sched/gov.h>

#include <memctrlext.h>
#include <vec.h>

#include <sched/spin_lock.h>

#include <echo/fmt.h>

i32
sched_unsetname (apac_ctx_t *apac_ctx)
{
  schedthread_t *thinfo = sched_find (pthread_self (), apac_ctx);
  if (thinfo == NULL)
    return -1;

  if (thinfo->context_name == NULL)
    return 0;

  prctl (PR_SET_NAME, "Undefined", 0, 0, 0);

  apfree ((char *)thinfo->context_name);
  return 0;
}

i32
sched_setname (const char *message, apac_ctx_t *apac_ctx)
{
  schedgov_t *gov = apac_ctx->governor;
  if (!gov || !message)
    return -1;

  u64 thread = pthread_self ();

  /* Avoiding ERANGE problems, because thread's name can only have 16 or less
   * bytes as a C string (including null byte) */
  if (strlen (message) > 15)
    return -1;

  schedthread_t *thinfo = sched_find (thread, apac_ctx);
  if (thinfo == NULL)
    return -1;

  sched_unsetname (apac_ctx);
  thinfo->context_name = strdup (message);

  prctl (PR_SET_NAME, thinfo->context_name, 0, 0, 0);
  return 0;
}

u8
sched_getcount (const apac_ctx_t *apac_ctx)
{
  const schedgov_t *gov = apac_ctx->governor;
  if (gov == NULL)
    return 0;
  if (gov->threads_info == NULL)
    return 0;

  const u8 count = (u8)vec_using (gov->threads_info);

  return count;
}

typedef struct sigaction native_sigaction_t;

static i32
sched_installsig (schedgov_t *governor)
{

  static native_sigaction_t action
      = { .sa_handler = worker_killsig, .sa_flags = SA_INTERRUPT };
  sigemptyset (&action.sa_mask);
  sigaddset (&action.sa_mask, SIGUSR1);
  pthread_sigmask (SIG_UNBLOCK, (const sigset_t *)&action.sa_mask,
                   &governor->thread_dflt);

  sigaction (SIGUSR1, (const native_sigaction_t *)&action, NULL);

  return 0;
}

static i32
sched_set_ss (apac_ctx_t *apac_ctx)
{
  schedgov_t *gov = apac_ctx->governor;

#if defined(__linux__)
  const u32 pages = getpagesize ();
#else
  const u32 pages = sysconf (_SC_PAGESIZE);
#endif

  pthread_attr_setstacksize (gov->thread_attrs, pages * 8);
  echo_info (apac_ctx,
             "Thread's stack size will be maximized to x.8, where X "
             "is _SC_PAGESIZE (%u)\n",
             pages * 8);

  return 0;
}

i32
sched_init (apac_ctx_t *apac_ctx)
{
  schedgov_t *gov = apac_ctx->governor;
  if (gov == NULL)
    return -1;

  gov->threads_info = (vecdie_t *)apmalloc (sizeof (vecdie_t));
  gov->thread_attrs = (pthread_attr_t *)apmalloc (sizeof (pthread_attr_t));

  spin_init (&gov->mutex);

#define SCHED_DEF_THREAD_CNT 8
  pthread_attr_init (gov->thread_attrs);
  sched_set_ss (apac_ctx);

  const i32 vec_ret = vec_init (sizeof (schedthread_t), SCHED_DEF_THREAD_CNT,
                                gov->threads_info);

  const u8 ccpu = super_getcores ();
  echo_assert (apac_ctx, ccpu > 0,
               "Pool: invalid count of cores, "
               "this must be reported!\n");
  if (ccpu != SCHED_DEF_THREAD_CNT)
    {
      vec_resize (ccpu, gov->threads_info);
    }
  gov->cores = ccpu;

  sched_installsig (gov);

  schedthread_t *this_thread = vec_emplace (gov->threads_info);
  if (vec_ret == 0 && (sched_configure (this_thread, apac_ctx) == 0))
    {
      gov->threads_count++;
      sched_setname ("Apac Core", apac_ctx);
      return 0;
    }

  vec_deinit (gov->threads_info);
  apfree (gov->threads_info);

  gov->threads_info = NULL;

  return -1;
}

schedthread_t *
sched_find (pthread_t thread, apac_ctx_t *apac_ctx)
{
  if (apac_ctx == NULL)
    return NULL;

  schedgov_t *gov = apac_ctx->governor;

  if (thread == 0)
    thread = pthread_self ();

  spin_rlock (&gov->mutex);
  vec_reset (gov->threads_info);
  schedthread_t *ret = NULL;

  for (schedthread_t *thinfo = NULL;
       (thinfo = vec_next (gov->threads_info)) != NULL && !ret;)
    {
      if (thinfo->thread_handler != thread)
        continue;
      ret = thinfo;
    }

  spin_runlock (&gov->mutex);
  vec_reset (gov->threads_info);
  return ret;
}

static const char *s_thread_name[]
    = { "Abigail", "Yone",     "Remu",   "Sara", "Mary",
        "Pandora", "Beatrice", "Emilia", "Lain", NULL };

#define THREAD_NSIZE sizeof (s_thread_name) / sizeof (const char *)

const char *
sched_sortname ()
{
  while (true)
    {
      u8 index = rand () % THREAD_NSIZE;
      const char *thname = s_thread_name[index];

      if (!thname)
        continue;
      s_thread_name[index] = NULL;

      return thname;
    }
}

i32
sched_configure (schedthread_t *thinfo, apac_ctx_t *apac_ctx)
{
  srand (time (NULL));
#define THREAD_MESSAGE_SIZE 0x100

  if (!thinfo)
    return -1;
  static i32 s_tname_index = 0;
  if (s_tname_index >= THREAD_NSIZE)
    return -1;

  if (!thinfo->thread_handler)
    thinfo->thread_handler = pthread_self ();

  thinfo->echo_size = THREAD_MESSAGE_SIZE;
  thinfo->echo_message = (char *)apmalloc (sizeof (char) * thinfo->echo_size);

  thinfo->thread_name = sched_sortname ();

  thinfo->native_tid = gettid ();
  thinfo->core_owner = sched_getcpu ();

  return 0;
}

i32
sched_cleanup (schedthread_t *thread, apac_ctx_t *apac_ctx)
{
  if (thread->thread_name)
    thread->thread_name = NULL;
  if (thread->echo_message)
    {
      apfree ((char *)thread->echo_message);
      thread->echo_message = NULL;
      thread->echo_size = 0;
    }
  if (thread->context_name)
    apfree ((char *)thread->context_name);

  memset (thread, 0, sizeof *thread);

  return 0;
}

i32
sched_deinit (apac_ctx_t *apac_ctx)
{
  schedgov_t *gov = apac_ctx->governor;

  if (gov == NULL)
    return -1;

  if (!gov->threads_info)
    return 0;

  /* Removing the entire thread data list, at this point, just only a thread
   * must exist */
  vec_deinit (gov->threads_info);
  if (gov->threads_info)
    apfree (gov->threads_info);

  pthread_attr_destroy (gov->thread_attrs);
  if (gov->thread_attrs)
    apfree (gov->thread_attrs);

  spin_deinit (&gov->mutex);

  return 0;
}
