#include <stddef.h>
#include <string.h>

#include <dyn_loader.h>
#include <echo/fmt.h>

i32 dyn_getinfo(const void* addr, dyninfo_t* fill) {
	if (addr == NULL || fill == NULL) return -1;
	return dladdr(addr, fill);
}
external_module_t dyn_loadbyname(const char* objfile) {
	if (objfile == NULL) return NULL;

	void* shared = dlopen(objfile, RTLD_LAZY);

	if (shared == NULL) {
		echo_error(NULL, "Can't load a shared object with "
				"pathname: %s, because of %s\n", objfile, dlerror());
		return NULL;
	}

	echo_info(NULL, "New object loaded with address %p from pathname %s\n", 
			shared, objfile);

	return shared;

}
external_func_t dyn_loadsymbol(external_module_t handle, const char* loadsym) {
	if (handle == NULL) return NULL;

	void* dlobject = dlsym(handle, loadsym);
	if (dlobject == NULL) return NULL;

	echo_info(NULL, "New object loaded at address %p with name `%s` from object %p\n", 
			dlobject, loadsym, handle);

	return dlobject;
}

const char* dyn_getsymbolname(const void* addr, dyninfo_t* fill) {
	if (addr == NULL || fill == NULL) return NULL;
	
	const i32 ret = dladdr(addr, fill);

	echo_debug(NULL, "Attempting to locate a object symbol " 
		"name with %p address has %s\n", addr, fill->dli_sname ? "successed" : "failed");
	if (ret == 0) {
		memset(fill, 0, sizeof *fill);
		return NULL;
	}

	const char* possible_name = fill->dli_sname;
	return possible_name != NULL ? possible_name : "unresolvable";
}

i32 dyn_unload(external_module_t objaddr) {
	if (objaddr == NULL) return -1;

	echo_info(NULL, "Unloading a shared object (SO) at %p\n", objaddr);

	dlclose(objaddr);
	return 0;
}


