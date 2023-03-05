
#include <stdio.h>
#include <string.h>

#include <api.h>
#include <memctrlext.h>
#include <session.h>
#include <rt.h>
#include <echo/fmt.h>

#include <pool/gov.h>
#include <inner.h>

#define WRAPPER_TYPE_TO_STR(type) #type
#define TYPE_2_STR(type)\
	WRAPPER_TYPE_TO_STR(typeof(type))

static i32 apac_san(apac_ctx_t* apac_ctx) {
	const char* symbol_name = NULL;

	if (apac_ctx->user_session == NULL)
		symbol_name = WRAPPER_TYPE_TO_STR(apac_ctx->user_session);
	else if (apac_ctx->echo_system == NULL)
		symbol_name = WRAPPER_TYPE_TO_STR(apac_ctx->echo_system);
	else if (apac_ctx->core_backend == NULL)
		symbol_name = WRAPPER_TYPE_TO_STR(apac_ctx->core_backend);
	else if (apac_ctx->governor == NULL)
		symbol_name = WRAPPER_TYPE_TO_STR(apac_ctx->governor);
	else if (apac_ctx->root == NULL)
		symbol_name = WRAPPER_TYPE_TO_STR(apac_ctx->root);
	else if (apac_ctx->locker == NULL)
		symbol_name = WRAPPER_TYPE_TO_STR(apac_ctx->locker);

	if (symbol_name != NULL) goto nonallocated;

	return 0;

nonallocated:
	echo_error(apac_ctx, "%s wasn't allocated, this is a non irrecuperable status", 
		symbol_name);
	return -1;
}

static i32 apac_init(apac_ctx_t* apac_ctx) {

	apac_ctx->user_session  = (session_ctx_t*) apmalloc(sizeof(session_ctx_t));
	apac_ctx->echo_system   = (echo_ctx_t*)    apmalloc(sizeof(echo_ctx_t));

	apac_ctx->core_backend  = (backend_ctx_t*) apmalloc(sizeof(backend_ctx_t));
	apac_ctx->governor      = (schedgov_t*)    apmalloc(sizeof(schedgov_t));
	apac_ctx->root          = (storage_tree_t*)apmalloc(sizeof(storage_tree_t));
	apac_ctx->locker        = (lockerproc_t*)  apmalloc(sizeof(lockerproc_t));

	memset(apac_ctx->user_session, 0, sizeof(*apac_ctx->user_session));
	memset(apac_ctx->echo_system, 0, sizeof(*apac_ctx->echo_system));
	memset(apac_ctx->core_backend, 0, sizeof(*apac_ctx->core_backend));
	memset(apac_ctx->governor, 0, sizeof(*apac_ctx->governor));
	memset(apac_ctx->root, 0, sizeof(*apac_ctx->root));
	memset(apac_ctx->locker, 0, sizeof(*apac_ctx->locker));

	const i32 sanret = apac_san(apac_ctx);
	if (sanret != 0) {
		echo_error(apac_ctx, "Core object sanitization was failed!\n");
		return sanret;
	}

	return 0;
}

static i32 apac_deinit(apac_ctx_t* apac_ctx) {
	if (apac_ctx->user_session != NULL) apfree(apac_ctx->user_session);
	if (apac_ctx->echo_system != NULL)  apfree(apac_ctx->echo_system);

	if (apac_ctx->core_backend != NULL) apfree(apac_ctx->core_backend);
	if (apac_ctx->governor != NULL)     apfree(apac_ctx->governor);

	if (apac_ctx->root != NULL)         apfree(apac_ctx->root);
	if (apac_ctx->locker != NULL)       apfree(apac_ctx->locker);

	return 0;
}

i32 main(i32 argc, char** argv) {
	install_handlers(); 

	apac_ctx_t* apac_main = (apac_ctx_t*)apmalloc(sizeof(apac_ctx_t));

	if (apac_main == NULL) {
		echo_error(apac_main, "Can't allocate main context, "
			"this must be reported!\n");
	}

	const i32 aret = apac_init(apac_main);
	if (aret != 0) {
		echo_error(apac_main, "Can't initialize apac main context!\n");
		
		apfree(apac_main);
		return -1;
	}
	
	const i32 sched = sched_init(apac_main);
	if (sched != 0) {
		echo_error(apac_main, "Can't setup the main core thread name, "
			"the scheduler wasn't activated!\n");
		return sched;
	}

	const i32 sinit = session_init(argc, argv, apac_main);
	if (sinit != 0) {
		if (sinit == -1) {
			echo_error(apac_main, "Session starting was failed, "
				"killing the process...\n");
		}
		session_deinit(apac_main);
		goto going_out;
	}

	const i32 apac_ret = inner_apacentry(apac_main);

	session_deinit(apac_main);

going_out:
	sched_deinit(apac_main);
	apac_deinit(apac_main);

	apfree(apac_main);
	return apac_ret;
}

