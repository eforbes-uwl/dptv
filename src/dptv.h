#ifndef _DPTV_H_
#define _DPTV_H_

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#define PROJECT_NAME "Dual Pipetrace Viewer"


typedef struct parameter_type {
    char *name;
    char *value;
} parameter_t;

typedef struct stage_type {
    uint64_t cycle;
    char identifier;
    char * id_str;
    uint32_t color;
    char * name;
    struct parameter_type * params;
    uint32_t n_params;
} stage_t;

typedef struct instruction_type {
    uint8_t valid; // to distinguish between instructions, and padding used to align instructions from two traces
    uint8_t tid;
    uint64_t pc;
    char * pc_text;
    char * instruction;
    uint8_t committed;
    struct stage_type * stages;
    uint32_t n_stages;
} instruction_t;

typedef struct trace_type {
    char *name;
    struct instruction_type * insts;
    uint64_t n_insts;
} trace_t;


extern bool quit;

#endif
