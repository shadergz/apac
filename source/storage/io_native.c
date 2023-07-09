#include <stdlib.h>
#include <string.h>

#include <storage/io_native.h>

native_flags_t
native_solve_flags(const char* flags)
{
    if (flags == NULL)
        return 0;
    const char* final = strchr(flags, ':');

    if (final == NULL)
        return 0;

    native_flags_t ret = 0;
    bool dir = false, file = false, read = false, write = false, create = false;

    while (flags != final)
        switch (*flags++) {
        case 'd':
            dir = true;
            break;
        /* This "read" flag is needed when open a directory, this behaviour must
         * be conserved! */
        case 'r':
            read = true;
            break;
        case 'w':
            write = true;
            break;
        case 'f':
            file = true;
            break;
        case 'c':
            create = true;
            break;
        }

    if (dir && file)
        return 0;

    if (read && write)
        ret |= STORAGE_FLAG_RW;
    if (!read && write)
        ret |= STORAGE_FLAG_WRITE;

    if (file)
        ret |= STORAGE_FLAG_FILE;
    if (dir)
        ret |= STORAGE_FLAG_DIRECTORY;

    if (create)
        ret |= STORAGE_FLAG_CREAT;

    return ret;
}

static i32
native_solver(char* solver)
{
    char* hiphen = strchr(solver, '-');
    if (hiphen == NULL)
        return -1;

    u64 location = 3 - (u64)(hiphen - solver);
    if (!location)
        return -1;
    while (location-- != 0)
        *hiphen++ = '0';

    return 0;
}

native_perms_t
native_default_perms(storage_node_id_e node_type)
{
    if (node_type & STORAGE_FLAG_FILE) {
        // 0644
        return STORAGE_PERM_USER_READ | STORAGE_PERM_USER_WRITE
            | STORAGE_PERM_GROUP_READ | STORAGE_PERM_OTHERS_READ;
    }

    return 0;
}

native_perms_t
native_solve_perms(const char* perms)
{
    const char* perm_cur = strpbrk(perms, ":");
    if (perm_cur == NULL)
        return 0;
    else
        perm_cur++;

    native_perms_t ret = 0;
    char perm_sequence[4] = {};

    strncpy(perm_sequence, perm_cur, 3);
    native_solver(perm_sequence);

    if (*perm_sequence == '\0' || *perm_sequence == '0')
        return 0;

    i32 octal = strtoul(perm_sequence, NULL, 8);
    if (octal == 0)
        return 0;
    if (octal & 0700) {
        ret |= STORAGE_PERM_USER_FULL;
        goto group;
    }
    if (octal & 0400)
        ret |= STORAGE_PERM_USER_READ;
    if (octal & 0200)
        ret |= STORAGE_PERM_USER_WRITE;
    if (octal & 0100)
        ret |= STORAGE_PERM_USER_EXEC;

group:
    if (octal & 0070) {
        ret |= STORAGE_PERM_GROUP_FULL;
        goto others;
    }
    if (octal & 0040)
        ret |= STORAGE_PERM_GROUP_READ;
    if (octal & 0020)
        ret |= STORAGE_PERM_GROUP_WRITE;
    if (octal & 0010)
        ret |= STORAGE_PERM_GROUP_EXEC;
others:
    if (octal & 0007) {
        ret |= STORAGE_PERM_OTHERS_FULL;
    }
    if (octal & 0004)
        ret |= STORAGE_PERM_OTHERS_READ;
    if (octal & 0002)
        ret |= STORAGE_PERM_OTHERS_WRITE;
    if (octal & 0001)
        ret |= STORAGE_PERM_OTHERS_EXEC;

    return ret;
}
