#include <malloc.h>

#include <memctrlext.h>
#include <echo/fmt.h>

#define MALLOC_MAX_LIMIT 1 << 7 // 128 bytes (if u will want more simplicity)
#define MALLOC_DEBUG 0

u64 explicit_align(u64 nsize, u64 alignment) {
	u64 misalignment = nsize % alignment;
	
	if (misalignment == 0) return nsize;
	
	return nsize + alignment - misalignment;
}

#if MALLOC_DEBUG
static inline __attribute__((always_inline)) void memctrl_report(u64 block_size, 
		void* new, const char* wrapper_name) {
	
	echo_debug(NULL, "New memory block of size %#04lu allocated at %#p, "
			"using %8s wrapper\n", block_size, new, wrapper_name);
}
#endif

void* apcalloc(u64 nele, u64 esize) {
	if (nele == 0  || esize == 0) {
		echo_warning(NULL, "Attempting to allocate a block with a zero expression "
				"evaluation, (nele %lu) or (esize %lu) can't be 0!", 
				nele, esize);
		return NULL;
	}
	
	echo_assert(NULL, esize % 2 == 0, "Attempting to allocate a non "
		"aligned pointer with calloc");
	
	void* new = calloc(nele, esize);
	#if MALLOC_DEBUG
	memctrl_report(nele * esize, new, "apcalloc");
	#endif

	return new;
}

void* apmalloc(u64 rsize) {
	if (rsize >= MALLOC_MAX_LIMIT) {
		#define AP_CALLOC_PARAM 1

		void* nptr = apcalloc(AP_CALLOC_PARAM, rsize);	
		return nptr;
	}

	if (rsize == 0) return NULL;
	// Memory isn't alignmented, this may lead to some errors
	echo_assert(NULL, rsize % 2 == 0, "Can't allocate a non aligned pointer");

	void* nptr = malloc(rsize);
	#if MALLOC_DEBUG
	memctrl_report(rsize, nptr, "apmalloc");
	#endif

	return nptr;
}

void apfree(void* endptr) {
	if (!endptr) return;
	
	#if MALLOC_DEBUG
	#if defined(__linux__)
	echo_debug(NULL, "Memory in %#p with %04lu being de-allocated\n", endptr, 
			malloc_usable_size(endptr));
	#else
	echo_debug(NULL, "Memory in %#p being de-allocated\n", endptr);
	#endif

	#endif
	free(endptr);
}


