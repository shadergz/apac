#include <conf.h>

i32
conf_init (apac_ctx_t *apac_ctx)
{
  session_ctx_t *session = apac_ctx->user_session;
  config_user_t *configs = session->user_config;

  if (!configs)
    return -1;
  configs->default_input = "package_in.apk, package_in.ipa";
  configs->default_output = "package_out.apk, package_out.ipa";

  configs->exec_script = NULL;
  configs->structure_model = "APKTOOL";

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
