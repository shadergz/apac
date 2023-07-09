#ifndef APAC_THREAD_SLEEP_H
#define APAC_THREAD_SLEEP_H

#include <api.h>

typedef enum thread_sleepconv {
    THREAD_SLEEPCONV_SECONDS,
    THREAD_SLEEPCONV_MILI,
    THREAD_SLEEPCONV_NANO
} thread_sleepconv_e;

i32 thread_sleepby(u64 by, thread_sleepconv_e conv);

#endif
