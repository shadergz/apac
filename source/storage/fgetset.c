#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <storage/fhandler.h>

i32
fio_ondisk (storage_fio_t *file, u64 offset, u64 len, fio_ondisk_e method_type)
{
  if (!file)
    return -1;

  switch (method_type)
    {
    case FIO_ONDISK_PREALLOCATE:
      posix_fallocate (file->file_fd, offset, len);
      break;
    case FIO_ONDISK_TRUNCATE:
      ftruncate (file->file_fd, len);
      if (file->cursor_offset > len)
        file->cursor_offset = len;
    }

  return 0;
}

typedef struct stat native_stat_t;

u64
fio_get (storage_fio_t *file, fio_get_e get)
{
  if (file == NULL)
    return 0;
  u64 getret = 0;

  switch (get)
    {
    case FIO_GET_ONDISK_SIZE:
      {
        native_stat_t fs = {};
        fstat (file->file_fd, &fs);
        return (u64)fs.st_size;
      }
    case FIO_GET_ONDISK_CURSOR:
      getret = lseek (file->file_fd, 0, SEEK_CUR);
      break;
    }

  return getret;
}
