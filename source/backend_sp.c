
#include <stddef.h>
#include <stdio.h>

#include <backend_sp.h>
#include <layer.h>
#include <memctrlext.h>
#include <ocl_driver.h>
#include <tip.h>

#include <echo/fmt.h>
#include <storage/fhandler.h>

static i32
back_load_ocl (apac_ctx_t *apac_ctx)
{
  backend_ctx_t *core = apac_ctx->core_backend;
  core->ocl_shared = (storage_fio_t *)apmalloc (sizeof (storage_fio_t));

  opencl_int_t *interface = core->ocl_interface;
  interface->ocl_driver = NULL;

  echo_success (apac_ctx,
                "Searching for a valid OpenCL driver on your system\n");

  if (core->ocl_shared == NULL)
    return -1;

  i32 bret = -1;

  char *ocl_pathname = NULL;
  // Attempting to load an "OpenCL shared object" local reference
  if (fio_open ("libOpenCL.so", "ref", core->ocl_shared) == 0)
    goto load_now;
  if (fio_open ("libOpenCL.so.1", "ref", core->ocl_shared) == 0)
    goto load_now;

  if (fio_open ("/usr/lib/x86_64-linux-gnu/libOpenCL.so.1", "ref",
                core->ocl_shared)
      == 0)
    goto load_now;

#if defined(__ANDROID__)
  const char *ocl_system = "/system/vendor/lib64";
#elif defined(__linux__)
  const char *ocl_system = "/usr/lib";
#endif

  layer_asprintf (&ocl_pathname, "%s/libOpenCL.so", ocl_system);
  if (fio_open (ocl_pathname, "ref", core->ocl_shared) == 0)
    goto load_now;

  echo_error (apac_ctx, "Can't found a valid OpenCL driver on your "
                        "system paths\n");
  tip_ocl_driver ("OCL_NOT_FOUND");

  /* Can't found a valid OpenCL reference! */
  goto backret;

load_now:
  __attribute__ ((hot));

  echo_info (apac_ctx,
             "There's a OpenCL driver located at %s, "
             "trying to load it!\n",
             core->ocl_shared->file_path);

  bret = ocl_init (apac_ctx);
  if (bret != 0)
    {

      echo_error (apac_ctx, "Can't load the OpenCL driver %s\n",
                  core->ocl_shared->file_name);
      fio_finish (core->ocl_shared);
      apfree (core->ocl_shared);
    }

backret:
  if (ocl_pathname)
    apfree (ocl_pathname);
  return bret;
}

static i32
back_unload_ocl (apac_ctx_t *apac_ctx)
{
  backend_ctx_t *core = apac_ctx->core_backend;
  if (core->ocl_shared == NULL)
    return -1;

  ocl_deinit (apac_ctx);
  fio_finish (core->ocl_shared);
  apfree (core->ocl_shared);

  return 0;
}

i32
back_select_devices (apac_ctx_t *apac_ctx)
{
  backend_ctx_t *backend = apac_ctx->core_backend;
  cl_platform_id platform_id;
  cl_uint num_devices;

  // Trying to fetch a regular GPU device or fallback to a CPU
  ocl_getplatformids (apac_ctx, 1, &platform_id, NULL);
  // Getting the number of devices currently available
  ocl_getdeviceids (apac_ctx, platform_id, CL_DEVICE_TYPE_ALL, 0, NULL,
                    &num_devices);

  if (!num_devices)
    {
      echo_error (apac_ctx,
                  "No one device has found inside the selected plataform!\n");
      return -1;
    }

  const i32 sucret
      = ocl_getdeviceids (apac_ctx, platform_id, CL_DEVICE_TYPE_GPU, 1,
                          &backend->device_inuse, NULL);

  if (backend->device_inuse == NULL || sucret != 0)
    {
      echo_warning (apac_ctx, "Can't found a valid GPU device in your "
                              "platform, falling back to a valid CPU\n");
      ocl_getdeviceids (apac_ctx, platform_id, CL_DEVICE_TYPE_CPU, 1,
                        &backend->device_inuse, NULL);
    }

#define DRIVER_DEVICE_NAME 0x5f
  char driver_name[DRIVER_DEVICE_NAME] = {};
  ocl_getdeviceinfo (apac_ctx, backend->device_inuse, CL_DEVICE_NAME,
                     DRIVER_DEVICE_NAME, &driver_name, NULL);

  echo_success (apac_ctx, "%s\n", driver_name);

  backend->sound_device_inuse = alcOpenDevice (NULL);
  if (!backend->sound_device_inuse)
    {
      echo_error (apac_ctx, "Can't found a valid OpenAL device\n");
      return -1;
    }

  backend->sound_context
      = alcCreateContext (backend->sound_device_inuse, NULL);
  if (!backend->sound_context)
    {
      echo_error (apac_ctx,
                  "Can't create a valid context object for a OpenAL device\n");
      return -1;
    }

  alcMakeContextCurrent (backend->sound_context);

  echo_success (
      apac_ctx, "Found OpenAL device: %s\n",
      alcGetString (backend->sound_device_inuse, ALC_DEVICE_SPECIFIER));

  return 0;
}

i32
back_init (apac_ctx_t *apac_ctx)
{
  backend_ctx_t *core = apac_ctx->core_backend;
  core->ocl_interface
      = (opencl_int_t *)apmalloc (sizeof (*core->ocl_interface));
  if (core->ocl_interface == NULL)
    return -1;

  core->sound_device_inuse = NULL;
  core->sound_context = NULL;

  const i32 back_ret = back_load_ocl (apac_ctx);

  if (back_ret != 0)
    return back_ret;

  return 0;
}

i32
back_deinit (apac_ctx_t *apac_ctx)
{
  backend_ctx_t *back = apac_ctx->core_backend;
  if (back == NULL)
    return -1;

  if (back->sound_device_inuse != NULL)
    {
      alcMakeContextCurrent (NULL);
      alcDestroyContext (back->sound_context);
      alcCloseDevice (back->sound_device_inuse);
    }

  if (back->ocl_interface != NULL)
    {
      back_unload_ocl (apac_ctx);
      apfree (back->ocl_interface);
    }

  return 0;
}
