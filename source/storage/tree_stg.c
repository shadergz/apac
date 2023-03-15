
#include <stdio.h>
#include <string.h>

#include <layer.h>
#include <memctrlext.h>
#include <storage/dirhandler.h>

#include <storage/fhandler.h>
#include <storage/tree_stg.h>

#include <echo/fmt.h>

#include <doubly_int.h>

u32
tree_makeroot (const char *relative, apac_ctx_t *apac_ctx)
{
  if (relative == NULL)
    return -1;

  storage_tree_t *root = apac_ctx->root;

  memset (root, 0, sizeof (storage_tree_t));
  root->node_level = 0;

  storage_dirio_t *dir = apmalloc (sizeof (storage_dirio_t));
  root->leafs = apmalloc (sizeof (doublydie_t));
  doubly_init (root->leafs);

  tree_open_dir (dir, relative, apac_ctx);
  return (u32)root->node_level;
}

// Fetch a tree structure from user real fs path
storage_tree_t *
tree_getfromuser (const char *user, storage_tree_t *root)
{
  return root;
}

i32
tree_solve_relative (storage_tree_t *here, char *out, u64 out_size,
                     storage_tree_t *root)
{
  if (out_size < 2)
    return -1;
  if (root == here)
    {
      *out++ = '.';
      *out = '\0';
      return 1;
    }
  return -1;
}

i32
tree_detach_file (storage_tree_t *from, const char *restrict relative,
                  storage_fio_t **in, apac_ctx_t *apac_ctx)
{
  *in = NULL;

  char *file_relative;

  for (storage_tree_t *next = NULL;
       (next = doubly_next (from->leafs)) != NULL;)
    {
      if (next->node_id == STORAGE_NODE_ID_DIR)
        continue;
      storage_fio_t *nfile = next->node_file;

      if (nfile == NULL)
        return -1;

      file_relative = NULL;
      layer_asprintf (&file_relative, "%s/%s", nfile->file_rel,
                      nfile->file_name);
      if (strncmp (file_relative, relative, strlen (file_relative)) != 0)
        {
          apfree (file_relative);
          continue;
        }
      /* We have found the correct relative pathname file object
       * dropping the actual node pointed by `cursor`! */
      doubly_drop (from->leafs);

      *in = nfile;
      apfree (next);
      goto detached;
    }
detached:
  if (file_relative != NULL)
    apfree (file_relative);

  doubly_reset (from->leafs);
  return *in != NULL ? 0 : -1;
}

storage_fio_t *
tree_close_file (bool *closed, const char *filename, apac_ctx_t *apac_ctx)
{
  *closed = false;
  storage_tree_t *root = apac_ctx->root;
  storage_tree_t *file_dir = tree_getfromuser (filename, root);

  storage_fio_t *fio;
  tree_detach_file (file_dir, filename, &fio, apac_ctx);

  if (fio)
    {
      *closed = fio_finish (fio) == 0;
      apfree ((char *)fio->file_rel);
      fio->file_rel = 0;
    }

  return fio;
}

i32
tree_open_dir (storage_dirio_t *place, const char *user_path,
               apac_ctx_t *apac_ctx)
{

  storage_tree_t *root = apac_ctx->root;
  storage_tree_t *dir_put = tree_getfromuser (user_path, root);
  if (dir_put == NULL)
    return -1;

  /* Runtime execution directory is used as the root of our system, everything
   * that performs I/O operations will use this root directory as a
   * requirement! */
  i32 dirio = dirio_open (user_path, "d:7-", place);
  if (dirio != 0)
    {
      echo_error (apac_ctx,
                  "Can't open a directory (%s) inside the tree "
                  "(%s)\n",
                  user_path, dirio_getname (dir_put->node_dir));
      return dirio;
    }

  dirio = tree_attach_dir (dir_put, place);
  if (dirio != 0)
    return dirio;

  char rel_path[TREE_PATH_MAX_REL];
  tree_solve_relative (dir_put, rel_path, sizeof (rel_path), root);

  place->dir_relative = strdup (rel_path);
  return 0;
}

storage_fio_t *
tree_getfile (const char *relative, apac_ctx_t *apac_ctx)
{
  if (relative == NULL || apac_ctx == NULL)
    return NULL;
  storage_tree_t *dirlocal = tree_getfromuser (relative, apac_ctx->root);
  storage_fio_t *desired = NULL;

  for (storage_tree_t *cursor = NULL;
       (cursor = doubly_next (dirlocal->leafs)) != NULL && desired == NULL;)
    {

      storage_fio_t *fnode = cursor->node_file;

      char *full_relpath = NULL;
      layer_asprintf (&full_relpath, "%s/%s", fnode->file_rel,
                      fnode->file_name);

      if (strncmp (full_relpath, relative, strlen (full_relpath)) == 0)
        desired = fnode;
      apfree (full_relpath);
    }

  doubly_reset (dirlocal->leafs);
  return desired;
}

i32
tree_open_file (storage_fio_t *file, const char *path, const char *perm,
                apac_ctx_t *apac_ctx)
{
  storage_tree_t *root = apac_ctx->root;
  storage_tree_t *dir_put = tree_getfromuser (path, root);
  if (dir_put == NULL)
    return -1;

  i32 fio_ret = fio_open (path, perm, file);
  if (fio_ret != 0)
    {
      echo_error (apac_ctx, "Can't open a file with pathname %s\n", path);
      return fio_ret;
    }

  fio_ret = tree_attach_file (dir_put, file);

  if (fio_ret < 0)
    return fio_ret;

  char fnrel[TREE_PATH_MAX_REL];
  tree_solve_relative (dir_put, fnrel, sizeof (fnrel), root);

  file->file_rel = strdup (fnrel);

  return 0;
}

i32
tree_fill (storage_tree_t *mirror, storage_tree_t *fill, storage_fio_t *fmem,
           storage_dirio_t *dmem)
{
  storage_node_id_e id = 0;
  u64 level = 0;

  if (fill == NULL)
    fill = mirror;

  if (!fmem && dmem)
    {
      id = STORAGE_NODE_ID_DIR;
      fill->node_dir = dmem;
      level = mirror->node_level + 1;

      goto fillup;
    }

  if (!(!dmem && fmem))
    return -1;

  id = STORAGE_NODE_ID_FILE;
  fill->node_file = fmem;
  level = mirror->node_level;

fillup:
  fill->node_id = id;
  if (mirror != fill)
    {
      fill->node_level = level;
      fill->parent = mirror;
    }

  return 0;
}

i32
tree_attach_dir (storage_tree_t *with, storage_dirio_t *dir)
{
  if (__builtin_expect ((with == NULL), 0))
    return -1;
  tree_fill (with, NULL, NULL, dir);
  return 0;
}

i32
tree_attach_file (storage_tree_t *with, storage_fio_t *file)
{
  if (with->node_id != STORAGE_NODE_ID_DIR)
    {
      echo_error (NULL, "Can't attach a file into another file\n");
      return -1;
    }

  storage_tree_t *file_fs = (storage_tree_t *)apmalloc (sizeof (*with));
  if (file_fs == NULL)
    return -1;

  tree_fill (with, file_fs, file, NULL);

  const i32 ret = doubly_insert (file_fs, with->leafs);
  return ret;
}

i32
tree_close (storage_tree_t *collapse, bool force)
{
  if (collapse->node_id == STORAGE_NODE_ID_DIR)
    {
      for (storage_tree_t *next = NULL;
           (next = doubly_next (collapse->leafs)) != NULL;)
        {
          tree_close (next, true);
        }

      dirio_close (collapse->node_dir);

      apfree ((char *)collapse->node_dir->dir_relative);
      apfree (collapse->node_dir);

      doubly_deinit (collapse->leafs);
      apfree (collapse->leafs);

      goto goahead;
    }

  if (!collapse->node_file)
    return 0;

  apfree ((char *)collapse->node_file->file_rel);
  fio_close (collapse->node_file);

  apfree (collapse->node_file);

goahead:
  collapse->leafs = NULL;
  return 0;
}
