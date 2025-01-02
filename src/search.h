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



