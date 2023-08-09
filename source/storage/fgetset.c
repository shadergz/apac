#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <echo/fmt.h>
#include <storage/fhandler.h>

i32 fio_ondisk(storage_fio_t* file, u64 offset, u64 len, fio_ondisk_e method_type)
{
    if (!file)
        return -1;

    int truncated = 0;

    switch (method_type) {
    case FIO_ONDISK_PREALLOCATE:
        posix_fallocate(file->file_fd, offset, len);
        break;
    case FIO_ONDISK_TRUNCATE:
        truncated = ftruncate(file->file_fd, len);
        if (file->cursor_offset > len)
            file->cursor_offset = len;
    }

    if (truncated != 0) {
        echo_error(NULL, "ftruncate() has failed to truncate file with name %s\n",
            file->file_name);
        return -1;
    }

    return 0;
}

typedef struct stat native_stat_t;

u64 fio_get(storage_fio_t* file, fio_get_e get)
{
    if (file == NULL)
        return 0;
    u64 getret[1] = {};

    switch (get) {
    case FIO_GET_ONDISK_SIZE: {
        native_stat_t fs = {};
        fstat(file->file_fd, &fs);

        getret[0] = fs.st_size;
        break;
    }
    case FIO_GET_ONDISK_CURSOR:
        getret[0] = lseek(file->file_fd, 0, SEEK_CUR);
        break;
    }

    return getret[0];
}
