#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <echo/fmt.h>
#include <echo/core.h>

static void console_dump(const char* msg, const char* thread) {
	fprintf(stdout, "%s:%s", msg, thread);
}

i32 echo_init(apac_ctx_t* apac_ctx) {
	echo_ctx_t* logger = apac_ctx->echo_system;
	memset(logger, 0, sizeof(echo_ctx_t));
	logger->event_announce = console_dump;

	return 0;
}


i32 echo_dispatcher(apac_ctx_t* apac_ctx, echo_level_e level, const char* message) {
	fprintf(stderr, "%s", message);
	return 0;
}
i32 echo_release() {
	return 0;
}
i32 echo_format(apac_ctx_t* apac_ctx, char* msg, u64 msgs, echo_level_e level, 
		const char* format, va_list va) {
	if (msg == NULL) return -1;

	const char* level_str = NULL;
	char level_id = '\0';

	i32 ret;
	switch (level) {
	case ECHO_LEVEL_SUCCESS: level_str = "Success"; level_id = 'S'; break;
	case ECHO_LEVEL_INFO:    level_str = "Info";    level_id = 'I'; break;
	
	#if defined(APAC_IS_UNDER_DEBUG)
	case ECHO_LEVEL_DEBUG:   level_str = "Debug";   level_id = 'D'; break;
	#endif
	
	case ECHO_LEVEL_WARNING: level_str = "Warning"; level_id = 'W'; break;
	case ECHO_LEVEL_ERROR:   level_str = "Error";   level_id = 'E'; break;
	
	case ECHO_LEVEL_ASSERT:  level_str = "Assert";  level_id = 'A'; break;
	}

	(void)level_str;

	if (apac_ctx == NULL) {
		if (level_id == '\0') return -1;

		ret = vsnprintf(msg, msgs, format, va);
	}

	return ret;
}

i32 echo_do(apac_ctx_t* apac_ctx, echo_level_e msg_level, i32 code_line, const char* code_filename, 
		const char* func_name, const char* format, ...) {
	va_list va;
	va_start(va, format);

	if (apac_ctx != NULL) {
		goto echo_finish;
	}

	#define ECHO_MAX_STR 0x144

	char msg_buffer[ECHO_MAX_STR];
	echo_format(apac_ctx, msg_buffer, sizeof msg_buffer, msg_level, format, va);

	const i32 disp = echo_dispatcher(apac_ctx, msg_level, msg_buffer);

	echo_finish:
	va_end(va);
	return disp;
}

i32 echo_deinit(apac_ctx_t* apac_ctx) {
	echo_ctx_t* system = apac_ctx->echo_system;
	if (system->event_announce == console_dump)
		system->event_announce = NULL;
	return 0;
}

const char* assert_format = "Assertion caught at %4d:%10s in function *%s*, because (`%s`) %s\n";

