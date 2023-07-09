#include <stdio.h>
#include <string.h>
#include <time.h>

#include <fast_cache.h>

#include <storage/extio/advise.h>
#include <storage/fhandler.h>

#include <doubly_int.h>
#include <echo/fmt.h>

#include <memctrlext.h>
#include <storage/tree_stg.h>

typedef struct tm native_time_t;

i32 cache_dump_info(apac_ctx_t* apac_ctx)
{
    fast_cache_t* cache = apac_ctx->fastc;
    cache_header_t* header = cache->header;

    native_time_t time_CAM[3] = {};

    localtime_r(&header->creation_date, &time_CAM[0]);
    localtime_r(&header->last_access, &time_CAM[1]);
    localtime_r(&header->last_moder, &time_CAM[2]);

#define FORMATTED_SIZEBF 0x36
    char formatted_time[3][FORMATTED_SIZEBF];

    strftime(formatted_time[0], FORMATTED_SIZEBF, "%T", &time_CAM[0]);
    strftime(formatted_time[1], FORMATTED_SIZEBF, "%T", &time_CAM[1]);
    strftime(formatted_time[2], FORMATTED_SIZEBF, "%T", &time_CAM[2]);

    echo_success(apac_ctx,
        "Treating cache file with:\n"
        "\tCreating date: %s; Last access date: %s; Last modification "
        "date: %s\n",
        formatted_time[0], formatted_time[1], formatted_time[2]);

    return 0;
}

static cache_entry_t*
cache_readent(storage_fio_t* driver, fast_cache_t* cache)
{
#define ENTRY_AUX_BSZ sizeof(typeof(cache_entry_t))
    u8 entry_header[ENTRY_AUX_BSZ];

    cache_entry_t* ent = NULL;

    const off_t file_local = fio_seekbuffer(driver, 0, FIO_SEEK_CURSOR);
    fio_read(driver, entry_header, sizeof entry_header);

    ent = (cache_entry_t*)entry_header;
    if (ent->stream_size == 0)
        return NULL;
    fio_advise(driver, file_local, ent->stream_size, FIO_ADVISE_SEQUENTIAL);

    cache_entry_t* rentry = (cache_entry_t*)apmalloc(
        sizeof(cache_entry_t) + explicit_align(ent->stream_size, 2));
    if (!rentry)
        return NULL;

    memmove(rentry, ent, sizeof(*ent));
    fio_read(driver, rentry + sizeof(*rentry), ent->stream_size);

    return rentry;
}

u64 cache_reload(apac_ctx_t* apac_ctx)
{
    fast_cache_t* cache = apac_ctx->fastc;
    cache_header_t* header = cache->header;

    storage_fio_t* driver = tree_getfile("./layout_level.acache", apac_ctx);

    fio_seekbuffer(driver, 0, FIO_SEEK_SET);
    fio_read(driver, cache->header, sizeof *header);

    // Unloading all collected and modified entries
    doubly_deinit(cache->entries);

    const i32 sanentries_cnt
        = header->string_entcount + header->ocl_binary_entcount;

    if (header->creation_date == 0) {
        header->cache_magic = 0xcace;
        header->creation_date = header->last_moder = time(NULL);
    }
    header->last_access = time(NULL);

    if (sanentries_cnt != header->entries_count) {
        echo_error(apac_ctx,
            "Can't retrieve cache entries, header is corrupted\n");

        header->entries_count = header->string_entcount
            = header->ocl_binary_entcount = 0;
        memset(header->sha2_entrsum, 0, sizeof *header->sha2_entrsum);

        return 0;
    }

    u64 eidx = 0;
    for (; eidx < header->entries_count; eidx++) {
        cache_entry_t* entity = cache_readent(driver, cache);
        if (entity == NULL) {
            echo_error(apac_ctx,
                "Can't allocate a node for a cache entry in "
                "position %lu\n",
                eidx);
            return 0;
        }

        doubly_insert(entity, cache->entries);
    }

    return eidx;
}

i32 cache_init(apac_ctx_t* apac_ctx)
{

    fast_cache_t* cache = apac_ctx->fastc;
    cache->header = (cache_header_t*)apmalloc(sizeof(cache_header_t));
    memset(cache->header, 0, sizeof(*cache->header));

    if (!cache->header) {
        echo_error(apac_ctx, "Can't allocate the cache header structure\n");
        return -1;
    }

    storage_fio_t* cache_file
        = (storage_fio_t*)apmalloc(sizeof(storage_fio_t));
    memset(cache_file, 0, sizeof *cache_file);

    tree_open_file(cache_file, "./layout_level.acache", "rwc:-", apac_ctx);
    if (cache_file->file_fd < 0)
        return -1;

    echo_success(apac_ctx, "Cache file (%s) loaded in %p\n",
        cache_file->file_path, cache_file);

    // We will read the entire cache header at once
    fio_advise(cache_file, 0, sizeof *cache->header, FIO_ADVISE_SEQUENTIAL);
    i32 cre = cache_reload(apac_ctx);
    if (cre != -1)
        cre = 0;

    cre = cache_dump_info(apac_ctx);

    return cre;
}

i32 cache_sync(apac_ctx_t* apac_ctx)
{
    fast_cache_t* cache = apac_ctx->fastc;
    cache_header_t* header = cache->header;
    storage_fio_t* driver = tree_getfile("./layout_level.acache", apac_ctx);

    fio_seekbuffer(driver, 0, FIO_SEEK_SET);
    fio_ondisk(driver, 0, sizeof *header, FIO_ONDISK_PREALLOCATE);

    fio_write(driver, header, sizeof *header);

    // Truncating the file at the end of cache header
    if (fio_get(driver, FIO_GET_ONDISK_SIZE) > sizeof *header) {
        const off_t fcurr = fio_get(driver, FIO_GET_ONDISK_SIZE);

        fio_ondisk(driver, 0, fcurr, FIO_ONDISK_TRUNCATE);
    }

    doubly_reset(cache->entries);
    for (cache_entry_t* ne = NULL; (ne = doubly_next(cache->entries));)
        fio_write(driver, ne, sizeof *ne + ne->stream_size);

    return 0;
}

i32 cache_fetch(cache_entry_t* entries, u64 entries_count, u64 entry_size,
    cache_type_e retr_type, apac_ctx_t* apac_ctx)
{
    return 0;
}

i32 cache_update(cache_entry_t* entry, u64 entry_size, u64 epos,
    apac_ctx_t* apac_ctx)
{
    return 0;
}

i32 cache_deinit(apac_ctx_t* apac_ctx)
{
    cache_sync(apac_ctx);

    fast_cache_t* cache = apac_ctx->fastc;

    if (cache->header != NULL)
        apfree(cache->header);

    doubly_reset(cache->entries);

    for (cache_entry_t* ent = NULL; (ent = doubly_next(cache->entries));)
        apfree(ent);

    doubly_deinit(cache->entries);

    bool cache_hasclosed;
    storage_fio_t* fobject = NULL;
    fobject
        = tree_close_file(&cache_hasclosed, "./layout_level.acache", apac_ctx);

    if (fobject)
        apfree(fobject);

    if (cache_hasclosed != true) {
        echo_error(apac_ctx, "Can't close the cache file\n");
    }

    return cache_hasclosed == true ? 0 : -1;
}
