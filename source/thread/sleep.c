
#include <time.h>
#include <errno.h>
#include <math.h>

#include <thread/sleep.h>
#include <thread/ctx_now.h>

#include <echo/fmt.h>

typedef struct timespec native_timespec_t;

i32 thread_sleepby(u64 by, thread_sleepconv_e conv) {
	if (!by) return 0;
	native_timespec_t request = {};

	switch (conv) {
	case THREAD_SLEEPCONV_SECONDS: request.tv_sec = by; break;
	case THREAD_SLEEPCONV_MILI:
		const u64 nano = by * (u64)pow(10, 6);
		request.tv_nsec = nano;
	case THREAD_SLEEPCONV_NANO:    request.tv_nsec = by; break;
	default: return -1;
	}

	// The value of the nanoseconds field must be in the range 0 to 999999999
	if (request.tv_nsec > 999999999) return -1;

	const u64 thread = pthread_self();
	
	echo_success(NULL, "Thread %lu being to sleep now!\n", thread);
#define THREAD_SLEEP_BF 0x10
	char sleep_bf[THREAD_SLEEP_BF] = {};

	thread_save(sleep_bf, sizeof sleep_bf, thread, "Sleeping...");
	const i32 nret = nanosleep(&request, NULL);

	if (nret == EINTR) {
		echo_error(NULL, "Thread %lu was wakeuped because of a signal\n", thread);
		thread_save(sleep_bf, sizeof sleep_bf, thread, "SIG Wakeuped!");

		return -1;
	}

	thread_restore(thread, sleep_bf);
	return 0;
}

