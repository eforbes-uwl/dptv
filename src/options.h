  /* This file is part of the Dual PipeTrace Viewer (dptv) pipeline 
   * trace visualization tool. The dptv project was written by Adam 
   * Grunwald and Elliott Forbes, University of Wisconsin-La Crosse, 
   * copyright 2021-2025.
   *
   * dptv is free software: you can redistribute it and/or modify it
   * under the terms of the GNU General Public License as published 
   * by the Free Software Foundation, either version 3 of the License, 
   * or (at your option) any later version.
   *
   * dptv is distributed in the hope that it will be useful, but 
   * WITHOUT ANY WARRANTY; without even the implied warranty of 
   * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
   * General Public License for more details.
   *
   * You should have received a copy of the GNU General Public License 
   * along with dptv. If not, see <https://www.gnu.org/licenses/>. 
   *
   *
   *
   * The dptv project can be found at https://cs.uwlax.edu/~eforbes/dptv/
   *
   * If you use dptv in your published research, please consider 
   * citing the following:
   *
   * Grunwald, A., Nguyen, P. and Forbes, E., "dptv: A New PipeTrace
   * Viewer for Microarchitectural Analysis," Proceedings of the 55th 
   * Midwest Instruction and Computing Symposium, April 2023. 
   *
   * If you found dptv helpful, please let us know! Email eforbes@uwlax.edu
   *
   * There are bound to be bugs, let us know those too.
   */

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
