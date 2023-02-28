#ifndef APAC_STORAGE_IO_NATIVE_H
#define APAC_STORAGE_IO_NATIVE_H

#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>

#include <api.h>

enum storage_flags {
	STORAGE_FLAG_DIRECTORY    = O_DIRECTORY,
	STORAGE_FLAG_READ         = O_RDONLY,
	STORAGE_FLAG_WRITE        = O_WRONLY,
	STORAGE_FLAG_RW           = O_RDWR,
	STORAGE_FLAG_FILE         = ~O_DIRECTORY,

	STORAGE_FLAG_CREAT        = O_CREAT
};

enum storage_perms {
	STORAGE_PERM_USER_READ    = S_IRUSR,
	STORAGE_PERM_USER_WRITE   = S_IWUSR,
	STORAGE_PERM_USER_EXEC    = S_IXUSR,
	STORAGE_PERM_USER_FULL    = S_IRWXU,

	STORAGE_PERM_GROUP_READ   = S_IRGRP,
	STORAGE_PERM_GROUP_WRITE  = S_IWGRP,
	STORAGE_PERM_GROUP_EXEC   = S_IXGRP,
	STORAGE_PERM_GROUP_FULL   = S_IRWXG,

	STORAGE_PERM_OTHERS_READ  = S_IROTH,
	STORAGE_PERM_OTHERS_WRITE = S_IWOTH,
	STORAGE_PERM_OTHERS_EXEC  = S_IXOTH,
	STORAGE_PERM_OTHERS_FULL  = S_IRWXO,
};

typedef struct native_dirent {
	u64 d_ino;
	u64 d_off;
	u16 d_reclen;
	u8 d_type;
	char d_name[];
} native_dirent_t;

typedef i32 native_flags_t;
typedef mode_t native_perms_t;

u64 inline __attribute__((always_inline)) dirio_getentries
	(i32 fd, native_dirent_t* dirinfo, u32 count) {
	return syscall(SYS_getdents64, (u32)fd, dirinfo, count);
}

native_flags_t native_solve_flags(const char* flags);
native_perms_t native_solve_perms(const char* perms);

native_perms_t native_default_perms(storage_node_id_e node_type);

#endif

