
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <errno.h>
#include <stdarg.h>

#include <sys/stat.h>

#include <storage/fio.h>
#include <storage/extio/flock.h>
#include <storage/extio/stream_mime.h>

#include <memctrlext.h>
#include <echo/fmt.h>

#include <storage/io_native.h>
#include <unistd.h>

#define FIO_IS_REFERENCE(perm) (strncasecmp(perm, "ref", 3) == 0)

typedef struct stat native_stat_t;

static i32 fio_check(const native_stat_t* stat_buffer, storage_fio_t* file)
{	
	if (!(!S_ISREG(stat_buffer->st_mode) && 
		S_ISLNK(stat_buffer->st_mode))) return -1;

#define REAL_FILENAME_SZ 0x32
	char real_fn[REAL_FILENAME_SZ];

	const i32 ret = readlink(fio_getpath(file), real_fn, sizeof real_fn - 1);
	if (ret == -1) return ret;

	file->real_filename = strdup(real_fn);
	file->is_link = true;

	return 0;
}

i32 fio_open(const char* path, const char* perm, storage_fio_t* file) {	
	memset(file, 0, sizeof(*file));
	
	native_stat_t* stat_buffer = (native_stat_t*)file->rw_cache;

	const native_flags_t flags = native_solve_flags(perm);
	native_perms_t perms = native_solve_perms(perm);
	if (!perms) 
		perms = native_default_perms(STORAGE_NODE_ID_FILE);

	const i32 stat_ret = stat(path, stat_buffer);

	if (stat_ret != 0 && !(flags & STORAGE_FLAG_CREAT)) 
		// In case that we can't create a new file, go out!
		goto ferror;

	if (fio_check(stat_buffer, file) != 0) goto ferror;

	file->file_path = strdup(path);
	const char* relobs = strrchr(file->file_path, '/');
	
	if (relobs)
		file->file_name = strdup(relobs + 1);

	if (__builtin_expect(FIO_IS_REFERENCE(perm), 0)) return 0;
	file->file_fd = open(path, flags, perms);

	if (file->file_fd < 3) goto ferror;

	file->mime_identifier = mime_fromfile(file);
	
	file->cache_cursor = file->rw_cache;
	file->cursor_offset = file->cache_block = 0;

	return 0;

ferror:
	file->recerror = errno;
	return -1;
}

#define FIO_MIN(x, y) ((x > y ? y : x))

static inline bool fio_insideblock(const storage_fio_t* file) {
	if (file->cursor_offset < 4096 && file->cache_cursor == 0) 
		return true;

	return false;
}

const u64 fio_write(storage_fio_t* file, const void* data, u64 datas) {
	if (file == NULL) return -1;

	const u64 wb = write(file->file_fd, data, datas);
	if (wb == -1) {
		file->recerror = errno;
		return -1;
	}
	memcpy(file->cache_cursor, data, 
			FIO_MIN(file->rw_cache - file->cache_cursor, datas));

	file->cursor_offset += datas;
	return wb;
}

const u64 fio_read(storage_fio_t* file, void* data, u64 datas) {
	if (file == NULL) return -1;

	u64 br = 0;
	if (datas < (file->rw_cache - file->cache_cursor) && fio_insideblock(file)) {
		memcpy(data, file->rw_cache, datas);
		br = datas;
		goto update_rc;
	}

	br = read(file->file_fd, data, datas);

	if (__builtin_expect(br == -1 || br > SSIZE_MAX, 0)) {

		echo_error(NULL, "Couldn't read from the file %s because of: %s\n",
			file->file_path, strerror(errno));
		if (errno == EBADF)
			echo_info(NULL, "May the FD %d from %s is corrupted or not "
					"more valid\n", file->file_fd, file->file_path);
		return br;
	}
update_rc:
	file->cursor_offset += datas;
	file->cache_cursor += datas;

	return br;
}

i32 fio_seekbuffer(storage_fio_t* fio, u64 offset, fio_seek_e seek_type) {
	switch (seek_type) {
	case FIO_SEEK_SET:
		lseek(fio->file_fd, offset, SEEK_SET);
	}
	const i32 fret = fsync(fio->file_fd);

	return fret;
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

	const i32 fiow = (i32)fio_write(file, file->rw_cache, 
			strlen((const char*)file->rw_cache));

	va_end(va);

	return fiow;
}

i32 fio_finish(storage_fio_t* file) {
	if (file->is_locked) {
		echo_info(NULL, "Unlocking the file fd %d with filename %s\n", 
				file->file_fd, file->file_name);
		fio_unlock(file);
	}

	if (file->is_link)   apfree((char*)file->real_filename);
	if (file->file_name) apfree((char*)file->file_name);
	
	if (file->file_path) {
		apfree((char*)file->file_path);
	}

	
	file->file_path = file->file_name = NULL;
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

