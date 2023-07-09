#include <stdarg.h>
#include <stdio.h>

#include <layer.h>
#include <memctrlext.h>
#include <rt.h>

#include <user/cli_clash.h>

#include <echo/fmt.h>

void cli_clash(apac_ctx_t* apac_ctx, const char* fmt, ...)
{
    const session_ctx_t* session = apac_ctx->user_session;

    va_list args;
    va_start(args, fmt);

    if (session == NULL)
        run_raise(SIGINT);
    if (session->user_options == NULL)
        run_raise(SIGINT);

    char* cli_msg = NULL;

    layer_vasprintf(&cli_msg, fmt, args);

    va_end(args);

    if (cli_msg != NULL) {
        echo_error(apac_ctx, "CLI: %s", cli_msg);
        apfree(cli_msg);
    }
    run_raisef(SIGINT, "Was clashed in CLI processing!");
}
