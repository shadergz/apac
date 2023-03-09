#ifndef APAC_LAYER_H
#define APAC_LAYER_H

#include <api.h>

#include <stdarg.h>

i32 layer_vasprintf (char **restrict lstrp, const char *restrict lfmt,
                     va_list lap);
i32 layer_asprintf (char **restrict lstrp, const char *restrict lfmt, ...);

#endif
