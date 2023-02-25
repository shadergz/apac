#include <string.h>
#include <stdlib.h>

#include <storage/io_native.h>

native_flags_t native_solve_flags(const char* flags) {
	if (flags == NULL) 
		return 0;
	const char* final = strchr(flags, ':');
	
	if (final == NULL) 
		return 0;

	native_flags_t ret = 0;

	while (flags != final) switch (*flags++) {
	case 'd':
		if (ret & STORAGE_FLAG_FILE) return 0;
		ret |= STORAGE_FLAG_DIRECTORY;
	// This "read" flag is needed when open a directory, this behaviour must be conserved
	case 'r': ret |= STORAGE_FLAG_READ; break;
	case 'w': ret |= STORAGE_FLAG_WRITE; break;
	case 'f':
		if (ret & STORAGE_FLAG_DIRECTORY) return 0;
		ret |= STORAGE_FLAG_FILE;
		break;
	}

	return ret;
}

native_perms_t native_solve_perms(const char* perms) {
	const char* perm_cur = strpbrk(perms, ":");
	if (perm_cur == NULL) return 0;
	else perm_cur++;

	native_perms_t ret = 0;
	char perm_sequence[4] = {};
	strncpy(perm_sequence, perm_cur, 3);

	i32 octal = strtoul(perm_sequence, NULL, 8);
	if (octal == 0) return 0;
	if (octal & 0700) {
		ret |= STORAGE_PERM_USER_FULL;
		goto group;
	}
	if (octal & 0400) ret |= STORAGE_PERM_USER_READ;
	if (octal & 0200) ret |= STORAGE_PERM_USER_WRITE;
	if (octal & 0100) ret |= STORAGE_PERM_USER_EXEC;

	group:
	if (octal & 0070) {
		ret |= STORAGE_PERM_GROUP_FULL;
		goto others;
	}
	if (octal & 0040) ret |= STORAGE_PERM_GROUP_READ;
	if (octal & 0020) ret |= STORAGE_PERM_GROUP_WRITE;
	if (octal & 0010) ret |= STORAGE_PERM_GROUP_EXEC;
	others:
	if (octal & 0007) {
		ret |= STORAGE_PERM_OTHERS_FULL;
	}
	if (octal & 0004) ret |= STORAGE_PERM_OTHERS_READ;
	if (octal & 0002) ret |= STORAGE_PERM_OTHERS_WRITE;
	if (octal & 0001) ret |= STORAGE_PERM_OTHERS_EXEC;

	return ret;
}

