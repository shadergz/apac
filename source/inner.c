#include <inner.h>

#include <echo/fmt.h>

#include <sched/gov.h>

i32
inner_apacentry (apac_ctx_t *apac_ctx)
{

  sched_start (apac_ctx);

  echo_success (apac_ctx, "Apac was initialized with success\n"
                          "Hyper fast Android and IOS 's installable package "
                          "decoder/encoder software!\n");

  sched_stop (apac_ctx);

  return 0;
}
