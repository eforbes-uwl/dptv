#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#define CMD_OPTIONS_OK            0
#define CMD_HELP                 -1
#define CMD_ERR_UNINITIALIZED    -2
#define CMD_ERR_BAD_OPTION       -3
#define CMD_ERR_BAD_ARG          -4
#define CMD_ERR_BAD_VALUE        -5
#define CMD_ERR_NO_TRACES        -6
#define CMD_ERR_TOO_MANY_TRACES  -7
#define CMD_ERR_NO_ARGS          -8
#define CMD_ERR_FONT_PATH        -9

#include "gfx.h"

int   process_cmd_line(int, char **);
void  dump_cmd_opts();


typedef struct options_type {
    int win_width;
    int win_height;
    int win_xpos;
    int win_ypos;
    char *win_title;
    char *font_path;
    int num_traces;
    char **trace_filenames;
    char *commit_stage;
    line_sec_t *lines;
    double* scale;
    int main_trace;
    int trace_remove_squash;
    int trace_disable_dummy;
    int trace_disable_cutoff;
    char *arg_command;
    int instr_window_width;
} options_t;

extern options_t *OPTIONS;


typedef struct colors_type {
    gfx_color_t bg;
    gfx_color_t ui;
    gfx_color_t trace_a;
    gfx_color_t trace_b;
    gfx_color_t highlight;
    gfx_color_t cur_search;
} colors_t;

extern colors_t *COLORS;


#endif
