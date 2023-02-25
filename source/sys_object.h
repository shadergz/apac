#ifndef APAC_SYS_OBJECT_H
#define APAC_SYS_OBJECT_H

#include <api.h>

external_module_t sys_loadobject(const char* object_file);

external_func_t sys_loadsym(external_module_t handle, const char* loadsym);

i32 sys_unload(external_module_t object);

#endif


