#ifndef APAC_SESSION_H
#define APAC_SESSION_H

#include <api.h>

i32 session_init (i32 argc, char *argv[], apac_ctx_t *apac_ctx);

i32 session_deinit (apac_ctx_t *apac_ctx);

#endif
