
#ifndef APAC_STORAGE_EXTIO_FLOCK_H
#define APAC_STORAGE_EXTIO_FLOCK_H

#include <storage/fio.h>

typedef enum fio_locker {
        FIO_LOCKER_WRITE
} fio_locker_e;


i32 fio_unlock(storage_fio_t* file);

i32 fio_lock(storage_fio_t* file, fio_locker_e locker);

#endif

