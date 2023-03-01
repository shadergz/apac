#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <memctrlext.h>
#include <strings.h>
#include <user/cli.h>
#include <rt.h>
#include <layer.h>

#include <echo/fmt.h>

enum cli_arg_type {
	CLI_ARG_NONE,
	CLI_ARG_BOOLEAN,
	CLI_ARG_SWITCHER
};

enum cli_arg_prob {
	CLI_PROB_NONE,
	CLI_PROB_OPTIONAL = 0x100,
	CLI_PROB_REQUIRED = 0x200,
};

struct cli_arg {
	i32 type_flags;
	union {
		bool bvalue;
	};
};

struct cli_option {
	const char* option;
	char opt;
	struct cli_arg argument;
};

i32 user_cli_init(apac_ctx_t* apac_ctx) {
	user_options_t* user = apac_ctx->user_session->user_options;
	
	user->dsp_help = false;
	user->dsp_banner = true;

	user->enb_log_system = true;

	user->echo_level = 0;
	user->enb_colors = false;

	return 0;
}

static const char cli_bool_true[] = "true";
static const char cli_switcher_enb[] = "enable";

static bool cli_fmt_bool(const char* boovalue) {
	if (!boovalue) return false;
	
	return strncasecmp(boovalue, cli_bool_true, sizeof cli_bool_true) == 0;
}

static bool cli_fmt_switcher(const char* switvalue) {
	if (switvalue == NULL) return false;

	return strncasecmp(switvalue, cli_switcher_enb, sizeof cli_switcher_enb) == 0;
}

static void cli_clash(apac_ctx_t* apac_ctx, const char* fmt, ...) 
	__attribute__((noreturn));

static void cli_clash(apac_ctx_t* apac_ctx, const char* fmt, ...) {
	const session_ctx_t* session = apac_ctx->user_session;

	va_list args;
	va_start(args, fmt);

	if (session == NULL) run_raise(SIGINT);
	if (session->user_options == NULL) run_raise(SIGINT);
	
	char* cli_msg = NULL;
	
	layer_vasprintf(&cli_msg, fmt, args);

	va_end(args);

	if (cli_msg != NULL) {
		echo_error(apac_ctx, "CLI: %s", cli_msg);
		apfree(cli_msg);
	}
	run_raisef(SIGINT, "was clashed in CLI processing!");
}

static i32 cli_get(const char** prog_name, i32 argc, char* argv[], 
		struct cli_option* opts, const struct cli_option** save_arg,
		apac_ctx_t* apac_ctx)
{
	if (save_arg) *save_arg = NULL;
	if (argc <= 1 || argv[argc] != NULL) return -1;
	if (prog_name) *prog_name = argv[0];

	static i32 curr_aptr = 1;
	const char* curr_val = argv[curr_aptr++];
	
	if (!curr_val) return -1;
	if (*curr_val++ != '-') return -1; 

	for (; opts && opts->option; opts++) {
		const char* arg = curr_val;
		
		if (*curr_val == '-') {
			if (strncmp(++arg, opts->option, strlen(opts->option)) != 0) continue;
		} else if (*curr_val != '-') {
			if (*curr_val != opts->opt) continue;
			curr_val++;
			if (*curr_val != '=' && *curr_val != '\0') {
				/* This command line syntax parameter is invalid and must not
				 * be accepted! */
				cli_clash(apac_ctx, "invalid parameter name \"%s\" after "
	      				"\"%s\"\n", 
					curr_val-2, curr_aptr > 2 ? argv[curr_aptr - 2] 
					: "none");
			}
		}
		if (save_arg) *save_arg = opts;

		struct cli_arg* cli = &opts->argument;
		cli->bvalue = true;

		const char* value = strchr(arg, '=');
		if (value == NULL) return opts->opt;
		else value++;

		switch ((enum cli_arg_type)cli->type_flags & 0xff) {
		case CLI_ARG_NONE: 
			/* Invalid context value, should be an "assert" here! */ break;
		case CLI_ARG_BOOLEAN:
			cli->bvalue = cli_fmt_bool(value); break;
		case CLI_ARG_SWITCHER:
			cli->bvalue = cli_fmt_switcher(value); break;
		}

		return opts->opt;
	}

	cli_clash(apac_ctx, "command argument \'%s\' not found\n", argv[curr_aptr - 1]);
}

static struct cli_option s_default_cli_args[] = {
	#define USER_CLI_HELP     'h'
	#define USER_CLI_BANNER   'B'
	#define USER_CLI_WOUT_OPT '\0'

	[0] = {"help",       USER_CLI_HELP,     {CLI_ARG_BOOLEAN  | CLI_PROB_OPTIONAL}},
	[1] = {"banner",     USER_CLI_BANNER,   {CLI_ARG_BOOLEAN  | CLI_PROB_OPTIONAL}},
	[2] = {"log-system", USER_CLI_WOUT_OPT, {CLI_ARG_SWITCHER | CLI_PROB_OPTIONAL}},
	[3] = {}
};

i32 user_cli_parser(i32 argc, char* argv[], apac_ctx_t* apac_ctx) {
	user_options_t* user_conf = apac_ctx->user_session->user_options;

	i32 c;
	const struct cli_option* user_opt;
	
	while ((c = cli_get(NULL, argc, argv, s_default_cli_args, &user_opt, apac_ctx)) != -1) {
		const struct cli_arg* arg = &user_opt->argument;
		if (!arg) return -1;

		switch (c) {
		case USER_CLI_HELP: 	
			user_conf->dsp_help = arg->bvalue; break;
		case USER_CLI_BANNER:	
			user_conf->dsp_banner = arg->bvalue; break; 
		case USER_CLI_WOUT_OPT:
			if (strncmp(user_opt->option, "log-system", strlen(user_opt->option)) == 0)
				user_conf->enb_log_system = arg->bvalue;
		}
	}

	return 0;
}

i32 user_cli_san(const apac_ctx_t* apac_ctx) {
	return 0;
}

i32 user_cli_deinit(apac_ctx_t* apac_ctx) {
	return 0;
}

