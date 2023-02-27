#include <locker.h>

#include <echo/fmt.h>
#include <rt.h>

i32 locker_init(apac_ctx_t* apac_ctx) {

	storage_tree_t* root = apac_ctx->root;
	lockerproc_t* locker = apac_ctx->locker;
	

	return 0;
}

i32 locker_deinit(apac_ctx_t* apac_ctx) {
	return 0;
}

i32 locker_acquire(apac_ctx_t* apac_ctx) {
	return 0;
}

// This function should be called when some signals like SISEGV has spawned!
i32 locker_release(apac_ctx_t* apac_ctx) {
	if (__builtin_expect(apac_ctx == NULL, 0)) {
		#define RUN_DIR_SZ 0x100
		char run[RUN_DIR_SZ];
		run_getedir(&run, sizeof run - 1);

	}
	
	return 0;
}

