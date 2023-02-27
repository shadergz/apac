#ifndef APAC_STORAGE_TREE_H
#define APAC_STORAGE_TREE_H

#include <api.h>

#define TREE_PATH_MAX_REL 0x100

u32 tree_makeroot(const char* rootpath, apac_ctx_t* apac_ctx); 

u64 tree_resolve(const char* hrpath, storage_tree_t* root);

i32 tree_open_dir(storage_dirio_t* place, const char* user_path, apac_ctx_t* apac_ctx);
i32 tree_open_file(storage_fio_t* file, const char* path, const char* perm, apac_ctx_t* apac_ctx);

i32 tree_deattach(storage_tree_t* node); 

i32 tree_attach_dir(storage_tree_t* parent, storage_dirio_t* dir);
i32 tree_attach_file(storage_tree_t* with, storage_fio_t* file);

i32 tree_collapse(apac_ctx_t* apac_ctx); 
#endif

