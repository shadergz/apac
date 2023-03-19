
#include <string.h>

#include <doubly_int.h>
#include <memctrlext.h>

#include <cyclic_hw.h>

i32
doubly_init (doublydie_t *doubly)
{
  if (doubly == NULL)
    return -1;
  memset (doubly, 0, sizeof *doubly);

  doubly->prev = NULL;

  return 0;
}

u64
doubly_size (const doublydie_t *doubly)
{
  if (!doubly)
    return -1;
  if (doubly->prev)
    return -1;

  if (doubly->next == NULL && doubly->node_data)
    return 1;

  const doublydie_t *aux = doubly;
  u64 nsize = 1;
  while (aux->next)
    {
      aux = aux->next;
      nsize++;
    }

  return nsize;
}

i32
doubly_reset (doublydie_t *doubly)
{
  if (doubly == NULL)
    return -1;
  doubly->cursor = NULL;

  return 0;
}

void *
doubly_curr (const doublydie_t *doubly)
{
  if (!doubly)
    return NULL;
  if (!doubly->cursor)
    return doubly->node_data;

  return doubly->cursor->node_data;
}

void *
doubly_get (u64 pos, doublydie_t *doubly)
{
  if (!doubly)
    return NULL;
  doubly_reset (doubly);
  if (!pos)
    return NULL;

  for (; pos; pos--)
    {
      if (doubly_next (doubly) == NULL)
        return NULL;
    }

  void *udata = doubly_curr (doubly);
  return udata;
}

doublydie_t *
droubly_move (doublydie_t *here, doublydie_t *from)
{
  if (!from)
    {
      memset (here, 0, sizeof (*here));
      return NULL;
    }

  if (from->cursor != from)
    here->cursor = from->cursor;
  else
    here->cursor = here;

  here->next = from->next;
  here->prev = from->prev;

  here->node_crc = from->node_crc;

  return from;
}

void *
doubly_drop (doublydie_t *doubly)
{
  doublydie_t *rm = doubly->cursor;
  doubly->cursor = NULL;

  if (!rm || rm == doubly)
    {
      void *user = doubly->node_data;
      doublydie_t *discard = droubly_move (doubly, doubly->next);
      if (!discard)
        apfree (discard);
      else
        memset (doubly, 0, sizeof (*doubly));

      return user;
    }

  void *user = rm->node_data;

  if (!rm->prev)
    memset (rm, 0, sizeof (*rm));
  else
    rm->prev->next = rm->next;

  if (rm->next)
    rm->next->prev = rm->prev;

  apfree (rm);
  return user;
}

void *
doubly_next (doublydie_t *doubly)
{
  if (!doubly)
    return NULL;
  volatile doublydie_t *cursor = doubly->cursor;

  // We're reached at end
  if (cursor == doubly)
    return NULL;

  if (cursor == NULL && doubly->node_data != NULL)
    {
      if (doubly->next)
        doubly->cursor = doubly->next;
      else
        doubly->cursor = doubly;
      return doubly->node_data;
    }

  if (cursor == NULL)
    return NULL;

  void *ddata = cursor->node_data;
  if (cursor->next)
    doubly->cursor = cursor->next;

  return ddata;
}

i32
doubly_insert (void *data, doublydie_t *doubly)
{
  doublydie_t *newnode = NULL;
  i32 pos = 0;

  if (doubly->node_data == NULL)
    {
      newnode = doubly;
      goto attribute;
    }

  while (doubly->next)
    {
      doubly = doubly->next;
      pos++;
    }

  newnode = (doublydie_t *)apmalloc (sizeof *doubly);
  newnode->next = newnode->prev = newnode->cursor = NULL;

attribute:
  if (newnode != doubly)
    {
      newnode->prev = doubly;
      doubly->next = newnode;
    }

  newnode->node_data = data;
  newnode->node_crc = cyclic32_checksum (newnode->node_data, sizeof (void *));

  return pos;
}

i32
doubly_deinit (doublydie_t *doubly)
{
  if (doubly == NULL)
    return -1;
  if (doubly->prev != NULL)
    return -1;

  i32 destroyed = 0;
  doubly = doubly->next;
  while (doubly)
    {
      doublydie_t *next = NULL;
      next = doubly->next;
      apfree (doubly);
      doubly = next;
      destroyed++;
    }

  return destroyed;
}

i32
doubly_remove (void *data, doublydie_t *doubly)
{
  if (doubly->node_data == data && doubly->prev == NULL)
    {
      doubly->node_data = NULL;
      // We're inside the root node, we can't deallocate it!
      if (doubly->next == NULL)
        {
          doubly->node_crc = 0;
          doubly->cursor = doubly->prev = NULL;
          return 0;
        }
      // Removing the next node
      doubly->node_data = doubly->next->node_data;
      doublydie_t *fast_delete = doubly->next;

      doubly->node_crc = doubly->next->node_crc;

      doubly->next->prev = doubly;
      doubly->cursor = doubly->next->cursor;
      doubly->next = doubly->next->next;
      apfree (fast_delete);
    }

  i32 pos = 0;
  while ((doubly = doubly->next))
    {
      if (doubly->node_data == data)
        break;
      pos++;
    }
  if (doubly == NULL)
    return -1;
  if (doubly->prev)
    doubly->prev->next = doubly->next;
  if (doubly->next)
    doubly->next->prev = doubly->prev;

  return pos;
}
