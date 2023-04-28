#include <pthread.h>
#include <sched.h>

#include <stdatomic.h>

#include <sched/spin_lock.h>
#include <thread/ctx_now.h>
#include <thread/sleep.h>

static i32
spin_acquire (spinlocker_t *mutex, bool test)
{
  const i32 mt = atomic_exchange (&mutex->locker, test);
  return mt != 0;
}

static i32
spin_release (spinlocker_t *mutex)
{
  atomic_store (&mutex->locker, 0);
  return 0;
}

static __thread u64 g_attempts[1] = {};

#define WAIT_YIELD_BY 16
#define WAIT_SLEEP_BY 512
#define WAIT_SLEEP_MS 200 /* 1/5 seconds */

#define WAIT_MESSAGE_LEN 0x10

static void
spin_dirty ()
{
  char wait_[WAIT_MESSAGE_LEN] = {};
  ++g_attempts[0];

  // Putting the current thread into end of the CPU priority queue!
  if (!(g_attempts[0] % WAIT_YIELD_BY))
    {
      thread_save (wait_, sizeof wait_, 0, "Yield state");
      sched_yield ();
      thread_restore (0, wait_);
    }

  if (!(g_attempts[0] % WAIT_SLEEP_BY))
    {
      thread_save (wait_, sizeof wait_, 0, "Sleep state");
      thread_sleepby (WAIT_SLEEP_MS, THREAD_SLEEPCONV_MILI);
      thread_restore (0, wait_);
    }
}

static i32
spin_wait (spinlocker_t *mutex, bool acquire)
{
  bool waiting[1] = { true };

  waiting[0] = spin_acquire (mutex, acquire);
  if (acquire == 0 && waiting[0])
    {
      return 0;
    }

  if (acquire == 1 && waiting[0])
    {
      spin_dirty ();
      return 1;
    }

  return 0;
}

i32
spin_lock (spinlocker_t *mutex)
{
  i32 acquired = 0;
  g_attempts[0] = 0;
  do
    {
      acquired = !spin_wait (mutex, true);
    }
  while (acquired == 0);

  return 0;
}

i32
spin_unlock (spinlocker_t *mutex)
{
  i32 unlocked = 0;
  g_attempts[0] = 0;
  do
    {
      unlocked = !spin_wait (mutex, false);
    }
  while (unlocked == 0);

  return 0;
}

i32
spin_rtrylock (spinlocker_t *mutex)
{
  pthread_t pself = pthread_self ();

  if (spin_acquire (mutex, true) == 0)
    return 1;

  if (!pthread_equal (mutex->pid_owner, pself))
    return -1;

  mutex->count++;

  return 0;
}

i32
spin_rlock (spinlocker_t *mutex)
{
  volatile pthread_t acthread = pthread_self ();
  i32 locked;
  g_attempts[0] = 0;
  do
    {
      locked = spin_rtrylock (mutex);
      if (locked == -1)
        {
          locked = !spin_wait (mutex, true);
        }

      if (locked == 1)
        {
          mutex->pid_owner = acthread;
        }
    }
  while (locked == -1 || locked == 0);
  return 0;
}

i32
spin_rtryunlock (spinlocker_t *mutex)
{
  if (!mutex->pid_owner || !pthread_equal (mutex->pid_owner, pthread_self ()))
    return -1;

  if (!mutex->count)
    {
      mutex->pid_owner = (pthread_t)0;
      return 1;
    }

  --mutex->count;

  return 0;
}

i32
spin_runlock (spinlocker_t *mutex)
{
  i32 locked;
  g_attempts[0] = 0;
  do
    {
      locked = spin_rtryunlock (mutex);
      if (locked == -1)
        {
          locked = !spin_wait (mutex, false);
        }

      if (locked == 1)
        {
          spin_release (mutex);
        }
    }
  while (locked == -1 || locked == 0);

  return 0;
}
