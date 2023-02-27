#include <string.h>

#include <memctrlext.h>
#include <storage/dirio.h>

#include <storage/tree.h>
#include <storage/fio.h>

#include <echo/fmt.h>

#include <doubly_int.h>

u32 tree_makeroot(const char* rootpath, apac_ctx_t* apac_ctx) {
	if (rootpath == NULL) return -1;

	storage_tree_t* root = apac_ctx->root; 

	memset(root, 0, sizeof(storage_tree_t));
	root->node_level = 0;

	storage_dirio_t* dir = apmalloc(sizeof(storage_dirio_t));
	root->leafs = apmalloc(sizeof(doublydie_t));
	doubly_init(root->leafs);

	tree_open_dir(dir, rootpath, apac_ctx);
	return (u32)root->node_level;
}

storage_tree_t* tree_solve_rel(const char* rpath, storage_tree_t* root) {
	if (root->node_id == 0) return NULL;
	if (*rpath == '/' && *(rpath + 1) == '\0' ) return root;
	return NULL;
}

// Fetch a tree structure from user real fs path
storage_tree_t* tree_getfromuser(const char* user, storage_tree_t* root) {
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

i32 tree_open_dir(storage_dirio_t* place, const char* user_path, apac_ctx_t* apac_ctx) {

	storage_tree_t* root = apac_ctx->root;
	storage_tree_t* dir_put = tree_getfromuser(user_path, root);
	if (dir_put == NULL) return -1;

	/* Runtime execution directory is used as the root of our system, everything that
	 * performs I/O operations will use this root directory as a requirement! */
	i32 dirio = dirio_open(user_path, "d:700-", place);
	if (dirio != 0) {
		echo_error(apac_ctx, "Can't open a directory (%s) inside the tree "
			"(%s)", user_path, dirio_getname(dir_put->node_dir));	
		return dirio;
	}
	
	dirio = tree_attach_dir(dir_put, place);
	if (dirio != 0) return dirio;

	char rel_path[TREE_PATH_MAX_REL];
	tree_fetch_rel(dir_put, rel_path, sizeof(rel_path), root);

	place->dir_relative = strdup(rel_path);

	return 0;
}

storage_fio_t* tree_getfile(const char* filename, apac_ctx_t* apac_ctx) {
	if (filename == NULL || apac_ctx == NULL) return NULL;
	#define TREE_FILE_MATCH(file, name)\
		if ((strncmp(file->file_name, name, strlen(file->file_name)) == 0))

	storage_tree_t* dirlocal = tree_getfromuser(filename, apac_ctx->root);

	doubly_reset(dirlocal->leafs);

	for (storage_fio_t* cursor = NULL; 
		(cursor = doubly_next(dirlocal->leafs)) != NULL; ) {		
	
		TREE_FILE_MATCH(cursor, filename) return cursor;
	}

	doubly_reset(dirlocal->leafs);
	return NULL;
}

i32 tree_open_file(storage_fio_t* file, const char* path, const char* perm, apac_ctx_t* apac_ctx) {
	storage_tree_t* root = apac_ctx->root;
	storage_tree_t* dir_put = tree_getfromuser(path, root);
	if (dir_put == NULL) return -1;

	i32 fio_ret = fio_open(path, perm, file);
	if (fio_ret != 0) {
		echo_error(apac_ctx, "Can't open a file with pathname %s", path);
		return fio_ret;
	}
	
	fio_ret = tree_attach_file(dir_put, file);
	if (fio_ret != 0)
		return fio_ret;

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
	with->node_id = STORAGE_NODE_ID_DIR;
	with->node_dir = dir;

	return 0;
}

i32 tree_attach_file(storage_tree_t* with, storage_fio_t* file) {
	if (with->node_dir || with->node_file) {
		storage_tree_t* file_fs = (storage_tree_t*)apmalloc(sizeof(*with));
		if (file_fs == NULL) return -1;

		file_fs->parent = with;
		file_fs->node_file = file;
		file_fs->node_id = STORAGE_NODE_ID_FILE;

		const i32 ret = doubly_insert(file_fs, with->leafs);
		return ret;
	}

	with->node_id = STORAGE_NODE_ID_FILE;
	with->node_file = file;
	return 0;
}

i32 tree_close(storage_tree_t* collapse, bool force) {
	// We can't continue if node is the root and `force` isn't valid!
	if ((collapse->node_level == 0 && !force)) return -1;
	
	if (collapse->node_id == STORAGE_NODE_ID_DIR) {
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
