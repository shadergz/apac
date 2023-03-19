#include <math.h>
#include <string.h>

#include <host/cache.h>
#include <vec.h>

#include <memctrlext.h>

i32
vec_resize (u64 new_capa, vecdie_t *vec)
{
  // We can't resize the vector down
  if (vec->vec_used > new_capa)
    return -1;
  // Data size not specified
  if (vec->vec_dsize == 0)
    return -1;
  if (vec->vec_dynamic == NULL)
    {
      vec->vec_dynamic = (u8 *)apmalloc (new_capa * vec->vec_dsize);
      if (vec->vec_dynamic == NULL)
        return -1;
    }
  vec->vec_capa = new_capa;

  return 0;
}

u64
vec_capacity (const vecdie_t *vec)
{
  return vec->vec_capa;
}

u64
vec_using (const vecdie_t *vec)
{
  return vec->vec_used;
}

const void *
vec_ptr (const vecdie_t *vec)
{
  return vec->vec_dynamic;
}

void *
vec_next (vecdie_t *vec)
{
  u64 skip_bytes = vec->vec_dsize * vec->vec_cursor++;

  if (skip_bytes >= vec->vec_dsize * vec->vec_used)
    return NULL;

  return (void *)((u8 *)vec->vec_dynamic + skip_bytes);
}

void
vec_reset (vecdie_t *vec)
{
  vec->vec_cursor = 0;
}

i32
vec_init (u64 item_size, u64 initial_capa, vecdie_t *vec)
{
  memset (vec, 0, sizeof (*vec));
  vec->vec_dsize = item_size;

  i32 ret = 0;
  if ((ret = vec_resize (initial_capa, vec)) != 0)
    {
      return ret;
    }

  return 0;
}

i32
vec_deinit (vecdie_t *vec)
{
  if (vec->vec_dynamic != NULL)
    {
      apfree (vec->vec_dynamic);
    }
  vec->vec_dynamic = NULL;
  vec->vec_used = vec->vec_capa = 0;
  return 0;
}

// Vector glow algorithm
static u64
vec_agrow (vecdie_t *vec)
{
  if (vec->vec_capa < 100)
    {
      return vec->vec_capa * 2;
    }

  if (vec->vec_capa < 250)
    {
      /* Calculating 25 percentage of what `vec->vec_capa` currently is */
      double per_result = vec->vec_capa * 0.25;
      return vec->vec_capa + (u64)per_result;
    }

  double rational = pow (vec->vec_capa, 2);
  return (u64)rational;
}

static i32
vec_copy (void *dest, void *src, vecdie_t *vec)
{
  if (dest == NULL || src == NULL)
    return -1;

  if (vec->vec_dsize % 2 != 0 || vec->vec_dsize > sizeof (long))
    {
      memcpy (dest, src, vec->vec_dsize);

      return 0;
    }

  u8 *dptr = (u8 *)dest;
  u8 *sptr = (u8 *)src;
  u8 ssize = vec->vec_dsize;
  for (; ssize; ssize--)
    *dptr++ = *sptr++;

  return 0;
}

static inline i32
vec_grow (vecdie_t *vec)
{
  if (vec->vec_capa > vec->vec_used)
    return 0;

  const i32 resize = vec_resize (vec_agrow (vec), vec);
  return resize;
}

void *
vec_at (u64 index, vecdie_t *vec)
{
  if (index > vec->vec_capa)
    return NULL;
  u64 addr_index = vec->vec_dsize * index;
  return vec->vec_dynamic + addr_index;
}

i32
vec_push (void *item, vecdie_t *vec)
{
  if (vec_grow (vec) != 0)
    return -1;

  const i32 vec_copied = vec_copy (vec_at (vec->vec_used++, vec), item, vec);

  if (vec_copied != 0)
    return vec_copied;

  return 0;
}

void *
vec_emplace (vecdie_t *vec)
{
  if (vec_grow (vec) != 0)
    return NULL;

  void *data = vec_at (vec->vec_used++, vec);
  if (data == NULL)
    return NULL;

  cache_prefetch (data, vec->vec_dsize, CACHE_PREFETCH_LOW_WRITE);
  return data;
}

i32
vec_pop (vecdie_t *vec)
{
  if (vec->vec_used == 0)
    return -1;

  vec->vec_used--;
  void *delete = vec->vec_dynamic + (vec->vec_dsize * vec->vec_used);
  memset (delete, 0, vec->vec_dsize);

  return 0;
}
