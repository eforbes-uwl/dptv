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

#ifndef _SEARCH__
#define _SEARCH__

#include <stdbool.h>
#include "dptv.h"
#include "gfx.h"


bool init_search();
int search_test(const char* text, char* pattern, int n);
gfx_color_t* search_highlight(const char* text, gfx_color_t def, int sec, char* param_name);
gfx_color_t search_highlight_overall(const char* text, gfx_color_t color, int sec, char* param_name);

void search_input_begin(bool type);
void search_input_end();
void search_input_abort();
void search_input_key(char key);
void search_input_finish();

void search_setup_pattern();
void search_param_add(char* name);
void search_param_clear();
bool search_has_param(char* param_name);

void search_end();
int64_t search_find(bool, int64_t* x);
void search_next();
void search_prev();



#define SEARCHSEC_PC            0
#define SEARCHSEC_INSTR         1
#define SEARCHSEC_CYCLE         2
#define SEARCHSEC_ID            3
#define SEARCHSEC_NAME          4
#define SEARCHSEC_PARAM_NAME    5
#define SEARCHSEC_PARAM_VALUE   6
#define SEARCHSEC_BEGIN         7

#define SEARCHSEC_NUM           7

#define SEARCHSEC_CHARS "picdsnv"


typedef struct search_type {
    bool is_colon;
    // Overall search parameters
    char* input;
    int input_len;
    char* pattern;
    int pattern_len;
    bool* search_in;
    char** search_in_params;
    int search_in_params_len;
    // Current search position
    uint64_t cur_y;
    uint64_t cur_x;
    int trace_ind;
    int cur_section;
    size_t instr_ind;
    size_t stage_ind;
    size_t param_ind;
    instruction_t * cur_instr;
    stage_t * cur_stage;
    parameter_t * cur_param;
    
    char* cur_string;
    int cur_string_pos;
    int cur_string_num;
} search_t;


extern search_t *SEARCH;


#endif



