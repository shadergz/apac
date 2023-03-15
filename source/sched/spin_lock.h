#ifndef APAC_SCHED_SPIN_LOCK_H
#define APAC_SCHED_SPIN_LOCK_H

#include <api.h>

#include <pthread.h>
#include <stdatomic.h>
#include <string.h>

i32 spin_rtryunlock (spinlocker_t *mutex);
i32 spin_rtrylock (spinlocker_t *mutex);

i32 spin_runlock (spinlocker_t *mutex);
i32 spin_rlock (spinlocker_t *mutex);

i32 spin_unlock (spinlocker_t *mutex);
i32 spin_lock (spinlocker_t *mutex);

[[maybe_unused]] static inline i32
spin_init (spinlocker_t *mutex)
{
  atomic_flag_clear (&mutex->locked);

  mutex->owner_thread = 0;
  mutex->recursive_lcount = 0;

  return 0;
}

[[maybe_unused]] static inline i32
spin_deinit (spinlocker_t *mutex)
{

  memset (mutex, 0, sizeof *mutex);
  return 0;
}

#endif
