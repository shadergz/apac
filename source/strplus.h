#ifndef APAC_STRPLUS_H
#define APAC_STRPLUS_H

#include <api.h>

typedef enum padding_mode
{
  PADDING_MODE_END,
  PADDING_MODE_START,
  PADDING_MODE_CENTER

} padding_mode_e;

char *strplus_padding (char *dest, const char *src, u64 dest_size, i32 charset,
                       padding_mode_e paddm);

#endif
