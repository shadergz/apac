#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <layer.h>

#include <memctrlext.h>

i32 layer_vasprintf(char** restrict lstrp, const char* restrict lfmt, va_list lap) {
	va_list copy;

	va_copy(copy, lap);
	i32 needed = vsnprintf(NULL, 0, lfmt, copy);
	va_end(copy);

	if (needed < 0) return needed;

	/* This will force who is using this function to garante that what lstrp is pointer to,
	 * is null (or invalid) pointer, this may avoid some errors like override a valid 
	 * function pointer inside a loop
	*/
	if (lstrp == NULL) return -1;
	if (*lstrp != NULL) return -1;

	*lstrp = apmalloc(explicit_align(++needed, 4));
	if (lstrp == NULL) return -1;

	return vsprintf(*lstrp, lfmt, lap);
}

i32 layer_asprintf(char** restrict lstrp, const char* restrict lfmt, ...) {
	va_list sp;
	va_start(sp, lfmt);
	
	const i32 result = layer_vasprintf(lstrp, lfmt, sp);

	va_end(sp);
	return result;
}

