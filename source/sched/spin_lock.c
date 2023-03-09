#include <pthread.h>
#include <sched.h>

#include <stdatomic.h>

#include <sched/spin_lock.h>
#include <thread/sleep.h>
#include <thread/ctx_now.h>

static i32 spin_wait(spinlocker_t* mutex) {

	#define WAIT_YIELD_BY 64
	#define WAIT_SLEEP_BY 2
	#define WAIT_SLEEP_MS 500 // 1/2 seconds

	#define WAIT_MESSAGE_LEN 0xa

	bool waiting[1] = {true};
	u64 attempts[1] = {};
	char wait_[WAIT_MESSAGE_LEN] = {};

	do {
		++attempts[0];
		if (spin_rtrylock(mutex) == 0) {
			waiting[0] = false; continue;
		} else if (spin_trylock(mutex) == 0) {
			waiting[0] = false; continue;
		}

		// Putting the current thread into end of the CPU priority queue!
		if (!(attempts[0] % WAIT_YIELD_BY)) {
			thread_save(wait_, sizeof wait_, 0, "Yield state");
			sched_yield();
			thread_restore(0, wait_);
		}

		if (!(attempts[0] % WAIT_SLEEP_BY)) {
			thread_save(wait_, sizeof wait_, 0, "Sleep state");
			thread_sleepby(WAIT_SLEEP_MS, THREAD_SLEEPCONV_MILI);
			thread_restore(0, wait_);
		}
	} while (waiting[0] == true);
	return waiting[0];
}

i32 spin_lock(spinlocker_t* mutex) {
	if (spin_trylock(mutex) == 0) return 0;

	spin_wait(mutex);
	return spin_lock(mutex);
}

i32 spin_unlock(spinlocker_t* mutex) {
	if (spin_tryunlock(mutex) == 0) return 0;

	volatile bool last = atomic_flag_test_and_set_explicit(
			&mutex->spin, memory_order_acq_rel);

	if (last == 0) spin_wait(mutex); 

	return spin_unlock(mutex);
}

i32 spin_trylock(spinlocker_t* mutex) {
	// Already locked
	if (atomic_flag_test_and_set_explicit(&mutex->spin, 
		memory_order_acquire)) return 0;

	return 0;
}

i32 spin_tryunlock(spinlocker_t* mutex) {
	if (!atomic_flag_test_and_set_explicit(&mutex->spin, 
		memory_order_acq_rel)) return 0;

	atomic_flag_clear_explicit(&mutex->spin, memory_order_release);
	return 0;
}

i32 spin_rlock(spinlocker_t* mutex) {
	bool endf = false;

	if (spin_rtrylock(mutex) == 0) return 0;
	volatile pthread_t acthread = pthread_self();

	while (!endf) {
		if (mutex->owner_thread != 0 && 
			mutex->owner_thread != acthread) {

			spin_wait(mutex);
			mutex->owner_thread = acthread;
		}
	}

	return spin_rtrylock(mutex);
}

i32 spin_rtryunlock(spinlocker_t* mutex) {
	if (mutex->owner_thread && 
		mutex->owner_thread != pthread_self()) return -1;
	if (--mutex->recursive_lcount > 0) return 0;
	
	if (spin_tryunlock(mutex) != 0) return -1;

	mutex->owner_thread = (pthread_t)0;

	return 0;
}

i32 spin_runlock(spinlocker_t* mutex) {
	if (spin_rtryunlock(mutex) == 0) return 0;
	spin_wait(mutex);

	return spin_runlock(mutex);
}

i32 spin_rtrylock(spinlocker_t* mutex) {
	if (mutex->owner_thread != pthread_self()) return -1;
	if (mutex->recursive_lcount == 0) spin_trylock(mutex);
	
	mutex->recursive_lcount++;
	return 0;
}



