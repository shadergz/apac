#include <sched/gov.h>

#include <echo/fmt.h>

#include <memctrlext.h>

/* Spawn all physical threads as logical workers after fetcher the
 * real core count from the host CPU */
i32
sched_start (apac_ctx_t *apac_ctx)
{
  u8 physic_cores = super_getcores ();
  echo_assert (apac_ctx, physic_cores > 0,
               "Pool: invalid count of cores, "
               "this must be reported!\n");

#define CPU_REAL_NAME 0x40
#define CPU_VENDOR_MODELSZ 0x40
#define CPU_FEATURESSZ 0x70

  char *cpu_realname = (char *)apmalloc (sizeof (char) * CPU_REAL_NAME);
  char *cpu_vendormodel
      = (char *)apmalloc (sizeof (char) * CPU_VENDOR_MODELSZ);
  char *cpu_features = (char *)apmalloc (sizeof (char) * CPU_FEATURESSZ);

  if (!cpu_realname || !cpu_vendormodel || !cpu_features)
    {
      echo_error (apac_ctx,
                  "Has occured a problem when attempt to "
                  "allocate 3 blocks for store hosts CPU data information\n");
      return -1;
    }

  scalar_cpuinfo (cpu_vendormodel, cpu_realname, cpu_features,
                  CPU_VENDOR_MODELSZ, CPU_REAL_NAME, CPU_FEATURESSZ, NULL,
                  NULL);
  if (!*cpu_realname)
    scalar_cpuname (cpu_realname, CPU_REAL_NAME);

  echo_success (apac_ctx,
                "Sched Thread Pool is taking controller of:\n"
                "\tName: %s\n"
                "\tVendor: %s\n"
                "\tCPU features: %s\n",
                cpu_realname, cpu_vendormodel, cpu_features);

  apfree (cpu_realname);
  apfree (cpu_vendormodel);
  apfree (cpu_features);

  return 0;
}

i32
sched_stop (apac_ctx_t *apac_ctx)
{
  return 0;
}
