#include <stdio.h>

#include <dyn_loader.h>
#include <trace.h>

#if defined(__ANDROID__)

#include <unwind.h>

struct traceback_state
{
  void **curr_state;
  void **end_ptr;
};

static _Unwind_Reason_Code
un_store_holder (struct _Unwind_Context *context, void *arg_data)
{
  struct traceback_state *frame = (struct traceback_state *)arg_data;
  u64 program_counter = _Unwind_GetIP (context);
  if (program_counter == 0)
    goto fish_next;

  if (frame->curr_state == frame->end_ptr)
    {
      // We don't have no more memory for holder a possible new function inside
      // our stack frame
      return _URC_END_OF_STACK;
    }

  *frame->curr_state++ = (void *)program_counter;

fish_next:
  return _URC_NO_REASON;
}

#elif defined(__linux__)

#include <execinfo.h>

#endif

u64
trace_capture (void *save_frame[], u64 req_frame)
{
  u64 back_ret;
#if defined(__ANDROID__)
  struct traceback_state frame_state
      = { save_frame, save_frame + (req_frame - 1) };
  _Unwind_Backtrace (un_store_holder, &frame_state);

  back_ret = frame_state.end_ptr - frame_state.curr_state;
#else
  back_ret = backtrace (save_frame, req_frame);

#endif
  if (back_ret != req_frame)
    return 0;

  return back_ret;
}

u64
trace_dump (void *captured_frame[], void *user, traceprint_t print_trace,
            u64 dsp_count)
{
  if (dsp_count == -1 || print_trace == NULL)
    return 0;

#define MAX_TRACE_MSG_SZ 0x45
  char format_trace[MAX_TRACE_MSG_SZ];

  u64 frame_idx;

  dyninfo_t dlproc = {};

  dyn_getinfo ((void *)trace_dump, &dlproc);
  print_trace (0, false, user, "%s", dlproc.dli_fname, dlproc.dli_fbase);

  for (u64 frame_idx = 0; frame_idx < dsp_count; frame_idx++)
    {
      const void *addr_buffer = captured_frame[frame_idx];
      const char *func_symbol = "stripped";

      const char *dynname = dyn_getsymbolname (addr_buffer, &dlproc);
      if (dynname != NULL)
        func_symbol = dynname;

      snprintf (format_trace, sizeof format_trace, "\t%lu> $(%%s): [%%#llx]\n",
                frame_idx);
      print_trace (frame_idx, true, user, format_trace, func_symbol,
                   addr_buffer);
    }

  return frame_idx;
}
