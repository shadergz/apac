#ifndef APAC_SCHED_SPIN_LOCK_H
#define APAC_SCHED_SPIN_LOCK_H

#include <api.h>

#include <stdatomic.h>
#include <string.h>

#include <echo/fmt.h>

i32 spin_runlock(spinlocker_t* mutex);
i32 spin_rlock(spinlocker_t* mutex);

i32 spin_rtrylock(spinlocker_t* mutex);
i32 spin_rtryunlock(spinlocker_t* mutex);

[[maybe_unused]] static inline i32
spin_init(spinlocker_t* mutex)
{
    atomic_init(&mutex->locker, 0);
    echo_assert(NULL, atomic_is_lock_free(&mutex->locker),
        "This SPINLOCKER implementation isn't valid for your machine!");

    mutex->pid_owner = 0;
    mutex->count = 0;

    return 0;
}

[[maybe_unused]] static inline i32
spin_deinit(spinlocker_t* mutex)
{

    memset(mutex, 0, sizeof *mutex);
    return 0;
}

#endif
