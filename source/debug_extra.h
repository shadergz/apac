#ifndef APAC_DEBUG_EXTRA_H
#define APAC_DEBUG_EXTRA_H

#include <stdarg.h>
#include <stdio.h>

#include <echo/fmt.h>

i32 debug_printout(const char* restrict fmt, ...)
{
#define DBG_BSZ 0x100
    va_list va;
    va_start(va, fmt);

    char dbg[DBG_BSZ];
    const i32 pret = vsnprintf(dbg, sizeof dbg, fmt, va);

    echo_debug(NULL, "%s", dbg);
    va_end(va);

    return pret;
}
#if __has_builtin(__builtin_dump_struct)
#define DEBUG_DUMP_STRUCT(st)                                      \
    _Generic((st), typeof(st)                                      \
             : __builtin_dump_struct(&st, debug_printout), default \
             : __builtin_dump_struct(&st, debug_printout))
#else
#define DEBUG_DUMP_STRUCT(st)
#endif

#endif
