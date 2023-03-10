#include <string.h>

#include <user/selector.h>

#include <echo/fmt.h>
#include <memctrlext.h>
#include <strhandler.h>

#include <doubly_int.h>

i32
select_move (apac_ctx_t *apac_ctx)
{
  session_ctx_t *current = apac_ctx->user_session;
  rule_selector_t *nsel = apmalloc (sizeof (rule_selector_t));
  memset (nsel, 0, sizeof *nsel);

  if (!nsel)
    return -1;

  doubly_insert (nsel, current->selectors);

  select_change ("select", doubly_size (current->selectors), apac_ctx);

  return 0;
}
#define OPTION_RM_DUP(selector, field, default)                               \
  if (selector->field != default)                                             \
  apfree ((char *)selector->field)

i32
select_disc (rule_selector_t *dsel, apac_ctx_t *apac_ctx)
{
  session_ctx_t *us = apac_ctx->user_session;
  const config_user_t *conf = us->user_config;

  OPTION_RM_DUP (dsel, rule_pkglist, conf->default_input);
  OPTION_RM_DUP (dsel, rule_outdirs, conf->default_output);
  OPTION_RM_DUP (dsel, structure_model, conf->structure_model);
  OPTION_RM_DUP (dsel, run_script, conf->exec_script);

  apfree (dsel->rule_settings);
  dsel->rule_settings = NULL;

  doubly_rm (dsel, us->selectors);
  apfree (dsel);

  return 0;
}

i32
select_change (const char *opt, u64 user_value, apac_ctx_t *apac_ctx)
{
  u64 select_id = 0;
  const char *select_rule = NULL;

  session_ctx_t *us = apac_ctx->user_session;

  if ((strncmp (opt, "select", strlen ("select"))) == 0)
    select_id = user_value;
  else if ((strncmp (opt, "select-by-name", strlen ("select-by-name"))) == 0)
    select_rule = (const char *)user_value;

  bool changed = false;
  if (select_rule)
    {

      for (rule_selector_t *rule = NULL;
           (rule = (rule_selector_t *)doubly_next (us->selectors));)
        {
          if (strstr (rule->rule_pkglist, (const char *)user_value) != 0)
            {
              changed = true;
              break;
            }
        }
    }
  else
    {
      rule_selector_t *pkg_rule
          = (rule_selector_t *)doubly_get (select_id, us->selectors);
      if (pkg_rule != NULL)
        changed = true;
    }

  apfree ((char *)select_rule);

  if (changed == false)
    {
      echo_error (
          apac_ctx,
          "Can't change selector context, invalid input package(s) name/id\n");
    }
  return !changed;
}

const char *
validate_execute (const char *cmd, const char **invalid)
{
  static const char *invalid_cmd[]
      = { "cmd", "..", "sudo", "doas", "su", "mv", NULL };

  const char *instr = strhandler_search (cmd, invalid_cmd);

  if (instr)
    {
      *invalid = instr;
      return NULL;
    }

  return cmd;
}

i32
select_treat (const char *opt, const void *optv, apac_ctx_t *apac_ctx)
{
  session_ctx_t *us = apac_ctx->user_session;
  rule_selector_t *curr_sel = doubly_curr (us->selectors);
  config_user_t *conf = us->user_config;
  pkg_settings_t *pkgtodo = curr_sel->rule_settings;

  i32 ev = 0;
  const char *opts = (const char *)optv;

  if (strncmp (opt, "execute", strlen ("execute")) == 0)
    {

      const char *invalid_cmd = NULL;
      pkgtodo->execute_prompt = validate_execute (opts, &invalid_cmd);
      if (!pkgtodo->execute_prompt)
        {
          echo_error (apac_ctx,
                      "After-run execute command %s, cmd: "
                      "\'%s\' is invalid\n",
                      opts, invalid_cmd);
          ev = -1;
        }
    }
  else if (strncmp (opt, "in", strlen ("in")) == 0)
    {
      if (curr_sel->rule_pkglist
          && curr_sel->rule_pkglist != conf->default_input)
        {
          echo_error (
              apac_ctx,
              "You can't set the input twices inside the same selector\n");
          ev = -1;
        }
      else
        {
          curr_sel->rule_pkglist = opts;
        }
    }
  else if (strncmp (opt, "out", strlen ("out")) == 0)
    {
      if (curr_sel->rule_outdirs
          && curr_sel->rule_outdirs != conf->default_output)
        {
          echo_error (apac_ctx,
                      "You can't modify the output dirs twice, use a "
                      "different selector configuration\n");
          ev = -1;
        }
      else
        {
          curr_sel->rule_outdirs = opts;
        }
    }

  if (ev != 0)
    apfree ((char *)opts);
  return ev;
}
