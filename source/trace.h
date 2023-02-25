#ifndef APAC_TRACE_H
#define APAC_TRACE_H

#include <api.h>

u64 trace_capture(void* save_frame[], u64 req_frame); 

typedef i32 (*traceprint_t)(u64 idx, bool valid_trace, void* user_data, const char* format, ...);

u64 trace_dump(void* captured_frame[], void* user, traceprint_t print_trace, u64 dsp_count);

#endif

