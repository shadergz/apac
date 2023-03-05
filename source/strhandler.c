#include <string.h>

#include <strhandler.h>

const char* strhandler_skip(const char* str, const char* skip) {
	if (!str || !skip) return NULL;

	const char* next = strstr(str, skip);
	if (!next) return NULL;

	const char* ok = next + strlen(skip);
	
	return ok;
}
