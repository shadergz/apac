#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <storage/fhandler.h>

i32 fio_snreadf(char* out, u64 out_size, storage_fio_t* file,
    const char* restrict format, ...)
{
    va_list va;
    va_start(va, format);

    u8* out_buffer = (u8*)out;
    if (!out_buffer || !out_size)
        return -1;

    fio_read(file, out_buffer, out_size);
    const i32 vss = vsscanf((char*)out_buffer, format, va);

    va_end(va);

    return vss;
}

i32 fio_snwritef(char* out, u64 outs, storage_fio_t* file,
    const char* restrict format, ...)
{
    va_list va;
    va_start(va, format);

    u8* wrb_buffer = (u8*)out;
    if (!wrb_buffer || !outs)
        return -1;

    vsnprintf((char*)out, outs, format, va);

    const i32 fiow = (i32)fio_write(file, out, strlen(out));

    va_end(va);

    return fiow;
}
