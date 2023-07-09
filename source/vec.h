#ifndef APAC_VEC_H
#define APAC_VEC_H

#include <api.h>

i32 vec_init(u64 item_size, u64 initial_capa, vecdie_t* vec);
i32 vec_push(void* new_item, vecdie_t* vec);

void* vec_emplace(vecdie_t* vec);

void* vec_next(vecdie_t* vec);
void vec_reset(vecdie_t* vec);

i32 vec_pop(vecdie_t* vec);

u64 vec_capacity(const vecdie_t* vec);

u64 vec_using(const vecdie_t* vec);

const void* vec_ptr(const vecdie_t* vec);

i32 vec_deinit(vecdie_t* vec);
i32 vec_resize(u64 new_capa, vecdie_t* vec);

#endif
