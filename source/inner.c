#include <inner.h>

#include <echo/fmt.h>

#include <fast_cache.h>
#include <sched/gov.h>

i32
inner_apacentry (apac_ctx_t *apac_ctx)
{
  i32 cret;
  if ((cret = cache_init (apac_ctx)) == -1)
    goto goout;
  cache_dump_info (apac_ctx);

  if ((cret = sched_start (apac_ctx)) == -1)
    goto goout;

  echo_success (apac_ctx, "Apac was initialized with success\n"
                          "Hyper fast Android and IOS 's installable package "
                          "decoder/encoder software!\n");

  sched_stop (apac_ctx);
  cache_deinit (apac_ctx);

goout:
  return cret;
}
