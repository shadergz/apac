#include <string.h>

#include <memctrlext.h>
#include <storage/dirhandler.h>

#include <fcntl.h>
#include <unistd.h>

const char *
dirio_getname (storage_dirio_t *dir)
{
  return dir->dir_path;
}

i32
dirio_open (const char *path, const char *perms, storage_dirio_t *dir)
{
  if (dir == NULL)
    return -1;
  dir->dir_path = strdup (path);
  // This piece of memory doesn't needed to be allocated because it's part of
  // `dir_path`
  dir->dir_name = strrchr (dir->dir_path, '/');
  if (dir->dir_name == NULL)
    dir->dir_name = dir->dir_path;
  else
    dir->dir_name++;

  const native_flags_t native_perm = native_solve_flags (perms);
  const native_perms_t native_mode = native_solve_perms (perms);

  dir->dir_fd = open (path, native_perm, native_mode);
  dir->buf_pos = dir->buf_end = 0;

  if (dir->dir_fd < 2 || !native_perm || !native_mode)
    {
      apfree ((void *)dir->dir_path);
      dir->dir_relative = NULL;
      return -1;
    }

  return 0;
}

i32
dirio_rewind (storage_dirio_t *dir)
{
  if (dir->dir_fd < 3)
    return -1;
  return dir->buf_pos = dir->buf_end = dir->position
         = lseek (dir->dir_fd, 0, SEEK_SET);
}

u64
dirio_read (void *store_local, u64 buf_size, storage_dirio_t *dir)
{
  if (dir->dir_fd < 3 || store_local == NULL || buf_size == 0)
    return 0;

  if (__builtin_expect ((dir->buf_pos >= dir->buf_end), 0))
    {
      const u64 dir_len = dirio_getentries (
          dir->dir_fd, (native_dirent_t *)dir->dir_info, sizeof dir->dir_info);
      dir->buf_end = dir_len;
      dir->buf_pos = 0;

      if (!dir->buf_end)
        {
          memset (store_local, 0, buf_size);
          return 0;
        }
    }

  const native_dirent_t *entity_info
      = (const native_dirent_t *)(dir->dir_info + dir->buf_pos);
  dir->buf_pos += entity_info->d_reclen;
  /* Length of this linux_dirent */
  dir->position = entity_info->d_off;

  memcpy (store_local, (const void *)entity_info, entity_info->d_reclen);

  return entity_info->d_reclen;
}

i32
dirio_close (storage_dirio_t *dir)
{
  if (dir->dir_path != NULL)
    {
      apfree ((char *)dir->dir_path);
      dir->dir_path = dir->dir_name = NULL;
    }

  if (dir->dir_fd > 0)
    {
      close (dir->dir_fd);
      dir->dir_fd = -1;
    }

  return 0;
}
