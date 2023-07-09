#ifndef APAC_USER_CLI_H
#define APAC_USER_CLI_H

#include <api.h>

i32 user_cli_init(apac_ctx_t* apac_ctx);
i32 user_cli_parser(i32 argc, char* argv[], apac_ctx_t* apac_ctx);

i32 user_cli_san(const apac_ctx_t* apac_ctx);
i32 user_cli_deinit(apac_ctx_t* apac_ctx);

i32 config_set(const char* option, const char* value, apac_ctx_t* apac_ctx);

#endif
