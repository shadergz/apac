
#include <stdio.h>
#include <stdarg.h>

#include <session.h>
#include <user/cli.h>
#include <embed/user.h>

#include <rt.h>
#include <locker.h>
#include <memctrlext.h>
#include <storage/dirio.h>
#include <storage/tree.h>
#include <echo/core.h>

#include <backend_space.h>
#include <echo/fmt.h>

#include <pool/gov.h>

static i32 dsp_help(apac_ctx_t* apac_ctx) {

	const session_ctx_t* session = apac_ctx->user_session;
	const user_options_t* user = session->user_options; 

	if (user->dsp_help == false) return -1;

	const i32 result = echo_success(apac_ctx, "%s\n", g_embed_help_msg);

	return result;
}

static const char s_apac_version[] = "0.1.4";
static const char s_apac_rev[] = "a1";

static i32 dsp_banner(apac_ctx_t* apac_ctx) {
	const session_ctx_t* session = apac_ctx->user_session;
	const user_options_t* user   = session->user_options;
	
	if (user->dsp_banner == false) return -1;

	const i32 print_res = echo_success(apac_ctx, "apac (version %s) rev.%s Copyright (C) 2023 the "
		"Apac's developers\n", s_apac_version, s_apac_rev);

	return print_res;
}

static i32 enable_logsystem(apac_ctx_t* apac_ctx) {
	const session_ctx_t* session = apac_ctx->user_session;
	const user_options_t* user   = session->user_options;
	
	if (user->enb_log_system == false) {
		echo_success(apac_ctx, "`echo` log system was been disabled for this session!\n");
		return -1;
	}

	const i32 ret = echo_init(apac_ctx);
	if (ret != 0) {
		echo_error(apac_ctx, "Can't starts *echo* log system, this is dangerous!\n");
		return ret;
	}

	return ret;
}

static i32 session_cli(i32 argc, char* argv[], apac_ctx_t* apac_ctx) {
	const i32 user_init = user_cli_init(apac_ctx);
	
	if (user_init != 0) {
		echo_error(apac_ctx, "User CLI options wasn't be initialized\n");
		return user_init;
	}

	const i32 user_san = user_cli_san(apac_ctx);
	if (user_san != 0) return user_san;

	const i32 user_parser = user_cli_parser(argc, argv, apac_ctx);
	if (user_parser != 0) {
		echo_error(apac_ctx, "Can't parser the user options, going out!\n");
		return user_parser;
	}

	enable_logsystem(apac_ctx);
	dsp_banner(apac_ctx);
	dsp_help(apac_ctx);

	return 0;
}

static i32 session_cpu_controller(apac_ctx_t* apac_ctx) {
	if (apac_ctx == NULL) return -1;

	const i32 sched = sched_init(apac_ctx);
	if (sched != 0) {
		echo_error(apac_ctx, "Can't setups the process main thread name, "
			"The scheduler was dieded!\n");
		return sched;

	}

	return 0;
}

i32 session_makestorage(apac_ctx_t* apac_ctx) {
	char* exec_dir = NULL;
	run_getedir(&exec_dir, DIRIO_MAXPATH_SZ);

	echo_info(apac_ctx, "*apac* begin executed inside of %s\n", exec_dir);

	const i32 tree_ret = tree_makeroot(".", apac_ctx);
	// Now the directory is under control by our storage handler
	if (tree_ret != 0)
		echo_error(apac_ctx, "Can't setup the tree's root storage node\n");

	// exec_dir ins't more needed
	apfree(exec_dir);
	return tree_ret;
}

i32 session_backend(apac_ctx_t* apac_ctx) {
	const i32 back = back_init(apac_ctx);
	if (back != 0)
		echo_error(apac_ctx, "Backend components wasn't be initialized\n");

	return back;
}

i32 session_lock(apac_ctx_t* apac_ctx) {
	i32 lret = locker_init(apac_ctx);
	lret = locker_acquire(apac_ctx);

	if (lret != 0)
		echo_error(apac_ctx, "Can't acquire the locker file, is't real problem\n");

	return lret;
}

i32 session_unlock(apac_ctx_t* apac_ctx) {
	i32 rele = locker_release(apac_ctx);
	rele = locker_deinit(apac_ctx);
	
	if (rele != 0)
		echo_error(apac_ctx, "Problems when releasing the locker\n");

	return rele;
}

i32 session_init(i32 argc, char* argv[], apac_ctx_t* apac_ctx) {
	session_ctx_t* session = apac_ctx->user_session;
	session->user_options = (user_options_t*)apmalloc(sizeof(user_options_t));
	if (session->user_options == NULL) {
		echo_error(NULL, "Can't allocate the user options\n");
		return -1;
	}

	if (session_cli(argc, argv, apac_ctx) != 0) {
		echo_error(apac_ctx, "Session was failed to initialize the "
			"CLI interface\n");
		goto ss_failed;
	}

	if (session_cpu_controller(apac_ctx) != 0) {
		echo_error(apac_ctx, "CPU controller was failed to initialize\n");
		goto cpuc_failed;
	}

	if (session_makestorage(apac_ctx) != 0) {
		echo_error(apac_ctx, "Storage wans't be initialized correct\n");
		goto storage_failed;
	}


	if (session_backend(apac_ctx) != 0) {
		echo_error(apac_ctx, "Backend wasn't not be started\n");
		goto back_failed;
	}

	if (session_lock(apac_ctx) != 0) {
		echo_error(apac_ctx, "Lock locking stage was failed\n");
		goto lock_failed;
	}

	echo_info(apac_ctx, "Core session was initialized with all components\n");

	return 0;

	lock_failed:
	session_unlock(apac_ctx);

	back_failed:
	tree_close(apac_ctx->root, true);

	storage_failed:
	sched_deinit(apac_ctx);

	cpuc_failed:
	user_cli_deinit(apac_ctx);

	ss_failed:
	return -1;
}

i32 session_deinit(apac_ctx_t* apac_ctx) {
	back_deinit(apac_ctx);

	session_ctx_t* session_user = apac_ctx->user_session;
	const user_options_t* user_opts = session_user->user_options;
	
	if (user_opts->enb_log_system == true) {
		echo_deinit(apac_ctx);
	}

	sched_deinit(apac_ctx);

	session_unlock(apac_ctx);

	user_cli_deinit(apac_ctx);

	tree_close(apac_ctx->root, true);	
	
	apfree(session_user->user_options);
	session_user->user_options = NULL;

	return 0;
}

