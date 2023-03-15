#ifndef APAC_DEBUG_EXTRA_H
#define APAC_DEBUG_EXTRA_H

#include <stdarg.h>
#include <stdio.h>

#include <echo/fmt.h>

/*
void debug_printout(const char *fmt, ...) {
    #define DBG_BSZ 0x100
    va_list va;
    va_start(va, fmt);

    char dbg[DBG_BSZ];
    vsnprintf (dbg, sizeof dbg, fmt, va);

    echo_debug(NULL, "%s", fmt);
    va_end(va);
}
#define DEBUG_DUMP_STRUCT(st)\
    _Generic((st), \
    typeof(st): __builtin_dump_struct(&st, debug_printout),\
    default: __builtin_dump_struct(&st, debug_printout)\
    )
*/

#endif // APAC_DEBUG_EXTRA_H
