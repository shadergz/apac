#ifndef APAC_MEMCTRLEXT_H
#define APAC_MEMCTRLEXT_H

#include <api.h>

void* apcalloc(u64 nele, u64 esize);
void* apmalloc(u64 rsize);

void apfree(void* endptr);

u64 explicit_align(u64 nsize, u64 alignment);

#endif


