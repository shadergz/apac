#include <conf.h>

i32
conf_init (apac_ctx_t *apac_ctx)
{
  session_ctx_t *session = apac_ctx->user_session;
  config_user_t *configuration = session->user_config;

  if (!configuration)
    return -1;
  configuration->default_input = "package_in.apk, package_in.ipa";
  configuration->default_output = "package_out.apk, package_out.ipa";

  return 0;
}

i32
conf_load (apac_ctx_t *apac_ctx)
{
  return 0;
}

i32
conf_save (apac_ctx_t *apac_ctx)
{
  return 0;
}

i32
conf_deinit (apac_ctx_t *apac_ctx)
{
  return 0;
}
