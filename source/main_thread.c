#include <stdio.h>

#include <api.h>
#include <memctrlext.h>
#include <session.h>
#include <rt.h>

#define WRAPPER_TYPE_TO_STR(type) #type
#define TYPE_2_STR(type)\
	WRAPPER_TYPE_TO_STR(typeof(type))

static i32 apac_san(apac_ctx_t* apac_ctx) {
	if (apac_ctx->user_options == NULL) {
		return -1;
	}
	return 0;
}

static i32 apac_init(apac_ctx_t* apac_ctx) {

	apac_ctx->user_options  = (user_options_t*)apmalloc(sizeof(user_options_t));
	apac_ctx->user_session  = (session_ctx_t*) apmalloc(sizeof(session_ctx_t));
	apac_ctx->echo_system   = (echo_ctx_t*)    apmalloc(sizeof(echo_ctx_t));

	apac_ctx->core_backend  = (backend_ctx_t*) apmalloc(sizeof(backend_ctx_t));
	apac_ctx->governor      = (schedgov_t*)    apmalloc(sizeof(schedgov_t));
	apac_ctx->root          = (storage_tree_t*)apmalloc(sizeof(storage_tree_t));

	apac_san(apac_ctx);
	return 0;
}

static i32 apac_deinit(apac_ctx_t* apac_ctx) {
	if (apac_ctx->user_options != NULL) apfree(apac_ctx->user_options);
	if (apac_ctx->user_session != NULL) apfree(apac_ctx->user_session);
	if (apac_ctx->echo_system != NULL)  apfree(apac_ctx->echo_system);

	if (apac_ctx->core_backend != NULL) apfree(apac_ctx->core_backend);
	if (apac_ctx->governor != NULL)     apfree(apac_ctx->governor);

	if (apac_ctx->root != NULL)         apfree(apac_ctx->root);

	return 0;
}

i32 main(i32 argc, char** argv) {
	install_handlers(); 

	apac_ctx_t* apac_main = (apac_ctx_t*)apmalloc(sizeof(apac_ctx_t));
       	
	apac_init(apac_main);
	session_init(argc, argv, apac_main);
	
	puts("Hello World my nobre!");

	session_deinit(apac_main);
	apac_deinit(apac_main);

	apfree(apac_main);
	return 0;
}

