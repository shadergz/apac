#include <pthread.h>
#include <unistd.h>

#include <sched/gov.h>
#include <sched/spin_lock.h>

#include <echo/fmt.h>
#include <thread/sleep.h>

#include <debug_extra.h>
#include <strplus.h>

schedgov_t* g_thread_gov = NULL;

void worker_killsig(i32 thrsig)
{
    echo_success(NULL, "Thread %lu is being killed...\n", pthread_self());

    if (g_thread_gov)
        // Release all locks
        spin_rtryunlock(&g_thread_gov->mutex);

    pthread_exit(NULL);
}

typedef struct sigaction native_sigaction_t;

static i32
sched_installsig(schedgov_t* governor)
{
#if defined(__ANDROID__)
    static native_sigaction_t action
        = { .sa_handler = worker_killsig, .sa_flags = 0 };
#else
    static native_sigaction_t action
        = { .sa_handler = worker_killsig, .sa_flags = SA_INTERRUPT };
#endif
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, (const sigset_t*)&action.sa_mask,
        &governor->thread_dflt);

    sigaction(SIGUSR1, (const native_sigaction_t*)&action, NULL);

    return 0;
}

static const char*
worker_doname(schedthread_t* self, char thbuffer[], u64 thsize)
{
    char tname[0x30];
    const char* color = NULL;
    switch (self->thread_color) {
    case NATURAL_COLOR_GREEN:
        color = "\e[1;42m";
        break;
    case NATURAL_COLOR_RED:
        color = "\e[1;41m";
        break;
    case NATURAL_COLOR_YELLOW:
        color = "\e[1;43m";
        break;
    case NATURAL_COLOR_CYAN:
        color = "\e[1;46m";
        break;
    default:
        color = "";
    }

    snprintf(tname, sizeof tname, "%s%s\e[0m", color, self->thread_name);

    strplus_padding(thbuffer, tname, 20, '*', PADDING_MODE_END);

    return thbuffer;
}

void* worker_entry(void* apac_ptr)
{
    apac_ctx_t* apac_ctx = (apac_ctx_t*)apac_ptr;
    if (!apac_ctx)
        pthread_exit(NULL);

    g_thread_gov = apac_ctx->governor;
    sched_installsig(g_thread_gov);

    spin_rlock(&g_thread_gov->mutex);
    DEBUG_DUMP_STRUCT(g_thread_gov->mutex);

    schedthread_t* self = sched_find(0, apac_ctx);
    sched_configure(self, apac_ctx);

    echo_assert(apac_ctx, self->executing == 0, "Thread is already executing!");
    self->executing = true;

    // We can change the thread name more than once inside contexts
    sched_setname(self->thread_name, apac_ctx);
#define THNAME_BSZ 30
    char thname[THNAME_BSZ] = {};

    echo_success(
        apac_ctx, "Thread (%s) with id %lu was started [\e[0;32mON\e[0m]\n",
        worker_doname(self, thname, sizeof thname), self->thread_handler);

    g_thread_gov->threads_count++;
    spin_runlock(&g_thread_gov->mutex);
    for (;;)
        thread_sleepby(100, THREAD_SLEEPCONV_SECONDS);
    return NULL;
}
