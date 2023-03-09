#include <stdlib.h>
#include <string.h>

#include <echo/fmt.h>
#include <tip.h>

#if defined(__ANDROID__)
static const char *s_lib_local = "/system/vendor/lib64";
#endif

#define CHECK_TIP(tip, name) if (strncasecmp (tip, name, strlen (tip)) == 0)

void
tip_ocl_driver (const char *tip_driver)
{
  CHECK_TIP ("OCL_NOT_FOULD", tip_driver)
  {
#if defined(__ANDROID__)
    const char *ldlib = getenv ("LD_LIBRARY_PATH");

    if (ldlib != NULL)
      if (strstr (ldlib, s_lib_local) != NULL)
        return;

    echo_info (
        NULL,
        "If you are on Termux, you must set "
        "LD_LIBRARY_PATH to something like this: "
        "`export LD_LIBRARY_PATH=/system/vendor/lib64:$LD_LIBRARY_PATH`");
#endif
  }
}
