#include <stdio.h>
#include <string.h>

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
	tree_open_file(proc_file, "./lock.alock", "rwc:-", apac_ctx);

	locker->locker_pid = (u16)getpid();
	locker->saved_st = 0;
	locker->time = 0;

	return 0;
}

i32 locker_deinit(apac_ctx_t* apac_ctx) {
	bool was_closed;
	lockerproc_t* locker = apac_ctx->locker;
	storage_fio_t* proc_file = tree_close_file(&was_closed, "./lock.alock", apac_ctx);

	if (was_closed != true) {
		echo_error(apac_ctx, "Lock file wasn't closed");
		return -1;
	}

	apfree(proc_file);
	memset(locker, 0, sizeof(*locker));

	return 0;
}

static const char* s_locker_format = 
	"ST:    %04d\n"
	"PID:   %04d\n"
	"TIME:  %04d\n";

i32 locker_acquire(apac_ctx_t* apac_ctx) {
	
	storage_fio_t* driver = tree_getfile("./lock.alock", apac_ctx);
	if (!driver) {
		echo_error(apac_ctx, "Can't locate the locker process file inside the tree\n");
		return -1;
	}

	bool has_locked = driver->is_locked;

	i32 st = 0, pid = 0, time = 0;

	fio_seekbuffer(driver, 0, FIO_SEEK_SET);
	fio_readf(driver, s_locker_format, &st, &pid, &time);

	if (has_locked) {
		while (fio_lock(driver, FIO_LOCKER_WRITE) != 0) {
			echo_info(apac_ctx, "The locker file is activated on %d "
				"process, waiting...\n", pid);

		}
	}
	fio_lock(driver, FIO_LOCKER_WRITE);
	lockerproc_t* locker = apac_ctx->locker;

	fio_seekbuffer(driver, 0, FIO_SEEK_SET);
	fio_writef(driver, s_locker_format, locker->saved_st, 
		locker->locker_pid, locker->time);

	echo_info(apac_ctx, "Locker acquired by PID: %d\n", locker->locker_pid);

	return 0;
}

// This function should be called when some signals like SISEGV has spawned!
i32 locker_release(apac_ctx_t* apac_ctx) {
	if (__builtin_expect(apac_ctx == NULL, 0)) {
		#define RUN_DIR_SZ 0x80
		char run[RUN_DIR_SZ];
		run_getedir((char**)&run, sizeof run - 1);

		remove(run);
		echo_info(NULL, "Locker file %s was removed\n");
	}

	storage_fio_t* driver = tree_getfile("./lock.alock", apac_ctx);
	if (driver == NULL) {
		echo_error(apac_ctx, "Locker wan't found, this is a serious bug!\n");
		return -1;

	}

	fio_seekbuffer(driver, 0, FIO_SEEK_SET);
	fio_writef(driver, s_locker_format, -1, -1, -1);

	return 0;
}

