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

#include <pool/gov.h>

static i32 main_msg(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

static i32 main_msg(const char* fmt, ...) {
	va_list fmt_va;
	va_start(fmt_va, fmt);
	
	const i32 fmt_out = vfprintf(stderr, fmt, fmt_va);

	va_end(fmt_va);

	return fmt_out;
}

static i32 dsp_help(apac_ctx_t* apac_ctx) {

	const user_options_t* user = apac_ctx->user_options;
	const session_ctx_t* session = apac_ctx->user_session;

	if (user->dsp_help == false) return -1;

	const i32 result = session->printf_here("%s\n", g_embed_help_msg);

	return result;
}

static const char s_apac_version[] = "0.1.3";
static const char s_apac_rev[] = "0b";

static i32 dsp_banner(apac_ctx_t* apac_ctx) {
	const user_options_t* user   = apac_ctx->user_options;
	const session_ctx_t* session = apac_ctx->user_session;
	if (user->dsp_banner == false) return -1;

	const i32 print_res = session->printf_here("apac (version %s) rev.%s Copyright (c) 2023 the "
		"apac's developers\n", s_apac_version, s_apac_rev);

	return print_res;
}

static i32 enable_logsystem(apac_ctx_t* apac_ctx) {
	const user_options_t* user   = apac_ctx->user_options;
	const session_ctx_t* session = apac_ctx->user_session;

	if (user->enb_log_system == false) {
		session->printf_here("`echo` log system was been disabled for this session!\n");
		return -1;
	}

	const i32 ret = echo_init(apac_ctx);
	return ret;
}

static i32 session_cli(i32 argc, char* argv[], apac_ctx_t* apac_ctx) {
	const i32 user_init = user_cli_init(apac_ctx);
	if (user_init != 0) 
		return user_init;

	const i32 user_san = user_cli_san(apac_ctx);
	if (user_san != 0) 
		return user_san;

	const i32 user_parser = user_cli_parser(argc, argv, apac_ctx);
	if (user_parser != 0) 
		return user_parser;

	enable_logsystem(apac_ctx);
	dsp_banner(apac_ctx);
	dsp_help(apac_ctx);

	return 0;
}

static i32 session_cpu_controller(apac_ctx_t* apac_ctx) {
	if (apac_ctx == NULL) 
		return -1;

	const i32 sched = sched_init(apac_ctx);
	if (sched != 0) 
		return sched;

	return 0;
}

i32 session_makestorage(apac_ctx_t* apac_ctx) {
	char* exec_dir = NULL;
	run_getedir(&exec_dir, DIRIO_MAXPATH_SZ);
	// Now the directory is under control by our filesystem handler
	const i32 tree_ret = tree_makeroot(".", apac_ctx);
	// exec_dir ins't more needed
	apfree(exec_dir);

	return tree_ret;
}

i32 session_backend(apac_ctx_t* apac_ctx) {
	back_init(apac_ctx);

	return 0;
}

i32 session_lock(apac_ctx_t* apac_ctx) {
	locker_init(apac_ctx);
	locker_acquire(apac_ctx);

	return 0;
}

i32 session_unlock(apac_ctx_t* apac_ctx) {
	locker_release(apac_ctx);
	locker_deinit(apac_ctx);
	
	return 0;
}

i32 session_init(i32 argc, char* argv[], apac_ctx_t* apac_ctx) {

	session_ctx_t* session = apac_ctx->user_session;
	session->printf_here = main_msg;

	session_cpu_controller(apac_ctx);
	session_cli(argc, argv, apac_ctx);

	session_makestorage(apac_ctx);
	session_backend(apac_ctx);

	session_lock(apac_ctx);	

	return 0;
}

i32 session_deinit(apac_ctx_t* apac_ctx) {
	back_deinit(apac_ctx);

	const user_options_t* user_opts = apac_ctx->user_options;
	
	if (user_opts->enb_log_system == true) {
		echo_deinit(apac_ctx);
	}

	user_cli_deinit(apac_ctx);
	sched_deinit(apac_ctx);

	session_unlock(apac_ctx);

	tree_close(apac_ctx->root, true);	
	apfree(apac_ctx->root);
	apac_ctx->root = NULL;

	return 0;
}

