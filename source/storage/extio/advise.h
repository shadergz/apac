#ifndef APAC_STORAGE_EXTIO_ADVISE_H
#define APAC_STORAGE_EXTIO_ADVISE_H

#include <api.h>

typedef enum fio_advise
{
  FIO_ADVISE_NEEDED,
  FIO_ADVISE_AVOID,
  FIO_ADVISE_ENTIRE,
  FIO_ADVISE_RANDOM,

} fio_advise_e;

i32 fio_advise (storage_fio_t *file, u64 offset, u64 len, fio_advise_e ad);

#endif
