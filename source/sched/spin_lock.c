#include <pthread.h>
#include <sched.h>

#include <stdatomic.h>

#include <sched/spin_lock.h>
#include <thread/ctx_now.h>
#include <thread/sleep.h>

enum spin_wait_mode
{
  SPIN_WAIT_ACQUIRE,
  SPIN_WAIT_RELEASE
};

static i32
spin_acquire (spinlocker_t *mutex)
{
  const i32 mt = atomic_exchange (&mutex->locker, 1);
  return mt != 0;
}

static i32
spin_release (spinlocker_t *mutex)
{
  atomic_store (&mutex->locker, 0);
  return 0;
}

static i32
spin_wait (spinlocker_t *mutex, enum spin_wait_mode mode)
{
#define SPIN_TEST(war, mode, mutex)                                           \
  if (mode == SPIN_WAIT_ACQUIRE)                                              \
    war = spin_acquire (mutex);                                               \
  if (mode == SPIN_WAIT_RELEASE)                                              \
    war = spin_release (mutex);

#define WAIT_YIELD_BY 16
#define WAIT_SLEEP_BY 512
#define WAIT_SLEEP_MS 200 /* 1/5 seconds */

#define WAIT_MESSAGE_LEN 0x10

  bool waiting[1] = { true };
  u64 attempts[1] = {};
  char wait_[WAIT_MESSAGE_LEN] = {};
  do
    {
      ++attempts[0];
      SPIN_TEST (waiting[0], mode, mutex);
      if (waiting[0] == false)
        continue;

      // Putting the current thread into end of the CPU priority queue!
      if (!(attempts[0] % WAIT_YIELD_BY))
        {
          thread_save (wait_, sizeof wait_, 0, "Yield state");
          sched_yield ();
          thread_restore (0, wait_);
        }

      if (!(attempts[0] % WAIT_SLEEP_BY))
        {
          thread_save (wait_, sizeof wait_, 0, "Sleep state");
          thread_sleepby (WAIT_SLEEP_MS, THREAD_SLEEPCONV_MILI);
          thread_restore (0, wait_);
        }
    }
  while (waiting[0] == true);
  return waiting[0];
}

i32
spin_lock (spinlocker_t *mutex)
{
  i32 acq = 0;
  do
    {
      acq = spin_acquire (mutex);
      spin_wait (mutex, SPIN_WAIT_ACQUIRE);
    }
  while (acq != 0);

  return 0;
}

i32
spin_unlock (spinlocker_t *mutex)
{
  i32 last = 0;
  do
    {
      last = spin_release (mutex);
      spin_wait (mutex, SPIN_WAIT_RELEASE);
    }
  while (last != 0);

  return 0;
}

i32
spin_rtrylock (spinlocker_t *mutex)
{
  pthread_t pself = pthread_self ();
  if (!pthread_equal (mutex->pid_owner, pself))
    return -1;

  if (spin_acquire (mutex) == 0)
    return 0;
  mutex->count++;

  return 0;
}

i32
spin_rlock (spinlocker_t *mutex)
{
  volatile pthread_t acthread = pthread_self ();

  if (!mutex->pid_owner)
    mutex->pid_owner = acthread;

  if (spin_rtrylock (mutex) == 0)
    return 0;

  do
    {
      spin_wait (mutex, SPIN_WAIT_ACQUIRE);
      mutex->pid_owner = acthread;
    }
  while (mutex->pid_owner != acthread);
  return 0;
}

i32
spin_rtryunlock (spinlocker_t *mutex)
{
  if (!mutex->pid_owner || !pthread_equal (mutex->pid_owner, pthread_self ()))
    return -1;

  if (!mutex->count)
    goto spinclean;

  --mutex->count;

  return 0;

spinclean:
  __attribute__ ((cold));

  const i32 sret = spin_release (mutex);
  mutex->pid_owner = (pthread_t)0;
  return sret;
}

i32
spin_runlock (spinlocker_t *mutex)
{
  if (spin_rtryunlock (mutex) == 0)
    return 0;

  spin_wait (mutex, SPIN_WAIT_RELEASE);

  return 0;
}
