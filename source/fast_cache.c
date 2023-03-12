#include <string.h>
#include <time.h>

#include <fast_cache.h>

#include <storage/extio/advise.h>
#include <storage/fio.h>

#include <doubly_int.h>
#include <echo/fmt.h>

#include <memctrlext.h>
#include <storage/tree.h>

typedef struct tm native_time_t;

i32
cache_dump_info (apac_ctx_t *apac_ctx)
{
  fast_cache_t *cache = apac_ctx->user_session->fastc;
  cache_header_t *header = cache->header;

  native_time_t *time_CAM[3] = {};

  time_CAM[0] = localtime (&header->creation_date);
  time_CAM[1] = localtime (&header->last_access);
  time_CAM[2] = localtime (&header->last_moder);

#define FORMATTED_SIZEBF 0x100
  char formatted_time[FORMATTED_SIZEBF][3];

  strftime (formatted_time[0], FORMATTED_SIZEBF, "%T", time_CAM[0]);
  strftime (formatted_time[1], FORMATTED_SIZEBF, "%T", time_CAM[1]);
  strftime (formatted_time[2], FORMATTED_SIZEBF, "%T", time_CAM[2]);

  echo_success (apac_ctx,
                "Treating cache file with:\n"
                "\tCreating date: %s; Last access date: %s; Last modification "
                "date: %s\n",
                formatted_time[0], formatted_time[1], formatted_time[2]);

  return 0;
}

static cache_entry_t *
cache_readent (storage_fio_t *driver, fast_cache_t *cache)
{

  cache_entry_t *ent = NULL;

  fio_read (driver, cache->aux_cache, sizeof cache->aux_cache);

  const off_t file_local = fio_seekbuffer (driver, 0, FIO_SEEK_CURSOR);
  fio_advise (driver, file_local, sizeof cache->aux_cache, FIO_ADVISE_ENTIRE);

  ent = (cache_entry_t *)cache->aux_cache;

  return ent;
}

u64
cache_reload (apac_ctx_t *apac_ctx)
{
  fast_cache_t *cache = apac_ctx->user_session->fastc;
  cache_header_t *header = cache->header;

  storage_fio_t *driver = tree_getfile ("layout_level.acache", apac_ctx);

  fio_seekbuffer (driver, 0, FIO_SEEK_SET);
  fio_read (driver, cache->header, sizeof *header);

  // Unloading all collected and modified entries
  doubly_deinit (cache->entries);

  const i32 sanentries_cnt
      = header->string_entcount + header->ocl_binary_entcount;

  if (header->creation_date == 0)
    header->creation_date = time (NULL);
  header->last_access = time (NULL);

  if (sanentries_cnt != header->entries_count)
    {
      echo_error (apac_ctx,
                  "Can't retrieve cache entries, header is corrupted\n");

      header->entries_count = header->string_entcount
          = header->ocl_binary_entcount = 0;
      memset (header->sha2_entrsum, 0, sizeof *header->sha2_entrsum);

      return 0;
    }

  for (u64 eidx = 0; eidx < header->entries_count; eidx++)
    {
      cache_entry_t *entity = cache_readent (driver, cache);
      cache_entry_t *dnode
          = apmalloc (sizeof (u8) * entity->stream_size + sizeof *entity - 8);

      if (dnode == NULL)
        {
          echo_error (
              apac_ctx,
              "Can't allocate a node for a cache entry in position %lu\n",
              eidx);
          return -1;
        }

      doubly_insert (dnode, cache->entries);
    }

  return 0;
}

i32
cache_init (apac_ctx_t *apac_ctx)
{

  fast_cache_t *cache = apac_ctx->user_session->fastc;
  storage_fio_t *cache_file
      = (storage_fio_t *)apmalloc (sizeof (storage_fio_t));
  memset (cache_file, 0, sizeof *cache_file);

  tree_open_file (cache_file, "layout_level.acache", "rwc:-", apac_ctx);
  if (cache_file->file_fd < 0)
    return -1;

  echo_success (apac_ctx, "Cache file (%s) loaded in %p\n",
                cache_file->file_path, cache_file);

  // We will read the entire cache header at once
  fio_advise (cache_file, 0, sizeof *cache->header, FIO_ADVISE_ENTIRE);
  const i32 cre = cache_reload (apac_ctx);

  return cre;
}

i32
cache_sync (apac_ctx_t *apac_ctx)
{
  fast_cache_t *cache = apac_ctx->user_session->fastc;
  cache_header_t *header = cache->header;
  storage_fio_t *driver = tree_getfile ("layout_level.acache", apac_ctx);

  fio_seekbuffer (driver, 0, FIO_SEEK_SET);
  fio_ondisk (driver, 0, sizeof *header, FIO_ONDISK_PREALLOCATE);

  fio_write (driver, header, sizeof *header);

  return 0;
}

i32
cache_fetch (cache_entry_t *entries, u64 entries_count, cache_type_e retr_type,
             apac_ctx_t *apac_ctx)
{
  return 0;
}

i32
cache_update (cache_entry_t *entry, u64 entry_size, u64 epos,
              apac_ctx_t *apac_ctx)
{
  return 0;
}

i32
cache_deinit (apac_ctx_t *apac_ctx)
{
  cache_sync (apac_ctx);

  fast_cache_t *header = apac_ctx->user_session->fastc;

  doubly_deinit (header->entries);

  bool cache_hasclosed;
  tree_close_file (&cache_hasclosed, "layout_level.acache", apac_ctx);

  return 0;
}
