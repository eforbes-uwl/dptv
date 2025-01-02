#include "dptv.h"

trace_t * read_yaml_trace(char * fname);
trace_t * read_yaml_trace_compressed(char * fname);
void setup_yaml_trace(trace_t * trace);
trace_t * read_yaml_trace_raw(void * data, size_t len);
