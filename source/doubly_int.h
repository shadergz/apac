#ifndef APAC_DOUBLY_INT_H
#define APAC_DOUBLY_INT_H

#include <api.h>

i32 doubly_init (doublydie_t *doubly);
i32 doubly_insert (void *data, doublydie_t *doubly);

u64 doubly_size (const doublydie_t *doubly);

i32 doubly_deinit (doublydie_t *doubly);
i32 doubly_remove (void *data, doublydie_t *doubly);

void *doubly_get (u64 pos, doublydie_t *doubly);

void *doubly_curr (const doublydie_t *doubly);

i32 doubly_reset (doublydie_t *doubly);
void *doubly_next (doublydie_t *doubly);

void *doubly_drop (doublydie_t *doubly);

#endif
