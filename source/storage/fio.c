
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <stdarg.h>

#include <sys/stat.h>

#include <storage/extio/flock.h>
#include <storage/extio/stream_mime.h>
#include <storage/fio.h>

#include <echo/fmt.h>
#include <memctrlext.h>

#include <storage/io_native.h>
#include <unistd.h>

#define FIO_IS_REFERENCE(perm) (strncasecmp (perm, "ref", 3) == 0)

typedef struct stat native_stat_t;

static i32
fio_check (const native_stat_t *stat_buffer, storage_fio_t *file)
{
  if (!(!S_ISREG (stat_buffer->st_mode) && S_ISLNK (stat_buffer->st_mode)))
    {
      file->is_link = false;

      return 0;
    }

#define REAL_FILENAME_SZ 0x32
  char real_fn[REAL_FILENAME_SZ];

  const i32 ret = readlink (file->file_path, real_fn, sizeof real_fn - 1);
  if (ret == -1)
    return ret;

  file->real_filename = strdup (real_fn);
  file->is_link = true;

  return 0;
}

i32
fio_open (const char *path, const char *perm, storage_fio_t *file)
{
  memset (file, 0, sizeof (*file));

  native_stat_t *stat_buffer = (native_stat_t *)file->rw_cache;

  const native_flags_t flags = native_solve_flags (perm);
  native_perms_t perms = native_solve_perms (perm);
  if (!perms)
    perms = native_default_perms (STORAGE_NODE_ID_FILE);

  const i32 stat_ret = stat (path, stat_buffer);

  if (stat_ret != 0 && !(flags & STORAGE_FLAG_CREAT))
    // In case that we can't create a new file, go out!
    goto ferror;

  file->file_path = strdup (path);
  const char *relobs = strrchr (file->file_path, '/');

  if (relobs)
    file->file_name = strdup (relobs + 1);
  if (__builtin_expect (FIO_IS_REFERENCE (perm), 0))
    return 0;
  if (fio_check (stat_buffer, file) != 0)
    goto ferror;

  file->file_fd = open (path, flags, perms);

  if (file->file_fd < 3)
    goto ferror;

  file->mime_identifier = mime_fromfile (file);

  file->cache_cursor = file->rw_cache;
  file->cursor_offset = file->cache_block = 0;

  return 0;

ferror:
  file->recerror = errno;
  return -1;
}

const u64
fio_write (storage_fio_t *file, const void *data, u64 datas)
{
  if (file == NULL)
    return -1;

  if (datas > file->cache_cursor - file->rw_cache)
    {
      file->cache_cursor = file->rw_cache;
      file->cache_valid = 0;
    }
  u64 wbytes = 0;

  if (!file->cache_valid && datas > file->cache_cursor - file->rw_cache)
    {
      do
        {
          wbytes = (u64)write (file->file_fd, data, datas);

          if (wbytes == -1)
            {
              file->recerror = errno;
              return wbytes;
            }

          file->cursor_offset += wbytes;
          if (wbytes < sizeof (file->rw_cache))
            goto copy2cache;

          // Updating output data transfer location pointer
          data += wbytes;

          file->cache_block++;
        }
      while ((datas %= sizeof (file->rw_cache)) != 0);
    }

copy2cache:
  memcpy (file->cache_cursor, data, wbytes);
  file->cache_valid = (file->cache_cursor - file->rw_cache) + wbytes;

  file->cursor_offset += wbytes;
  return wbytes;
}

const u64
fio_read (storage_fio_t *file, void *data, u64 datas)
{

  if (file == NULL)
    return -1;
  if (data == NULL)
    return -1;
  if (datas == 0)
    return -1;

  u64 br = 0;

  u8 *output_data = (u8 *)data;

  if (datas <= file->cache_valid)
    {
      echo_assert (NULL,
                   (file->cache_cursor - file->rw_cache) >= file->cache_valid,
                   "Something is going "
                   "wrong, cursor is outside the bounds\n");

      memcpy (output_data, file->cache_cursor, datas);
      goto update_rc;
    }

  if (!file->cache_cursor || datas > (file->cache_cursor - file->rw_cache))
    {
      file->cache_cursor = file->rw_cache;
      file->cache_valid = 0;
    }

  // Reading into the cache block
  do
    {
      br = read (file->file_fd, output_data, datas);
      if (br == -1 || br > SSIZE_MAX)
        goto error_read;

      if (br < sizeof (file->rw_cache))
        goto upcache;

      output_data += br;

      file->cursor_offset += br;
      file->cache_cursor += br;

      file->cache_block++;

      if (file->cache_cursor - file->rw_cache > sizeof (file->rw_cache))
        {
          const u64 inptr = (file->cache_cursor - file->rw_cache)
                            % sizeof (file->rw_cache);
          file->cache_cursor = file->rw_cache + inptr;
        }
    }
  while ((datas %= 4096) != 0);

upcache:
  memcpy (file->cache_cursor, output_data, br);
  file->cache_valid = br;

update_rc:
  file->cursor_offset += br;
  file->cache_cursor += br;

  return datas;

error_read:
  echo_error (NULL, "Couldn't read from the file %s because of: %s\n",
              file->file_path, strerror (errno));
  if (errno == EBADF)
    echo_info (NULL,
               "May the fd %d from %s is corrupted or not "
               "more valid\n",
               file->file_fd, file->file_path);

  if (br == -1)
    file->recerror = errno;
  return br;
}

i32
fio_seekbuffer (storage_fio_t *file, u64 offset, fio_seek_e seek_type)
{
  switch (seek_type)
    {
    case FIO_SEEK_SET:
      lseek (file->file_fd, offset, SEEK_SET);
      if (offset < file->cache_valid)
        file->cache_cursor = file->rw_cache + offset;
      file->cursor_offset = offset;
    }
  const i32 fret = fsync (file->file_fd);

  return fret;
}

i32
fio_snreadf (char *out, u64 out_size, storage_fio_t *file,
             const char *restrict format, ...)
{
  va_list va;
  va_start (va, format);

  u8 *out_buffer = (u8 *)out;
  if (!out_buffer || !out_size)
    return -1;

  fio_read (file, out_buffer, out_size);
  const i32 vss = vsscanf ((char *)out_buffer, format, va);

  va_end (va);

  return vss;
}

i32
fio_snwritef (char *out, u64 outs, storage_fio_t *file,
              const char *restrict format, ...)
{
  va_list va;
  va_start (va, format);

  u8 *wrb_buffer = (u8 *)out;
  if (!wrb_buffer || !outs)
    return -1;

  vsnprintf ((char *)out, outs, format, va);

  const i32 fiow = (i32)fio_write (file, out, strlen (out));

  va_end (va);

  return fiow;
}

i32
fio_finish (storage_fio_t *file)
{
  if (!file)
    return 0;

  if (file->is_locked)
    {
      echo_info (NULL, "Unlocking the file fd %d with filename %s\n",
                 file->file_fd, file->file_name);
      fio_unlock (file);
    }

  if (file->is_link)
    apfree ((char *)file->real_filename);
  if (file->file_name)
    apfree ((char *)file->file_name);

  if (file->file_path)
    {
      apfree ((char *)file->file_path);
    }

  file->file_path = file->file_name = NULL;
  const i32 cl = fio_close (file);

  return cl;
}

i32
fio_close (storage_fio_t *file)
{
  if (file->file_fd < 2)
    return -1;

  close (file->file_fd);
  file->file_fd = 0;
  return 0;
}
