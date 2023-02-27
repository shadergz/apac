#ifndef APAC_DYN_LOADER_H
#define APAC_DYN_LOADER_H

#include <dlfcn.h>

#if !defined(_GNU_SOURCE)
#error "`_GNU_SOURCE` must be defined somewhere into the code"
#endif

#include <api.h>

external_module_t dyn_loadbyname(const char* objfile);
external_func_t dyn_loadsymbol(external_module_t handle, const char* loadsym);

typedef Dl_info dyninfo_t;

i32 dyn_getinfo(const void* addr, dyninfo_t* fill);
const char* dyn_getsymbolname(const void* addr, dyninfo_t* fill);

i32 dyn_unload(external_module_t objaddr);

#endif


