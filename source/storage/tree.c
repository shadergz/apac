#include <string.h>

#include <memctrlext.h>
#include <storage/dirio.h>
#include <storage/tree.h>

#include <doubly_int.h>

u32 tree_makeroot(const char* rootpath, apac_ctx_t* apac_ctx) {
	if (rootpath == NULL) return -1;

	storage_tree_t* root = apac_ctx->root; 

	memset(root, 0, sizeof(storage_tree_t));
	root->node_level = 0;

	storage_dirio_t* dir = apmalloc(sizeof(storage_dirio_t));
	root->leafs = apmalloc(sizeof(doublydie_t));
	doubly_init(root->leafs);

	tree_open_dir(dir, rootpath, root);
	return (u32)root->node_level;
}

storage_tree_t* tree_solve_rel(const char* rpath, storage_tree_t* root) {
	if (root->node_id == 0) return NULL;
	if (*rpath == '/' && *(rpath + 1) == '\0' ) return root;
	return NULL;
}

// Fetch a tree structure from user real fs path
storage_tree_t* tree_from_userdir(const char* user, storage_tree_t* root) {
	return root;
}

i32 tree_fetch_rel(storage_tree_t* here, char* out, u64 out_size, storage_tree_t* root) {
	if (out_size < 2) return -1;
	if (root == here) {
		*out++ = '/';
		*out = '\0';
		return 1;
	}
	return -1;
}

static i32 tree_expand(storage_dirio_t* expand, storage_tree_t* here) {
	u8 cont_info[64];	
	dirio_read(cont_info, sizeof cont_info, expand);
	return 0;
}

i32 tree_open_dir(storage_dirio_t* place, const char* user_path, storage_tree_t* root) {
	storage_tree_t* dir_put = tree_from_userdir(user_path, root);
	if (dir_put == NULL) return -1;

	/* Runtime execution directory is used as the root of our system, everything that
	 * performs I/O operations will use this root directory as a requirement! */
	i32 dirio = dirio_open(user_path, "d:700-", place);
	if (dirio != 0) return dirio;
	
	dirio = tree_attach_dir(dir_put, place);
	if (dirio != 0) return dirio;

	char rel_path[TREE_PATH_MAX_REL];
	tree_fetch_rel(dir_put, rel_path, sizeof(rel_path), root);

	place->dir_relative = strdup(rel_path);

	tree_expand(place, root);

	return 0;
}

i32 tree_deattach(storage_tree_t* node) {
	if (node->parent == NULL) return -1;
	return 0;
}

i32 tree_attach_dir(storage_tree_t* with, storage_dirio_t* dir) {
	if (__builtin_expect((with == NULL), 0)) {
		return -1;
	}
	with->node_id = SOLID_NODE_ID_DIR;
	with->node_dir = dir;

	return 0;
}

i32 tree_close(storage_tree_t* collapse, bool force) {
	// We can't continue if node is the root and `force` isn't valid!
	if ((collapse->node_level == 0 && !force)) return -1;
	
	if (collapse->node_id == SOLID_NODE_ID_DIR) {
		dirio_close(collapse->node_dir);

		apfree((char*)collapse->node_dir->dir_relative);
		apfree(collapse->node_dir);
	}

	doubly_deinit(collapse->leafs);
	apfree(collapse->leafs);
	return 0;
}

i32 tree_collapse(apac_ctx_t* apac_ctx) {
	/* All files that was opened will be closed and some warning 
	 * maybe be dropped */
	storage_tree_t* root = apac_ctx->root;

	const i32 hio = tree_close(root, true);
	return hio;
}
