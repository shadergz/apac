#ifndef APAC_USER_SELECTOR_H
#define APAC_USER_SELECTOR_H

#include <api.h>

i32 select_move (apac_ctx_t *apac_ctx);
i32 select_remove (rule_selector_t *dsel, apac_ctx_t *apac_ctx);

i32 select_change (const char *opt, u64 user_value, apac_ctx_t *apac_ctx);

i32 select_treat (const char *opt, const void *optv, apac_ctx_t *apac_ctx);

#endif
