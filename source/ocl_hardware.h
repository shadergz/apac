#ifndef APAC_OCL_HARDWARE_H
#define APAC_OCL_HARDWARE_H

#include <api.h>

i32 ocl_init(apac_ctx_t* apac_ctx);

i32 ocl_deinit(apac_ctx_t* apac_ctx);

i32 ocl_getdeviceids(apac_ctx_t* apac_ctx, cl_platform_id platform, 
		cl_device_type device_type, cl_uint num_entries, 
		cl_device_id* devices, cl_uint* num_devices); 
#endif

