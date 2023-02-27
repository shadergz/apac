#include <stdio.h>

#include <memctrlext.h>

#include <storage/tree.h>

#include <storage/fio.h>
#include <storage/flock.h>

#include <locker.h>

#include <echo/fmt.h>
#include <rt.h>

i32 locker_init(apac_ctx_t* apac_ctx) {

	lockerproc_t* locker = apac_ctx->locker;

	storage_fio_t* proc_file = (storage_fio_t*)apmalloc(sizeof(storage_fio_t));
	tree_open_file(proc_file, "lock.alock", "rwc:-", apac_ctx);

	locker->locker_pid = (u16)getpid();
	locker->saved_st = 0;
	locker->time = 0;

	return 0;
}

i32 locker_deinit(apac_ctx_t* apac_ctx) {
	return 0;
}

static const char* s_locker_format = 
	"ST:\b\b\b%04d\n"
	"PID:\b\b%04d\n"
	"TIME:\b%04d\n";

i32 locker_acquire(apac_ctx_t* apac_ctx) {

	storage_fio_t* driver = tree_getfile("lock.alock", apac_ctx);
	bool has_locked = driver->is_locked;

	i32 st, pid, time;

	fio_readf(driver, s_locker_format, &st, &pid, &time);

	if (has_locked) {
		while (fio_lock(driver, FIO_LOCKER_WRITE) != 0) {
			echo_info(apac_ctx, "The locker file is activated on %d "
				"process, waiting...\n", pid);

		}
	}

	lockerproc_t* locker = apac_ctx->locker;

	fio_writef(driver, s_locker_format, st, pid, time);

	echo_info(apac_ctx, "Locker acquired by PID: %d\n", locker->locker_pid);

	return 0;
}

// This function should be called when some signals like SISEGV has spawned!
i32 locker_release(apac_ctx_t* apac_ctx) {
	if (__builtin_expect(apac_ctx == NULL, 0)) {
		#define RUN_DIR_SZ 0x80
		char run[RUN_DIR_SZ];
		run_getedir((char**)&run, sizeof run - 1);

		echo_info(NULL, "Locker file %s removed\n");
		remove(run);

	}

	storage_fio_t* driver = tree_getfile("lock.alock", apac_ctx);

	fio_writef(driver, s_locker_format, -1, -1, -1);
	fio_finish(driver);

	return 0;
}

