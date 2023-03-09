#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <memctrlext.h>
#include <strings.h>
#include <user/cli.h>

#include <user/cli_clash.h>
#include <user/line_format.h>

#include <echo/fmt.h>

enum cli_arg_type
{
  CLI_ARG_NONE,
  CLI_ARG_BOOLEAN,
  CLI_ARG_SWITCHER,

  CLI_ARG_STRING
};

enum cli_arg_prob
{
  CLI_PROB_NONE,
  CLI_PROB_OPTIONAL = 0x100,
  CLI_PROB_REQUIRED = 0x200,
};

struct cli_option
{
  const char *option;
  char opt;

  const enum cli_arg_prob opt_probs;

  const enum cli_arg_type opt_flags;
};

i32
user_cli_init (apac_ctx_t *apac_ctx)
{
  session_ctx_t *session = apac_ctx->user_session;
  user_options_t *user = session->user_options;
  config_user_t *conf = session->user_config;

  user->dsp_help = false;
  user->dsp_banner = true;

  user->enb_log_system = true;

  user->echo_level = 0;
  user->enb_colors = false;

  user->in_list = conf->default_input;
  user->out_list = conf->default_output;

  return 0;
}

static bool g_boolean;
static const char *g_optvalue;
static const char *g_option;
static i32
cli_get (const char **prog_name, i32 argc, char *argv[],
         const struct cli_option *opts, apac_ctx_t *apac_ctx)
{
  if (argc <= 1 || argv[argc] != NULL)
    return -1;
  if (prog_name)
    *prog_name = argv[0];

  static i32 curr_aptr = 1;
  const char *curr_val = argv[curr_aptr++];

  if (!curr_val)
    return -1;
  if (*curr_val++ != '-')
    return -1;

  for (; opts && opts->option; opts++)
    {
      const char *arg = curr_val;

      if (*curr_val == '-')
        {
          if (strncmp (++arg, opts->option, strlen (opts->option)) != 0)
            continue;
        }
      else if (*curr_val != '-')
        {
          if (*curr_val != opts->opt)
            continue;
          curr_val++;
          if (*curr_val == '=' || *curr_val == '\0')
            goto clisolver;
          /* This command line syntax parameter is invalid and must not
           * be accepted! */
          cli_clash (apac_ctx, "Invalid parameter name \'%s\' after \'%s\'\n",
                     curr_val - 2,
                     curr_aptr > 2 ? argv[curr_aptr - 2] : "$none$");
        }
    clisolver:
      g_option = arg;
      const char *value = strchr (arg, '=');
      if (value == NULL)
        {
          if (opts->opt_probs & CLI_PROB_NONE)
            return opts->opt;
          if (opts->opt_probs & CLI_PROB_REQUIRED)
            {
              cli_clash (apac_ctx, "The option \'%s\', need an argument", arg);
            }
          echo_assert (apac_ctx, opts->opt_probs & CLI_PROB_OPTIONAL,
                       "Wtf probes are you using?!\n");
          g_boolean = true;
          g_optvalue = "true";

          return opts->opt;
        }
      else
        value++;

      g_optvalue = NULL;
      g_boolean = false;

      switch (opts->opt_flags)
        {
        case CLI_ARG_NONE:
          echo_assert (apac_ctx, 0 != 0, "Invalid argument context\n");
          break;
        case CLI_ARG_BOOLEAN:
          {
            g_boolean = cli_fmt_bool (value);
            break;
          }
        case CLI_ARG_SWITCHER:
          {
            g_boolean = cli_fmt_switcher (value);
            break;
          }
        case CLI_ARG_STRING:
          {
            g_optvalue = strdup (value);
            break;
          }
        }

      return opts->opt;
    }

  cli_clash (apac_ctx, "Command argument \'%s\' not found\n",
             argv[curr_aptr - 1]);
  return -1;
}

static const struct cli_option g_default_cli_args[] = {
#define USER_CLI_HELP 'h'
#define USER_CLI_BANNER 'B'
#define USER_CLI_IN_LIST 'I'
#define USER_CLI_OUT_LIST 'O'
#define USER_CLI_WOUT_OPT '\0'

  { "help", USER_CLI_HELP, CLI_PROB_OPTIONAL, CLI_ARG_BOOLEAN },
  { "banner", USER_CLI_BANNER, CLI_PROB_OPTIONAL, CLI_ARG_BOOLEAN },
  { "log-system", USER_CLI_WOUT_OPT, CLI_PROB_OPTIONAL, CLI_ARG_SWITCHER },
  { "in", USER_CLI_IN_LIST, CLI_PROB_REQUIRED, CLI_ARG_STRING },
  { "out", USER_CLI_OUT_LIST, CLI_PROB_REQUIRED, CLI_ARG_STRING },

  {}
};

i32
user_cli_parser (i32 argc, char *argv[], apac_ctx_t *apac_ctx)
{
  user_options_t *user_conf = apac_ctx->user_session->user_options;

  i32 c;

  while ((c = cli_get (NULL, argc, argv, g_default_cli_args, apac_ctx)) != -1)
    {
      switch (c)
        {
        case USER_CLI_HELP:
          user_conf->dsp_help = g_boolean;
          break;
        case USER_CLI_BANNER:
          user_conf->dsp_banner = g_boolean;
          break;
        case USER_CLI_IN_LIST:
          user_conf->in_list = g_optvalue;
          break;
        case USER_CLI_OUT_LIST:
          user_conf->out_list = g_optvalue;
          break;

        case USER_CLI_WOUT_OPT:
          if (strncmp (g_option, "log-system", strlen (g_option)) == 0)
            user_conf->enb_log_system = g_boolean;
        }
    }

  return 0;
}

i32
user_cli_san (const apac_ctx_t *apac_ctx)
{
  return 0;
}

i32
user_cli_deinit (apac_ctx_t *apac_ctx)
{
  user_options_t *user_conf = apac_ctx->user_session->user_options;
  const config_user_t *conf = apac_ctx->user_session->user_config;

#define OPTION_RM_DUP(user, default, field)                                   \
  if (user->field != default)                                                 \
  apfree ((char *)user->field)

  OPTION_RM_DUP (user_conf, conf->default_input, in_list);
  OPTION_RM_DUP (user_conf, conf->default_output, out_list);

  memset (apac_ctx->user_session->user_options, 0, sizeof (user_options_t));

  return 0;
}
