#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ucontext.h>

#include <rt.h>
#include <layer.h>
#include <memctrlext.h>
#include <trace.h>
#include <locker.h>

#include <dyn_loader.h>

#include <echo/fmt.h>

void run_raise(i32 rsig) {
	raise(rsig);
	__builtin_unreachable();
}

static void thread_segv_handler(int signum, siginfo_t* info, void* context) __attribute__((noreturn));


void install_handlers() {
	
	struct sigaction deadly_action;
	
	deadly_action.sa_sigaction = thread_segv_handler;
	sigemptyset(&deadly_action.sa_mask);

	deadly_action.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &deadly_action, NULL);
}

static i32 rt_dump(u64 idx, bool valid_trace, void* user_data, const char* format, ...) {
	va_list va;
	va_start(va, format);

	#define RT_BUFFER_SZ 0x100
	char rt_buffer[RT_BUFFER_SZ];
	i32 ret = 0;

	if (valid_trace == false && idx == 0) {
		ret = vsnprintf(rt_buffer, sizeof rt_buffer, 
			"\tBacktrace: from %s module, loaded at %#llx\n", va);
		goto rt_end;
	}

	ret = vsnprintf(rt_buffer, sizeof rt_buffer, format, va);

	rt_end:
	echo_error(NULL, "%s", rt_buffer);

	va_end(va);
	return ret;
}

static void thread_segv_handler(int signum, siginfo_t* info, void* context) {
	// Getting the context of the signal, including the registers state
	const ucontext_t* prog_context = (ucontext_t*)context;
	typedef long long unsigned int register_t;

	echo_error(NULL, "\033[0;31mReceived SIGSEGV signal ");
	
	#if defined(__x86_64__)
	echo_error(NULL, "(x86_64)\033[0m\n");

	static const char* registers_name[] = {
		"RAX", "RBX", "RCX", "RDX", 
		"RSI", "RDI", "RBP", "RSP"};

	#define X86_PC_LOCATION 1 << 4
	const register_t PC = prog_context->uc_mcontext.gregs[X86_PC_LOCATION];
	const register_t* readable_regs = (const register_t*)
		prog_context->uc_mcontext.gregs;

	#elif defined(__aarch64__)
	
	echo_error(NULL, "(aarch64)\033[0m\n");
	static const char* registers_name[] = {"X0", "X1", "X2", "X3"};

	const register_t PC = prog_context->uc_mcontext.pc;
	const register_t* readable_regs = (const register_t*)
		prog_context->uc_mcontext.regs;
	#endif

	dyninfo_t retr = {};
	
	const char* possible = dyn_getsymbolname((void*)PC, &retr);
	if (possible != NULL) {
		echo_error(NULL, "May the function *%s* as caused this error!\n", possible);
	}	
	
	echo_error(NULL, "\033[0;32mRegister states:\033[0m\n");
	for (u8 reg_index = 0; reg_index < sizeof(registers_name)/sizeof(const char*); reg_index++) {
		echo_error(NULL, "\033[0;38m%s = \033[0;32m%016llx\033[0m\n", registers_name[reg_index],
			readable_regs[reg_index]);
	}

	echo_error(NULL, "\033[0;35mProgram Counter = \033[0;37m%016llx\033[0m\n", PC);

	echo_error(NULL, "\033[0;33mStack trace:\033[0m\n");

	#define BACK_MAX_LEVEL 5

	void* back_array[BACK_MAX_LEVEL];
	u64 trace_hooked = trace_capture(back_array, BACK_MAX_LEVEL);
	trace_dump(back_array, NULL, rt_dump, trace_hooked);

	locker_release(NULL);

	// I regret nothing... 
	exit(-1);
	__builtin_unreachable();
}

void run_raisef(i32 rsig, const char* fmt, ...) {
	echo_error(NULL, "Program was halted");
	if (!fmt) {
		echo_error(NULL, "\n");
		goto raise;
	}
	
	echo_error(NULL, " because : ");
	va_list rargs;
	va_start(rargs, fmt);

	#define RAISE_BF_SIZE 0x4f
	char raise_bf[RAISE_BF_SIZE];

	vsnprintf(raise_bf, sizeof raise_bf, fmt, rargs);
	va_end(rargs);

	echo_error(NULL, "%s", raise_bf);

	raise:
	raise(rsig);
	__builtin_unreachable();
}

i32 run_getedir(char** save_path, u64 req_sz) {
	
	char* read_path = NULL;
	layer_asprintf(&read_path, "/proc/%d/exe", getpid());

	if (save_path == NULL) return -1;
	if (read_path == NULL || *save_path != NULL) return -1;
	*save_path = (char*)apmalloc(req_sz);

	if (*save_path == NULL) {
		apfree(read_path);
		return -1;
	}

	const i32 re = readlink(read_path, *save_path, req_sz - 1);

	(*save_path)[re] = '\0';
	char* dirlimits = strrchr(*save_path, '/');
	
	if (dirlimits == NULL) {
		apfree(read_path);
		return -1;
	}
	
	*dirlimits = '\0';

	apfree(read_path);
	return re;
}

