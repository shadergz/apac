#ifndef APAC_STORAGE_FIO_H
#define APAC_STORAGE_FIO_H

#include <api.h>

typedef enum fio_seek {
	FIO_SEEK_SET
} fio_seek_e;

i32 fio_open(const char* path, const char* perm, storage_fio_t* file); 
const char* fio_getpath(const storage_fio_t* file) __attribute__((always_inline));

i32 fio_finish(storage_fio_t* file);
i32 fio_close(storage_fio_t* file); 

const u64 fio_write(storage_fio_t* file, const void* data, u64 datas);
i32 fio_writef(storage_fio_t* file, const char* restrict format, ...);

i32 fio_seekbuffer(storage_fio_t* fio, u64 offset, fio_seek_e seek_type);

const u64 fio_read(storage_fio_t* file, void* data, u64 datas);
i32 fio_readf(storage_fio_t* file, const char* restrict format, ...);


#endif


