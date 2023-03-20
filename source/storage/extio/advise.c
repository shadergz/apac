#include <fcntl.h>

#include <storage/extio/advise.h>

i32
fio_advise (storage_fio_t *file, u64 offset, u64 len, fio_advise_e ad)
{
  if (!file)
    return -1;

  const i32 ffd = file->file_fd;

  switch (ad)
    {
    case FIO_ADVISE_NEEDED:
      posix_fadvise (ffd, offset, len, POSIX_FADV_WILLNEED);
      break;
    case FIO_ADVISE_AVOID:
      posix_fadvise (ffd, offset, len, POSIX_FADV_DONTNEED);
      break;
    case FIO_ADVISE_SEQUENTIAL:
      posix_fadvise (ffd, offset, len, POSIX_FADV_SEQUENTIAL);
      break;
    case FIO_ADVISE_RANDOM:
      posix_fadvise (ffd, offset, len, POSIX_FADV_RANDOM);
      break;
    default:
      return -1;
    }

  return 0;
}
