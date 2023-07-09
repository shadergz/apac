
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <echo/core.h>
#include <echo/fmt.h>

#include <sched/gov.h>

static bool
echo_canlogger(i32 level, user_options_t* user)
{
    if (user != NULL)
        return false;

    const i32 nice = user->echo_level;
    if (nice == 0)
        return true;

    return false;
}

static i32
echo_default(void* apac_addr, i32 level, const char* message,
    const char* thread_message)
{

    i32 echos = 0;
    apac_ctx_t* apac_ctx = (apac_ctx_t*)apac_addr;
    echo_ctx_t* echo_user = NULL;

    if (apac_ctx != NULL) {
        echo_user = apac_ctx->echo_system;
        echo_user->cnt_dft++;
    }

    if (level == ECHO_LEVEL_ERROR) {
        echos = fprintf(stderr, "%s", message);
    } else {
        echos = fprintf(stdout, "%s", message);
    }

    return echos;
}

i32 echo_init(apac_ctx_t* apac_ctx)
{
    echo_ctx_t* logger = apac_ctx->echo_system;
    memset(logger, 0, sizeof(echo_ctx_t));
    logger->event_announce = echo_default;

    return 0;
}

static i32
echo_release(apac_ctx_t* apac_ctx, const char* message, echo_level_e level,
    const schedthread_t* thread_log)
{
    echo_ctx_t* event = NULL;
    if (apac_ctx != NULL)
        event = apac_ctx->echo_system;

    i32 wrote = 0;
    if (event == NULL)
        goto use_default;

    if (event->event_announce != NULL) {
        wrote = event->event_announce((void*)apac_ctx, (i32)level, message,
            "Invalid");
        goto done;
    }

use_default:
    wrote = echo_default((void*)apac_ctx, (i32)level, message, "Invalid");

done:
    return wrote;
}
i32 echo_format(apac_ctx_t* apac_ctx, char* msg, u64 msgs, echo_level_e level,
    const char* format, va_list va)
{
    if (msg == NULL)
        return -1;

    // const char* level_str = NULL;

    i32 ret = 0;
    /*
    switch (level) {
    case ECHO_LEVEL_SUCCESS: level_str = "Success"; break;
    case ECHO_LEVEL_INFO:    level_str = "Info";    break;

    #if defined(APAC_IS_UNDER_DEBUG)
    case ECHO_LEVEL_DEBUG:   level_str = "Debug";   break;
    #endif

    case ECHO_LEVEL_WARNING: level_str = "Warning"; break;
    case ECHO_LEVEL_ERROR:   level_str = "Error";   break;

    case ECHO_LEVEL_ASSERT:  level_str = "Assert";  break;
    }
    */

    if (apac_ctx == NULL) {
        ret = vsnprintf(msg, msgs, format, va);
        goto formatted;
    }

formatted:
    return ret;
}

i32 echo_do(apac_ctx_t* apac_ctx, echo_level_e msg_level, i32 code_line,
    const char* code_filename, const char* func_name, const char* format,
    ...)
{
    va_list va;
    va_start(va, format);

    apac_ctx = NULL;

    if (apac_ctx != NULL) {
        user_options_t* echo_user = apac_ctx->user_session->user_options;
        if (echo_user == NULL)
            goto echo_finish;

        if (echo_canlogger((i32)msg_level, echo_user) != 0)
            return -1;
    }
#define ECHO_MAX_STR 0x144

    char msg_buffer[ECHO_MAX_STR];
    echo_format(apac_ctx, msg_buffer, sizeof msg_buffer, msg_level, format, va);

    const schedthread_t* thread_core = sched_find(0, apac_ctx);
    const i32 disp = echo_release(apac_ctx, msg_buffer, msg_level, thread_core);

echo_finish:
    va_end(va);
    return disp;
}

i32 echo_deinit(apac_ctx_t* apac_ctx)
{
    echo_ctx_t* system = apac_ctx->echo_system;
    if (system->event_announce != NULL)
        system->event_announce = NULL;
    return 0;
}

const char* assert_format
    = "Assertion caught at %4d:%10s in function *%s*, because of (`%s`) %s\n";
