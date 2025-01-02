#ifndef _TRACE_HANDLER_H_
#define _TRACE_HANDLER_H_

#include "options.h"
#include "dptv.h"

void init_traces();
void dump_trace(int);

instruction_t * new_dummy_instruction();
trace_t * new_trace(char *name);

extern trace_t **TRACES;

#endif
