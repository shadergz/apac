#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <doubly_int.h>
#include <memctrlext.h>

#include <strings.h>
#include <user/cli.h>

#include <user/cli_clash.h>
#include <user/line_format.h>

#include <user/selector.h>

#include <echo/fmt.h>

enum cli_arg_type
{
  CLI_ARG_NONE,
  CLI_ARG_BOOLEAN,
  CLI_ARG_SWITCHER,

  CLI_ARG_STRING,
  CLI_ARG_INTEGER
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

  const i32 sret = select_move (apac_ctx);
  if (sret != 0)
    {
      echo_error (apac_ctx,
                  "Can't create the default selector, this may be reported\n");
      return -1;
    }

  rule_selector_t *drule = (rule_selector_t *)doubly_curr (session->selectors);
  drule->rule_pkglist = conf->default_input;
  drule->rule_outdirs = conf->default_output;
  drule->structure_model = conf->structure_model;
  drule->run_script = conf->exec_script;

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

  for (; opts->option; opts++)
    {
      const char *arg = curr_val;
      char *value = strrchr (arg, '=');
      if (strchr (arg, '-') != NULL)
        {
          arg++;
          u64 optlen = strlen (opts->option);
          if (strncmp (arg, opts->option, optlen) != 0)
            continue;
          if (*(arg + optlen) != '=' && *(arg + optlen))
            continue;
        }
      else
        {
          if (*arg != opts->opt)
            continue;
          arg++;
          if ((*arg == '=' || *arg == '\0'))
            goto clisolver;
          /* This command line syntax parameter is invalid and must not
           * be accepted! */
          cli_clash (
              apac_ctx, "Parameter name \'%s\' is invalid, after \'%s\'\n",
              curr_val - 2, curr_aptr > 2 ? argv[curr_aptr - 2] : argv[0]);
        }
    clisolver:
      g_option = arg;
      if (value == NULL)
        {
          if (opts->opt_probs == CLI_PROB_NONE)
            return opts->opt;
          if (opts->opt_probs & CLI_PROB_REQUIRED)
            {
              cli_clash (apac_ctx, "The option \'%s\', needs an argument\n",
                         g_option);
            }
          echo_assert (apac_ctx,
                       opts->opt_probs & CLI_PROB_OPTIONAL
                           || opts->opt_probs == CLI_PROB_NONE,
                       "Wtf probes are you using?!\n");
          g_boolean = true;
          g_optvalue = "true";

          return opts->opt;
        }
      else
        {
          *value = '\0';
          value++;
        }

      g_optvalue = NULL;
      g_boolean = false;

      switch (opts->opt_flags)
        {
        case CLI_ARG_NONE:
          echo_assert (
              apac_ctx, CLI_ARG_NONE,
              "Flags isn't setted correctly, this is an invalid context!");
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
        case CLI_ARG_INTEGER:
          {
            g_optvalue = (void *)strtoul (value, NULL, 0);
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
#define USER_CLI_SELECT 's'
#define USER_CLI_WOUT_OPT '\0'

  { "help", USER_CLI_HELP, CLI_PROB_OPTIONAL, CLI_ARG_BOOLEAN },
  { "banner", USER_CLI_BANNER, CLI_PROB_OPTIONAL, CLI_ARG_BOOLEAN },
  { "log-system", USER_CLI_WOUT_OPT, CLI_PROB_OPTIONAL, CLI_ARG_SWITCHER },
  { "in", USER_CLI_IN_LIST, CLI_PROB_REQUIRED, CLI_ARG_STRING },
  { "out", USER_CLI_OUT_LIST, CLI_PROB_REQUIRED, CLI_ARG_STRING },
  { "execute", USER_CLI_WOUT_OPT, CLI_PROB_REQUIRED, CLI_ARG_STRING },
  { "select-move", USER_CLI_WOUT_OPT, CLI_PROB_NONE, CLI_ARG_NONE },
  { "select-by-name", USER_CLI_WOUT_OPT, CLI_PROB_REQUIRED, CLI_ARG_STRING },
  { "select", USER_CLI_SELECT, CLI_PROB_REQUIRED, CLI_ARG_INTEGER },

  {}
};

i32
user_cli_parser (i32 argc, char *argv[], apac_ctx_t *apac_ctx)
{
  session_ctx_t *us = apac_ctx->user_session;
  user_options_t *user_conf = us->user_options;

  i32 c, rstop = 0;

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
        case USER_CLI_OUT_LIST:
          rstop = select_treat (g_option, g_optvalue, apac_ctx);
          break;
        case USER_CLI_SELECT:
          rstop = select_change (g_option, (u64)g_optvalue, apac_ctx);
          break;

        case USER_CLI_WOUT_OPT:
          if (strncmp (g_option, "log-system", strlen (g_option)) == 0)
            user_conf->enb_log_system = g_boolean;
          else if (strncmp (g_option, "select-move", strlen (g_option)) == 0)
            rstop = select_move (apac_ctx);
          else if (strncmp (g_option, "select-by-name", strlen (g_option))
                   == 0)
            rstop = select_change (g_option, (u64)g_optvalue, apac_ctx);
          else if (strncmp (g_option, "execute", strlen (g_option)) == 0)
            rstop = select_treat (g_option, g_optvalue, apac_ctx);
        }
      if (rstop != 0)
        break;
    }

  doubly_reset (us->selectors);

  return rstop;
}

i32
user_cli_san (const apac_ctx_t *apac_ctx)
{
  return 0;
}

i32
user_cli_deinit (apac_ctx_t *apac_ctx)
{
  session_ctx_t *us = apac_ctx->user_session;

  doubly_reset (us->selectors);
  for (rule_selector_t *pkgr;
       (pkgr = (rule_selector_t *)doubly_next (us->selectors));)
    {
      select_disc (pkgr, apac_ctx);
    }
  doubly_reset (us->selectors);

  memset (us->user_options, 0, sizeof (user_options_t));

  return 0;
}
