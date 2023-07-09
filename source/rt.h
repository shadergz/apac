#ifndef APAC_RT_H
#define APAC_RT_H

#include <api.h>
#include <signal.h>

void run_raise(i32 rsig) __attribute__((noreturn));
void run_raisef(i32 rsig, const char* fmt, ...) __attribute__((noreturn));

i32 run_getedir(char** save_path, u64 req_sz);

void install_handlers();

#endif
