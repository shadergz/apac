#ifndef APAC_OCL_DRIVER_H
#define APAC_OCL_DRIVER_H

#include <api.h>

i32 ocl_init (apac_ctx_t *apac_ctx);

i32 ocl_deinit (apac_ctx_t *apac_ctx);

i32 ocl_getdeviceids (apac_ctx_t *apac_ctx, cl_platform_id platform,
                      cl_device_type device_type, cl_uint num_entries,
                      cl_device_id *devices, cl_uint *num_devices);

i32 ocl_getdeviceinfo (apac_ctx_t *apac_ctx, cl_device_id device,
                       cl_device_info param_name, size_t param_value_size,
                       void *param_value, size_t *param_value_size_ret);

i32 ocl_getplatformids (apac_ctx_t *apac_ctx, cl_uint num_entries,
                        cl_platform_id *platforms, cl_uint *num_platforms);

#endif
