#ifndef APAC_STORAGE_FHANDLER_H
#define APAC_STORAGE_FHANDLER_H

#include <api.h>

typedef enum fio_seek {
    FIO_SEEK_SET,
    FIO_SEEK_CURSOR,
} fio_seek_e;

typedef enum fio_ondisk {
    FIO_ONDISK_PREALLOCATE,
    FIO_ONDISK_TRUNCATE
} fio_ondisk_e;

typedef enum fio_get {
    FIO_GET_ONDISK_SIZE,
    FIO_GET_ONDISK_CURSOR
} fio_get_e;

i32 fio_open(const char* path, const char* perm, storage_fio_t* file);

i32 fio_finish(storage_fio_t* file);
i32 fio_close(storage_fio_t* file);

const u64 fio_write(storage_fio_t* file, const void* data, u64 datas);
i32 fio_snwritef(char* out, u64 outs, storage_fio_t* file,
    const char* restrict format, ...);

i32 fio_seekbuffer(storage_fio_t* fio, u64 offset, fio_seek_e seek_type);

i32 fio_ondisk(storage_fio_t* file, u64 offset, u64 len,
    fio_ondisk_e method_type);
u64 fio_get(storage_fio_t* file, fio_get_e get);

const u64 fio_read(storage_fio_t* file, void* data, u64 datas);
i32 fio_snreadf(char* out, u64 out_size, storage_fio_t* file,
    const char* restrict format, ...);

#endif
