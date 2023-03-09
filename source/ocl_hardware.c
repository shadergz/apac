#include <ocl_hardware.h>

#include <dyn_loader.h>
#include <stddef.h>

#include <echo/fmt.h>
#include <storage/fio.h>

#define AP_LOAD_FUNC(ctx, ptr, sym_name, sym_type, lib_name, onerr)           \
  do                                                                          \
    {                                                                         \
      if (ptr == NULL)                                                        \
        return onerr;                                                         \
      ptr->sym_name = (sym_type)dyn_loadsymbol (ptr->ocl_driver, #sym_name);  \
      if (__builtin_expect (ptr->sym_name == NULL, 0))                        \
        {                                                                     \
          echo_error (ctx, "Can't load %s function entry point in (%s)\n",    \
                      #sym_name, #lib_name);                                  \
        }                                                                     \
    }                                                                         \
  while (0)

i32
ocl_init (apac_ctx_t *apac_ctx)
{
  backend_ctx_t *be_context = apac_ctx->core_backend;
  opencl_int_t *ocl_int = be_context->ocl_interface;

  if (ocl_int == NULL)
    {
      echo_error (apac_ctx, "OCL: interface can't be null for "
                            "this operation\n");
      return -1;
    }

  ocl_int->ocl_driver
      = dyn_loadbyname (((storage_fio_t *)be_context->ocl_shared)->file_path);
  if (!ocl_int->ocl_driver)
    return -1;

  AP_LOAD_FUNC (apac_ctx, ocl_int, clGetDeviceIDs, OCL_GETDEVICEIDS_FUNC,
                OpenCL, -1);

  return 0;
}

const char *
ocl_native_strerr (cl_int err)
{

#define CL_ERR(e_type)                                                        \
  case e_type:                                                                \
    return #e_type
#define CL_DEFAULT(e_type)                                                    \
  default:                                                                    \
    return #e_type

  switch (err)
    {
      // This list contains all OpenCL 1.2 possible problems and errors!

      CL_ERR (CL_SUCCESS);
      CL_ERR (CL_DEVICE_NOT_FOUND);
      CL_ERR (CL_DEVICE_NOT_AVAILABLE);
      CL_ERR (CL_COMPILER_NOT_AVAILABLE);
      CL_ERR (CL_MEM_OBJECT_ALLOCATION_FAILURE);
      CL_ERR (CL_OUT_OF_RESOURCES);
      CL_ERR (CL_OUT_OF_HOST_MEMORY);
      CL_ERR (CL_PROFILING_INFO_NOT_AVAILABLE);
      CL_ERR (CL_MEM_COPY_OVERLAP);
      CL_ERR (CL_IMAGE_FORMAT_MISMATCH);
      CL_ERR (CL_IMAGE_FORMAT_NOT_SUPPORTED);
      CL_ERR (CL_BUILD_PROGRAM_FAILURE);
      CL_ERR (CL_MAP_FAILURE);
      CL_ERR (CL_MISALIGNED_SUB_BUFFER_OFFSET);
      CL_ERR (CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);

      // Compiler problems
      CL_ERR (CL_COMPILE_PROGRAM_FAILURE);
      CL_ERR (CL_LINKER_NOT_AVAILABLE);
      CL_ERR (CL_LINK_PROGRAM_FAILURE);

      CL_ERR (CL_DEVICE_PARTITION_FAILED);
      CL_ERR (CL_KERNEL_ARG_INFO_NOT_AVAILABLE);

      // Compile-time errors

      CL_ERR (CL_INVALID_VALUE);
      CL_ERR (CL_INVALID_DEVICE_TYPE);
      CL_ERR (CL_INVALID_PLATFORM);
      CL_ERR (CL_INVALID_DEVICE);
      CL_ERR (CL_INVALID_CONTEXT);
      CL_ERR (CL_INVALID_QUEUE_PROPERTIES);
      CL_ERR (CL_INVALID_COMMAND_QUEUE);
      CL_ERR (CL_INVALID_HOST_PTR);
      CL_ERR (CL_INVALID_MEM_OBJECT);
      CL_ERR (CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
      CL_ERR (CL_INVALID_IMAGE_SIZE);
      CL_ERR (CL_INVALID_SAMPLER);
      CL_ERR (CL_INVALID_BINARY);
      CL_ERR (CL_INVALID_BUILD_OPTIONS);
      CL_ERR (CL_INVALID_PROGRAM);
      CL_ERR (CL_INVALID_PROGRAM_EXECUTABLE);
      CL_ERR (CL_INVALID_KERNEL_NAME);
      CL_ERR (CL_INVALID_KERNEL_DEFINITION);
      CL_ERR (CL_INVALID_KERNEL);
      CL_ERR (CL_INVALID_ARG_INDEX);
      CL_ERR (CL_INVALID_ARG_VALUE);
      CL_ERR (CL_INVALID_ARG_SIZE);
      CL_ERR (CL_INVALID_KERNEL_ARGS);
      CL_ERR (CL_INVALID_WORK_DIMENSION);
      CL_ERR (CL_INVALID_WORK_GROUP_SIZE);
      CL_ERR (CL_INVALID_WORK_ITEM_SIZE);
      CL_ERR (CL_INVALID_GLOBAL_OFFSET);
      CL_ERR (CL_INVALID_EVENT_WAIT_LIST);
      CL_ERR (CL_INVALID_EVENT);
      CL_ERR (CL_INVALID_OPERATION);
      CL_ERR (CL_INVALID_GL_OBJECT);
      CL_ERR (CL_INVALID_BUFFER_SIZE);
      CL_ERR (CL_INVALID_MIP_LEVEL);
      CL_ERR (CL_INVALID_GLOBAL_WORK_SIZE);
      CL_ERR (CL_INVALID_PROPERTY);
      CL_ERR (CL_INVALID_IMAGE_DESCRIPTOR);
      CL_ERR (CL_INVALID_COMPILER_OPTIONS);
      CL_ERR (CL_INVALID_LINKER_OPTIONS);
      CL_ERR (CL_INVALID_DEVICE_PARTITION_COUNT);

      CL_DEFAULT (CL_UNKNOWN_ERROR);
    }

  /* All extensions problems will not be converted, because its
   * usage is by default erroneous! */

  return NULL;
}

i32
ocl_getdeviceids (apac_ctx_t *apac_ctx, cl_platform_id platform,
                  cl_device_type device_type, cl_uint num_entries,
                  cl_device_id *devices, cl_uint *num_devices)
{

  echo_assert (NULL, apac_ctx != NULL,
               "Context is null, you can't use this\n");

  backend_ctx_t *back = apac_ctx->core_backend;
  opencl_int_t *ocl_ptr = back->ocl_interface;

  const cl_int err = ocl_ptr->clGetDeviceIDs (
      platform, device_type, num_entries, devices, num_devices);
  if (err != CL_SUCCESS)
    {
      echo_error (apac_ctx, "clGetDeviceIDs was failed because: %s\n",
                  ocl_native_strerr (err));
      return -1;
    }

  return 0;
}

i32
ocl_deinit (apac_ctx_t *apac_ctx)
{
  backend_ctx_t *back = apac_ctx->core_backend;
  if (back->ocl_interface == NULL)
    return 0;

  opencl_int_t *openCL = back->ocl_interface;

  dyn_unload (openCL->ocl_driver);

  openCL->clGetDeviceIDs = NULL;

  return 0;
}
