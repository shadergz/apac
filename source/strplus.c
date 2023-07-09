#include <stdio.h>
#include <string.h>

#include <echo/fmt.h>

#include <strplus.h>

char* strplus_padding(char* dest, const char* src, u64 dest_size, i32 charset,
    padding_mode_e paddm)
{
    u8 character = (u8)charset;
    char* origin_ptr = dest;
    while (dest_size && dest_size-- > 0)
        *dest++ = (char)character;

    dest_size = (u64)(dest - origin_ptr);

    u64 spos = 0;
    u64 originlen = strlen(src);

    switch (paddm) {
    case PADDING_MODE_END:
        if (dest_size > originlen)
            spos = dest_size - originlen - 1;
        echo_assert(NULL, (dest_size - spos) > 0, "Wtf, what's happens here?!");

        snprintf(origin_ptr + spos, dest_size - spos, "%s", src);
        break;
    case PADDING_MODE_START:
    case PADDING_MODE_CENTER:
        break;
    }

    return origin_ptr;
}
