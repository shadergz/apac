#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <strings.h>
#include <memctrlext.h>
#include <user/cli.h>

#include <user/line_format.h>
#include <user/cli_clash.h>

#include <echo/fmt.h>

enum cli_arg_type {
	CLI_ARG_NONE,
	CLI_ARG_BOOLEAN,
	CLI_ARG_SWITCHER,

	CLI_ARG_STRLIST
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
		const char* svalue;
	};
};

struct cli_option {
	const char* option;
	char opt;

	struct cli_arg argument;
	bool was_setted;
};

i32 user_cli_init(apac_ctx_t* apac_ctx) {
	session_ctx_t* session = apac_ctx->user_session;
	user_options_t* user = session->user_options;
	config_user_t* conf = session->user_config;

	user->dsp_help = false;
	user->dsp_banner = true;

	user->enb_log_system = true;

	user->echo_level = 0;
	user->enb_colors = false;

	user->in_list  = conf->default_input;
	user->out_list = conf->default_output;

	return 0;
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
		case CLI_ARG_BOOLEAN:  cli->bvalue = cli_fmt_bool(value); break;
		case CLI_ARG_SWITCHER: cli->bvalue = cli_fmt_switcher(value); break;
		case CLI_ARG_STRLIST:  cli->svalue = strdup(value); break;
		}
		opts->was_setted = true;

		return opts->opt;
	}

	cli_clash(apac_ctx, "command argument \'%s\' not found\n", argv[curr_aptr - 1]);
}

static struct cli_option s_default_cli_args[] = {
	#define USER_CLI_HELP     'h'
	#define USER_CLI_BANNER   'B'
	#define USER_CLI_IN_LIST  'I'
	#define USER_CLI_OUT_LIST 'O'
	#define USER_CLI_WOUT_OPT '\0'

	{"help",       USER_CLI_HELP,     {CLI_ARG_BOOLEAN  | CLI_PROB_OPTIONAL}},
	{"banner",     USER_CLI_BANNER,   {CLI_ARG_BOOLEAN  | CLI_PROB_OPTIONAL}},
	{"log-system", USER_CLI_WOUT_OPT, {CLI_ARG_SWITCHER | CLI_PROB_OPTIONAL}},
	{"in",         USER_CLI_IN_LIST,  {CLI_ARG_STRLIST  | CLI_PROB_REQUIRED}},
	{"out",        USER_CLI_OUT_LIST, {CLI_ARG_STRLIST  | CLI_PROB_REQUIRED}},

	{}
};

i32 user_cli_parser(i32 argc, char* argv[], apac_ctx_t* apac_ctx) {
	user_options_t* user_conf = apac_ctx->user_session->user_options;

	i32 c;
	const struct cli_option* user_opt;
	
	while ((c = cli_get(NULL, argc, argv, s_default_cli_args, &user_opt, apac_ctx)) != -1) {
		const struct cli_arg* arg = &user_opt->argument;
		if (!arg) return -1;

		switch (c) {
		case USER_CLI_HELP:     user_conf->dsp_help = arg->bvalue; break;
		case USER_CLI_BANNER:   user_conf->dsp_banner = arg->bvalue; break; 
		case USER_CLI_IN_LIST:  user_conf->in_list = arg->svalue; break;
		case USER_CLI_OUT_LIST: user_conf->out_list = arg->svalue; break;

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

	struct cli_option* ptr = s_default_cli_args;

	#define IF_OPTION_HAS_DUP(opt)\
		if ((opt->argument.type_flags & CLI_ARG_STRLIST) && \
			opt->was_setted) 

	for (; ptr->option != NULL; ptr++) {
		IF_OPTION_HAS_DUP(ptr){
			apfree((char*)ptr->argument.svalue);
		}
	}

	memset(apac_ctx->user_session->user_options, 0, sizeof(user_options_t));

	return 0;
}

