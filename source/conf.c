#include <string.h>

#include <conf.h>
#include <default.h>
#include <regex.h>

#include <echo/fmt.h>
#include <memctrlext.h>

i32 conf_init(apac_ctx_t* apac_ctx)
{
    session_ctx_t* session = apac_ctx->user_session;
    config_user_t* configs = session->user_config;

    if (!configs)
        return -1;
    configs->default_input = "package_in.apk, package_in.ipa";
    configs->default_output = "package_out.apk, package_out.ipa";

    configs->exec_script = "decode.asc";
    configs->structure_model = "APKTOOL";
    configs->user_max_cpu = false;
    configs->max_thread = CONFIG_DEFAULT_MAX_THREAD_MIN;

    configs->confsetexpr = (regex_t*)apmalloc(sizeof(regex_t));

    const i32 regc
        = regcomp(configs->confsetexpr,
            "^[[:alpha:]_]+:[[:alpha:]_]+=[[:alnum:]_]+$", REG_EXTENDED);
    if (regc != 0) {
        echo_error(apac_ctx, "Can't compile the regex for the config set\n");
        apfree(configs->confsetexpr);
        configs->confsetexpr = NULL;

        return -1;
    }

    return 0;
}

i32 conf_load(apac_ctx_t* apac_ctx)
{
    return 0;
}

i32 conf_save(apac_ctx_t* apac_ctx)
{
    return 0;
}

i32 conf_deinit(apac_ctx_t* apac_ctx)
{
    session_ctx_t* session = apac_ctx->user_session;
    config_user_t* configs = session->user_config;

    if (configs->confsetexpr) {
        regfree(configs->confsetexpr);
        apfree(configs->confsetexpr);
    }
    memset(configs, 0, sizeof *configs);

    return 0;
}
