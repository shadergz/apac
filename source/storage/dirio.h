#ifndef APAC_STORAGE_DIRIO_H
#define APAC_STORAGE_DIRIO_H

#include <api.h>

#include <storage/io_native.h>

#define DIRIO_MAXPATH_SZ 1<<6

i32 dirio_open(const char* path, const char* perm, storage_dirio_t* dir);

u64 dirio_read(void* store_local, u64 buf_size, storage_dirio_t* dir); 
i32 dirio_rewind(storage_dirio_t* dir);

i32 dirio_close(storage_dirio_t* dir);

#endif


