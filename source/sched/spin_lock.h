#ifndef APAC_SCHED_SPIN_LOCK_H
#define APAC_SCHED_SPIN_LOCK_H

#include <api.h>

#include <pthread.h>
#include <stdatomic.h>
#include <string.h>

i32 spin_runlock (spinlocker_t *mutex);
i32 spin_rlock (spinlocker_t *mutex);

[[maybe_unused]] static inline i32
spin_init (spinlocker_t *mutex)
{
  atomic_init (&mutex->locker, 0);

  mutex->pid_owner = 0;
  mutex->count = 0;

  return 0;
}

[[maybe_unused]] static inline i32
spin_deinit (spinlocker_t *mutex)
{

  memset (mutex, 0, sizeof *mutex);
  return 0;
}

#endif
