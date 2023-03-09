#ifndef APAC_SCHED_SPIN_LOCK_H
#define APAC_SCHED_SPIN_LOCK_H

#include <api.h>

i32 spin_rtryunlock(spinlocker_t* mutex);
i32 spin_rtrylock(spinlocker_t* mutex); 

i32 spin_runlock(spinlocker_t* mutex);
i32 spin_rlock(spinlocker_t* mutex);

i32 spin_tryunlock(spinlocker_t *mutex);
i32 spin_trylock(spinlocker_t* mutex); 

i32 spin_unlock(spinlocker_t* mutex);
i32 spin_lock(spinlocker_t* mutex);

#endif


