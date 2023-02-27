

#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <storage/flock.h>

i32 fio_unlock(storage_fio_t* file) {
        if (file == NULL) return -1;
        if (file->is_locked != true) return -1;

        static struct flock unlock = {};
        unlock.l_type = F_UNLCK;
        if (fcntl(file->file_fd, F_SETLK, &unlock)) return -1;

        return 0;
}

i32 fio_lock(storage_fio_t* file, fio_locker_e locker) {
        if (file == NULL) return -1;
        if (file->is_locked != false) return -1;

        struct flock lock_card;
        memset(&lock_card, 0, sizeof(lock_card));
        
        switch (locker) {
        case FIO_LOCKER_WRITE:
                lock_card.l_type = F_WRLCK; break;
        }

        while (fcntl(file->file_fd, F_SETLK, &lock_card)) {
                
                if (errno != EINTR) return -1;

        }

        file->is_locked = true;

        return 0;
}
