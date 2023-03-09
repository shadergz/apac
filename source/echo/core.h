#ifndef APAC_ECHO_CORE_H
#define APAC_ECHO_CORE_H

#include <api.h>

typedef enum echo_level
{
  ECHO_LEVEL_SUCCESS,
  ECHO_LEVEL_INFO,

#if defined(APAC_IS_UNDER_DEBUG)
  ECHO_LEVEL_DEBUG,
#endif

  ECHO_LEVEL_WARNING,

  ECHO_LEVEL_ERROR,

  ECHO_LEVEL_ASSERT

} echo_level_e;

i32 echo_init (apac_ctx_t *apac_ctx);
i32 echo_deinit (apac_ctx_t *apac_ctx);

i32 echo_do (apac_ctx_t *apac_ctx, echo_level_e msg_level, i32 code_line,
             const char *code_filename, const char *func_name,
             const char *format, ...) __attribute__ ((format (printf, 6, 7)));

extern const char *assert_format;

#define echo_assert(ctx, condition, message)                                  \
  do                                                                          \
    {                                                                         \
      if (!(condition))                                                       \
        echo_detailed (ctx, ECHO_LEVEL_ASSERT, assert_format, __LINE__,       \
                       __FILE__, __func__, #condition, message);              \
    }                                                                         \
  while (0)

#endif
