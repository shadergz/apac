
#include <regex.h>
#include <stdlib.h>
#include <string.h>

#include <default.h>
#include <memctrlext.h>

#include <user/cli.h>

#include <echo/fmt.h>

#define NOTIN_RANGE(setname, value, min, max, jumpto)                         \
  do                                                                          \
    {                                                                         \
      if (value < min || value > max)                                         \
        {                                                                     \
          echo_error (NULL,                                                   \
                      "Can sets %s new value, because %u is outside the "     \
                      "range!\n\tMin value: %u; Max value: %u\n",             \
                      setname, value, min, max);                              \
          goto jumpto;                                                        \
        }                                                                     \
    }                                                                         \
  while (0)

#define SEARCH_FIELD(table, field, user_table, user_field)                    \
  if (strncasecmp (table, user_table, strlen (table)) == 0                    \
      && strncasecmp (field, user_field, strlen (field)) == 0)

static i32
config_execute (char *set, config_user_t *cuser)
{
  char *tokyo[1];
  i32 ret = -1;

  char *table = strtok_r (set, ":", &tokyo[0]);
  if (!table)
    return ret;

  const char *set_table = strdup (table);

  char *field = strtok_r (NULL, ":", &tokyo[0]);
  if (!field)
    goto cleansets;

  const char *set_field = strdup (field);
  const char *svalue = strrchr (field, '=');
  if (svalue)
    svalue++;

  if (!svalue)
    goto cleansets;

  SEARCH_FIELD ("main", "max_thread", set_table, set_field)
  {
    const u8 thread_cnt = (u8)strtoul (svalue, NULL, 0);
    NOTIN_RANGE (set_field, thread_cnt, CONFIG_DEFAULT_MAX_THREAD_MIN,
                 CONFIG_DEFAULT_MAX_THREAD_MAX, cleansets);

    cuser->max_thread = thread_cnt;
  }
  ret = 0;
cleansets:
  apfree ((char *)set_table);
  apfree ((char *)set_field);

  return ret;
}

i32
config_set (const char *option, const char *value, apac_ctx_t *apac_ctx)
{
  char *bak = NULL, *tok = NULL;
  char *setbackup = strdup (value);
  i32 ret = -1;

  session_ctx_t *us = apac_ctx->user_session;

  config_user_t *cuser = us->user_config;

  tok = strtok_r (setbackup, ",;", &bak);
  for (; tok;)
    {
      const i32 exe = regexec (cuser->confsetexpr, tok, 0, NULL, 0);
      if (exe == REG_NOMATCH)
        {
          echo_error (apac_ctx, "Config set `%s` is wrong formatted\n", tok);
          return -1;
        }

      const i32 eset = config_execute (tok, cuser);
      if (eset != 0)
        goto giveup;
      tok = strtok_r (NULL, ",;", &bak);
    }

  ret = 0;

giveup:
  apfree (setbackup);
  apfree ((char *)value);

  return ret;
}