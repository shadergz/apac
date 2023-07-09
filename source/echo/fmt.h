#ifndef APAC_ECHO_FMT_H
#define APAC_ECHO_FMT_H

#include <echo/core.h>

#define echo_detailed(ctx, level, format, ...) \
    echo_do(ctx, level, __LINE__, __FILE__, __func__, format, ##__VA_ARGS__)

#define echo_success(ctx, format, ...) \
    echo_detailed(ctx, ECHO_LEVEL_SUCCESS, format, ##__VA_ARGS__)

#define echo_info(ctx, format, ...) \
    echo_detailed(ctx, ECHO_LEVEL_INFO, format, ##__VA_ARGS__)

#define echo_error(ctx, format, ...) \
    echo_detailed(ctx, ECHO_LEVEL_ERROR, format, ##__VA_ARGS__)

#define echo_warning(ctx, format, ...) \
    echo_detailed(ctx, ECHO_LEVEL_WARNING, format, ##__VA_ARGS__)

#if defined(APAC_IS_UNDER_DEBUG)
#define echo_debug(ctx, format, ...) \
    echo_detailed(ctx, ECHO_LEVEL_DEBUG, format, ##__VA_ARGS__)
#else
#define echo_debug(ctx, format, ...)
#endif

#endif
