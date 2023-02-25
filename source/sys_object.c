#include <dlfcn.h>
#include <stddef.h>

#include <sys_object.h>

external_module_t sys_loadobject(const char* object_file) {
	if (object_file == NULL) return NULL;

	return dlopen(object_file, RTLD_LAZY);
}

external_func_t sys_loadsym(external_module_t handle, const char* loadsym) {
	if (handle == NULL) return NULL;

	return dlsym(handle, loadsym);
}

i32 sys_unload(external_module_t object) {
	if (object == NULL) return -1;

	dlclose(object);
	return 0;
}


