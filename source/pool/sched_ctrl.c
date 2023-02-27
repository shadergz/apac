#include <stddef.h>
#include <string.h>

#include <pthread.h>
#include <sys/prctl.h>

#include <pool/gov.h>

#include <cyclic_hw.h>
#include <memctrlext.h>
#include <vec.h>

i32 sched_unsetname(apac_ctx_t* apac_ctx) {
	schedthread_t* thinfo = sched_find(pthread_self(), apac_ctx);
	if (thinfo == NULL) return -1;

	if (thinfo->thread_name == NULL) return 0;

	prctl(PR_SET_NAME, "Undefined", 0, 0, 0);
	apfree((char*)thinfo->thread_name);

	return 0;
}

i32 sched_setname(const char* message, apac_ctx_t* apac_ctx) {
	schedgov_t* gov = apac_ctx->governor;
	if (!gov || !message) 
		return -1;

	u64 thread = pthread_self();

	/* Avoiding ERANGE problems, because thread's name can only have 16 or less
	 * bytes as a C string (including null byte) */
	if (strlen(message) > 15) 
		return -1;

	schedthread_t* thinfo = sched_find(thread, apac_ctx);
	if (thinfo == NULL) return -1;

	sched_unsetname(apac_ctx);
	thinfo->thread_name = strdup(message);

	prctl(PR_SET_NAME, thinfo->thread_name, 0, 0, 0);
	return 0;
}

u8 sched_getcount(const apac_ctx_t* apac_ctx) {
	const schedgov_t* gov = apac_ctx->governor;
	if (gov == NULL)		  return 0;
	if (gov->thread_info_vec == NULL) return 0;
	return vec_using(gov->thread_info_vec);
}

i32 sched_init(apac_ctx_t* apac_ctx) {
	schedgov_t* gov = apac_ctx->governor;
	if (gov == NULL) return -1;

	gov->thread_info_vec = apmalloc(sizeof(vecdie_t));

	#define SCHED_DEF_THREAD_CNT 8

	const i32 vec_ret = vec_init(sizeof(schedthread_t), SCHED_DEF_THREAD_CNT, gov->thread_info_vec);

	if (vec_ret == 0 && (sched_configure(0, apac_ctx) != NULL)) {
		sched_setname("Apac Core", apac_ctx);
		return 0;
	}
	
	vec_deinit(gov->thread_info_vec);
	apfree(gov->thread_info_vec);

	gov->thread_info_vec = NULL;

	return -1;
}

schedthread_t* sched_find(u32 thread, apac_ctx_t* apac_ctx) {
	const schedgov_t* gov = apac_ctx->governor;

	if (thread == 0)
		thread = pthread_self();
	const u32 search_id = cyclic32_checksum(&thread, sizeof(u32));

	for (schedthread_t* tinfo = NULL; 
		(tinfo = vec_next(gov->thread_info_vec)) != NULL;) {

		if (tinfo->thread_id != search_id) continue;

		vec_reset(gov->thread_info_vec);
		return tinfo;
	}

	vec_reset(gov->thread_info_vec);
	return NULL;
}

schedthread_t* sched_configure(u32 thread, apac_ctx_t* apac_ctx) {
	schedgov_t* gov = apac_ctx->governor;

	schedthread_t* thinfo = sched_find(thread, apac_ctx);
	if (__builtin_expect((thinfo != NULL), 1)) return thinfo;
	
	thinfo = (schedthread_t*)vec_emplace(gov->thread_info_vec);
	
	if (thread == 0)
		thread = pthread_self();
	
	thinfo->thread_id = cyclic32_checksum(&thread, sizeof(i32));

	return thinfo;
}

i32 sched_deinit(apac_ctx_t* apac_ctx) {
	schedgov_t* gov = apac_ctx->governor;
	
	if (gov == NULL) return -1;

	if (!gov->thread_info_vec) return 0;

	schedthread_t* thinfo = vec_next(gov->thread_info_vec);
	vec_reset(gov->thread_info_vec);

	if (thinfo != NULL)
		sched_unsetname(apac_ctx);

	vec_deinit(gov->thread_info_vec);
	apfree(gov->thread_info_vec);

	return 0;
}


