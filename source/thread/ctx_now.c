#include <pthread.h>
#include <string.h>

#include <thread/ctx_now.h>

i32 thread_save(char* laststatus, u64 lsize, pthread_t thread_id, const char* save) {
	if (!save || !laststatus) return -1;
	if (strlen(save) > 15)    return -1;

	pthread_getname_np(thread_id, laststatus, lsize);
	pthread_setname_np(thread_id, save);

	return 0;
}

i32 thread_restore(pthread_t thread_id, const char* restore) {
	pthread_setname_np(thread_id, restore);
	return 0;
}

