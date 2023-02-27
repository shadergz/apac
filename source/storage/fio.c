#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include <sys/stat.h>

#include <storage/fio.h>
#include <storage/flock.h>

#include <memctrlext.h>
#include <echo/fmt.h>

#include <storage/io_native.h>

#define FIO_IS_REFERENCE(perm) (strncasecmp(perm, "ref", 3) == 0)

typedef struct stat native_stat_t;

i32 fio_open(const char* path, const char* perm, storage_fio_t* file) {	
	memset(file, 0, sizeof(*file));
	
	native_stat_t* stat_buffer = (native_stat_t*)file->rw_cache;
	const i32 stat_ret = stat(path, stat_buffer);

	if (stat_ret != 0) goto ferror;

	if (!S_ISREG(stat_buffer->st_mode) && S_ISLNK(stat_buffer->st_mode)) {
		#define REAL_FILENAME_SZ 0x32
		char real_fn[REAL_FILENAME_SZ];

		const i32 ret = readlink(path, real_fn, sizeof real_fn - 1);
		if (ret == -1) goto ferror;

		file->real_filename = strdup(real_fn);

		file->is_link = true;
	}

	file->file_path = strdup(path);
	const char* relobs = strrchr(file->file_path, '/');
	
	if (relobs)
		file->file_name = strdup(relobs + 1);


	if (__builtin_expect(FIO_IS_REFERENCE(perm), 0)) return 0;
	
	const native_flags_t flags = native_solve_flags(perm);
	const native_perms_t perms = native_solve_perms(perm);
	file->file_fd = open(path, flags, perms);

	if (file->file_fd < 3) goto ferror;

	return 0;

	ferror:
	file->recerror = errno;
	return -1;
}

const u64 fio_write(storage_fio_t* file, const void* data, u64 datas) {
	if (file == NULL) return -1;

	return write(file->file_fd, data, datas);
}

const u64 fio_read(storage_fio_t* file, void* data, u64 datas) {
	if (file == NULL) return -1;

	return read(file->file_fd, data, datas);
}

i32 fio_readf(storage_fio_t* file, const char* restrict format, ...) {
	va_list va;
	va_start(va, format);

	fio_read(file, file->rw_cache, sizeof file->rw_cache);
	const i32 vss = vsscanf((char*)file->rw_cache, format, va);

	va_end(va);

	return vss;
}

i32 fio_writef(storage_fio_t* file, const char* restrict format, ...) {
	va_list va;
	va_start(va, format);

	vsnprintf((char*)file->rw_cache, sizeof file->rw_cache, format, va);

	const i32 fiow = (i32)fio_write(file, file->rw_cache, strlen((const char*)file->rw_cache));

	va_end(va);

	return fiow;
}

i32 fio_finish(storage_fio_t* file) {
	if (file->is_link)   apfree((char*)file->real_filename);
	if (file->file_name) apfree((char*)file->file_name);
	
	if (file->file_path) {
		apfree((char*)file->file_path);
	}

	file->file_path = file->file_name = NULL;

	if (file->is_locked) {
		echo_info(NULL, "Unlocking the file with pathname %s\n", file->real_filename);
		fio_unlock(file);
	}

	const i32 cl = fio_close(file);

	return cl;
}

const char* fio_getpath(const storage_fio_t* file) {
	if (file == NULL) return NULL;
	if (file->is_link) return file->real_filename;

	return file->file_path;
}

i32 fio_close(storage_fio_t* file) {
	if (file->file_fd < 2) return -1;

	close(file->file_fd);
	file->file_fd = 0;
	return 0;
}

