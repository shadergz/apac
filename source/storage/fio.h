#ifndef APAC_STORAGE_FIO_H
#define APAC_STORAGE_FIO_H

#include <api.h>

i32 fio_open(const char* path, const char* perm, storage_fio_t* file); 
const char* fio_getpath(const storage_fio_t* file);

i32 fio_finish(storage_fio_t* file);
i32 fio_close(storage_fio_t* file); 

const u64 fio_write(storage_fio_t* file, const void* data, u64 datas);
i32 fio_writef(storage_fio_t* file, const char* restrict format, ...);

const u64 fio_read(storage_fio_t* file, void* data, u64 datas);
i32 fio_readf(storage_fio_t* file, const char* restrict format, ...);


#endif


