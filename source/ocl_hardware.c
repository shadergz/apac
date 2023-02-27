#include <ocl_hardware.h>

#include <stddef.h>
#include <dyn_loader.h>

#include <storage/fio.h>
#include <echo/fmt.h>

#define AP_LOAD_FUNC(ctx, ptr, sym_name, sym_type, lib_name, onerr)\
	do {\
		if (ptr == NULL) return onerr;\
		ptr->sym_name = (sym_type)dyn_loadsymbol(ptr->ocl_driver, #sym_name);\
		if (__builtin_expect(ptr->sym_name == NULL, 0)) {\
			echo_error(ctx, "Can't load %s function entry point in (%s)\n",\
				#sym_name, #lib_name);\
		}\
	} while (0)

i32 ocl_init(apac_ctx_t* apac_ctx) {
	backend_ctx_t* be_context = apac_ctx->core_backend;
	opencl_int_t* ocl_int = be_context->ocl_interface;
	
	ocl_int->ocl_driver = dyn_loadbyname(fio_getpath(be_context->ocl_shared)); 
	if (ocl_int == NULL) return -1;

	AP_LOAD_FUNC(apac_ctx, ocl_int, clGetDeviceIDs, OCL_GETDEVICEIDS_FUNC, OpenCL, -1);

	return 0;
}

i32 ocl_deinit(apac_ctx_t* apac_ctx) {
	backend_ctx_t* back = apac_ctx->core_backend;
	if (back->ocl_interface == NULL) return 0;

	opencl_int_t* openCL = back->ocl_interface;

	dyn_unload(openCL->ocl_driver);

	openCL->clGetDeviceIDs = NULL;

	return 0;
}

