#ifndef GEM_TRACE
#define GEM_TRACE


#include <stdio.h>


trace_t * read_gem5_trace(int trace, char * raw_data, size_t data_len, int tick_mult);


#endif
