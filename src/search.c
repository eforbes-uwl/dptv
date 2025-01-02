#include <SDL2/SDL.h>
#include <stdbool.h>
#include <assert.h>
#include "gfx.h"
#include "search.h"
#include "options.h"
#include "trace_handler.h"


bool first_in;
search_t* SEARCH;


bool init_search() {
    // Initialize search
    SEARCH = (search_t*) malloc(sizeof(search_t));
    assert(SEARCH);
    
    SEARCH->pattern = NULL;
    SEARCH->pattern_len = 0;
    SEARCH->input = NULL;
    SEARCH->input_len = 0;
    SEARCH->search_in = calloc(SEARCHSEC_NUM, sizeof(bool));
    SEARCH->search_in_params = NULL;
    SEARCH->search_in_params_len = 0;
    
    SEARCH->cur_y = 0;
    SEARCH->trace_ind = 0;
    SEARCH->instr_ind = 0;
    SEARCH->stage_ind = 0;
    SEARCH->param_ind = 0;
    
    SEARCH->cur_string = NULL;
    SEARCH->cur_string_pos = 0;
    SEARCH->cur_string_num = 0;

    SEARCH->is_colon = false;

    return true;
}


int search_test(const char* text, char* pattern, int n) {
    // Abort test if either is null
    if (text == NULL || pattern == NULL || n < 0) {
        return -1;
    }
    // Get string lengths
    int tl = strlen(text);
    int pl = strlen(pattern);
    // Loop through and check all the substrings, finding the nth subsring equal to the pattern
    int num_found = 0;
    // Only need to check textlen-searchlen substrings, as ones beyond that would be too small to match the search
    for(int i = 0; i <= (tl-pl); i++) {
        // Check if this substring matches
        if (strncmp(text+i, pattern, pl) == 0) {
            // Found a match!
            if (num_found == n) {
                return i;
            }
            num_found ++;
        }
    }
    // If we got here then it was not found
    return -1;
}

gfx_color_t* search_highlight(const char* text, gfx_color_t def_color, int sec, char* param_name) {
    if (text == NULL) {
        return NULL;
    }
    // Returns an array of colors to be used to color this string input
    int tl = strlen(text);
    gfx_color_t* colors = malloc(sizeof(gfx_color_t) * tl);
    for(int i = 0; i < tl; i++) {
        colors[i] = def_color;
    }
    if (SEARCH->pattern == NULL) {
        return colors;
    }
    // Check if searching in this section
    if (SEARCH->search_in[sec] == false) {
        // Check if searching in specific parameter
        if (param_name == NULL || sec != SEARCHSEC_PARAM_VALUE) {
            return colors;
        }
        if (!search_has_param(param_name)) {
            return colors;
        }
    }
    // Setup highlighted text
    for(int i = 0; true; i++) {
        int pos = search_test(text, SEARCH->pattern, i);
        if (pos == -1) {
            break;
        }
        for(int j = 0; j < SEARCH->pattern_len; j++) {
            assert(pos+j < tl);
            colors[pos+j] = COLORS->highlight;
        }
    }
    // Setup current highlighted text for current search position
    if (SEARCH->cur_string == text) {
        int pos = SEARCH->cur_string_pos;
        for(int j = 0; j < SEARCH->pattern_len; j++) {
            assert(pos+j < tl);
            colors[pos+j] = COLORS->cur_search;
        }
    }
    return colors;
}

gfx_color_t search_highlight_overall(const char* text, gfx_color_t color, int sec, char* param_name) {
    // Get most significant color from provided text
    if (text == NULL || SEARCH->pattern == NULL) {
        return color;
    }
    if (color.int_color == COLORS->cur_search.int_color) {
        return color;
    }
    int pos = search_test(text, SEARCH->pattern, 0);
    if (pos == -1) {
        return color;
    }
    if (SEARCH->search_in[sec] == false) {
        // Check if searching in specific parameter
        if (param_name == NULL || sec != SEARCHSEC_PARAM_VALUE) {
            return color;
        }
        if (!search_has_param(param_name)) {
            return color;
        }
    }
    color = COLORS->highlight;
    if (SEARCH->cur_string == text) {
        color = COLORS->cur_search;
    }
    return color;
}


void search_input_begin(bool type) {
    // Setup search input
    input_mode = INMODE_SEARCH;
    SEARCH->is_colon = type;
    SEARCH->cur_string = NULL;
    SEARCH->input_len = 0;
    if (SEARCH->input == NULL) {
        SEARCH->input = malloc(sizeof(char));
    }
    SEARCH->input[0] = '\0';
    setup_cmd();
    first_in = true;
}

void search_input_end() {
    input_mode = INMODE_CAM;
    setup_cmd();
}

void search_input_abort() {
    search_input_end();
    search_end();
}

void search_input_key(char key) {
    // Add or remove a character from the search
    if (key == 0) {
        // Remove character from search
        SEARCH->input_len --;
        if (SEARCH->input_len < 0) {
            // Abort searching if backspace when no characters
            search_input_abort();
            return;
        }
        //SEARCH->input = realloc(SEARCH->input, SEARCH->input_len+1);      // Want this?
        SEARCH->input[SEARCH->input_len] = '\0';
    } else if (!first_in) {
        // Check if a valid key to enter
        if (key >= 32 && key < 127) {
            // Add character to end of search
            char c = (char)key;
            SEARCH->input_len ++;
            SEARCH->input = realloc(SEARCH->input, SEARCH->input_len+1);
            SEARCH->input[SEARCH->input_len] = '\0';
            SEARCH->input[SEARCH->input_len-1] = c;
        }
    }
    first_in = false;
    search_setup_pattern();
    setup_cmd();
}

void search_setup_pattern() {
    if (SEARCH->is_colon) {
        return;
    }
    free(SEARCH->pattern);
    search_param_clear();
    memset(SEARCH->search_in, false, SEARCHSEC_NUM);
    // Extract search pattern and search locations from input
    // Setup interator over string, split by / characters
    char* input = strdup(SEARCH->input);
    char* token = strtok(input, "/");
    int num = 0;
    while(token) {
        char* next_token = strtok(NULL, "/");
        if (next_token == NULL) {
            // This is the last token
            SEARCH->pattern = strdup(token);
            if (num == 0) {
                // Only a search pattern, set all sections to be searched
                memset(SEARCH->search_in, true, SEARCHSEC_NUM);
            }
        } else {
            // This is not the last token
            // Determine what section this is saying to search in
            char* chars = SEARCHSEC_CHARS;
            char* loc = strchr(chars, token[0]);
            if (loc != NULL) {
                int ind = loc - chars;
                // Check if search for value of specific parameter
                if (strlen(token) >= 3 && token[0] == 'v' && token[1] == ':') {
                    // Add parameter to list
                    search_param_add(strdup(token+2));
                } else {
                    SEARCH->search_in[ind] = true;
                }
            }
        }
        token = next_token;
        num ++;
    }
    if (num == 0) {
        // No input yet, pattern is an empty string
        SEARCH->pattern = strdup("");
    }
    free(input);
    SEARCH->pattern_len = strlen(SEARCH->pattern);
}

void search_param_add(char* param) {
    // Add to list of parameters to search in
    if (SEARCH->search_in_params == NULL) {
        SEARCH->search_in_params = malloc(sizeof(char*));
        SEARCH->search_in_params_len = 1;
    } else {
        SEARCH->search_in_params_len ++;
        SEARCH->search_in_params = realloc(SEARCH->search_in_params, sizeof(char*) * SEARCH->search_in_params_len);
    }
    SEARCH->search_in_params[SEARCH->search_in_params_len - 1] = param;
}
void search_param_clear() {
    // Free all the parameters set to search in
    if (SEARCH->search_in_params == NULL) return;
    for(int i = 0; i < SEARCH->search_in_params_len; i++) {
        free(SEARCH->search_in_params[i]);
    }
    free(SEARCH->search_in_params);
    SEARCH->search_in_params = NULL;
    SEARCH->search_in_params_len = 0;
}
bool search_has_param(char* param_name) {
    for(int i = 0; i < SEARCH->search_in_params_len; i++) {
        if (strcmp(param_name, SEARCH->search_in_params[i]) == 0) {
            return true;
        }
    }
    return false;
}


void search_input_finish() {
    // Finish search input
    search_input_end();
    if (SEARCH->is_colon) {
        // Jump to instruction number
        gfx_jump_y(strtol(SEARCH->input, NULL, 10) * OPTIONS->num_traces);
    } else {
        // Setup searching variables to point to start of area to search
        SEARCH->cur_y = 0;
        SEARCH->cur_string_num = -1;
        SEARCH->trace_ind = 0;
        SEARCH->instr_ind = 0;
        SEARCH->stage_ind = 0;
        SEARCH->param_ind = 0;
        SEARCH->cur_instr = NULL;
        SEARCH->cur_stage = NULL;
        SEARCH->cur_param = NULL;
        SEARCH->cur_section = SEARCHSEC_BEGIN;
        SEARCH->cur_string = NULL;
        // Search for first
        search_find(true, NULL);
    }
}


void search_end() {
    if (!SEARCH->is_colon) {
        free(SEARCH->pattern);
        SEARCH->pattern = NULL;
        SEARCH->pattern_len = 0;
    }
    free(SEARCH->input);
    SEARCH->input = NULL;
    SEARCH->input_len = 0;
    SEARCH->is_colon = false;
}


int64_t search_find(bool next, int64_t* x) {
    if (SEARCH->pattern == NULL) {
        return -1;
    }
    uint64_t y_start = SEARCH->cur_y;
    bool left_y = false;
    bool returned_y = false;
    while(true) {
        // Search for pattern in current string
        if (next) {
            SEARCH->cur_string_num ++;
        } else {
            if (SEARCH->cur_string_num < 0) {
                // Get last valid string position
                SEARCH->cur_string_num = 0;
                while(true) {
                    if (search_test(SEARCH->cur_string, SEARCH->pattern, SEARCH->cur_string_num) == -1) {
                        break;
                    }
                    SEARCH->cur_string_num ++;
                }
            }
            SEARCH->cur_string_num --;
        }
        int pos = search_test(SEARCH->cur_string, SEARCH->pattern, SEARCH->cur_string_num);
        if (pos != -1) {
            SEARCH->cur_string_pos = pos;
            break;
        } else {
            while(true) {
                // Move to next section to search in
                SEARCH->cur_string_num = -1;
                if (next) {
                    search_next();
                } else {
                    search_prev();
                }
                // Detect infinite loop when we've left starting y-position, returned to that y-position, then left it again
                // TODO: This probobally doesn't work when viewing a trace with just one instruction (only one possible y-position). Is this a problem worth fixing?
                if (!left_y) {
                    if (SEARCH->cur_y != y_start) left_y = true;
                } else {
                    if (!returned_y) {
                        if (SEARCH->cur_y == y_start) returned_y = true;
                    } else {
                        if (SEARCH->cur_y != y_start) {
                            // Infinite loop, nothing must match the pattern
                            return -1;
                        }
                    }
                }
                // Check if our current section is one we're selected to search in
                if (SEARCH->search_in[SEARCH->cur_section] == true) {
                    break;
                }
                // Check if looking at specific parameter value selected to search for
                if (SEARCH->search_in_params != NULL) {
                    if (SEARCH->cur_section == SEARCHSEC_PARAM_VALUE) {
                        if (search_has_param(SEARCH->cur_param->name)) {
                            break;
                        }
                    }
                }
            }
            // Get next string to check
            switch(SEARCH->cur_section) {
                case(SEARCHSEC_PC):
                    // Get Program Counter text
                    SEARCH->cur_string = SEARCH->cur_instr->pc_text;
                    break;
                case(SEARCHSEC_INSTR):
                    // Get Instruction name
                    SEARCH->cur_string = SEARCH->cur_instr->instruction;
                    break;
                case(SEARCHSEC_CYCLE):
                    // May or may not impliment searching for cycle numbers in the future (probobally not :>)
                    break;
                case(SEARCHSEC_ID):
                    // Get stage identifier
                    SEARCH->cur_string = SEARCH->cur_stage->id_str;
                    break;
                case(SEARCHSEC_NAME):
                    // Get stage name
                    SEARCH->cur_string = SEARCH->cur_stage->name;
                    break;
                case(SEARCHSEC_PARAM_NAME):
                    // Get parameter name
                    SEARCH->cur_string = SEARCH->cur_param->name;
                    break;
                case(SEARCHSEC_PARAM_VALUE):
                    // Get parameter value
                    SEARCH->cur_string = SEARCH->cur_param->value;
                    break;
                default:
                    break;
            }
        }
    }
    // Get x position
    if (x != NULL) {
        if (SEARCH->cur_stage == NULL || SEARCH->cur_section < SEARCHSEC_ID) {
            if (SEARCH->cur_instr != NULL && SEARCH->cur_instr->stages != NULL) {
                *x = SEARCH->cur_instr->stages->cycle;
            }
        } else {
            *x = SEARCH->cur_stage->cycle;
        }
    }
    return (int64_t)SEARCH->cur_y;
}



void search_next() {
    // Go to next string to search
    switch(SEARCH->cur_section) {
        case(SEARCHSEC_PC):
            // To instruction text
            SEARCH->cur_section = SEARCHSEC_INSTR;
            break;
        case(SEARCHSEC_INSTR):
            // To stage id
            SEARCH->cur_section = SEARCHSEC_ID;
            SEARCH->stage_ind = 0;
        search_next_stage:
            // Continue to next instruction if no more stages
            if (SEARCH->stage_ind >= SEARCH->cur_instr->n_stages) {
                // To PC
                SEARCH->cur_section = SEARCHSEC_PC;
                SEARCH->cur_y ++;
                // Switch Trace
                SEARCH->trace_ind ++;
                if (SEARCH->trace_ind >= OPTIONS->num_traces) {
                    SEARCH->trace_ind = 0;
                    SEARCH->instr_ind ++;
                }
                // Check if loop back to top instruction
                if (SEARCH->instr_ind >= TRACES[SEARCH->trace_ind]->n_insts) {
                    SEARCH->cur_y = 0;
                    SEARCH->instr_ind = 0;
                }
                SEARCH->cur_instr = &TRACES[SEARCH->trace_ind]->insts[SEARCH->instr_ind];
            } else {
                SEARCH->cur_stage = &SEARCH->cur_instr->stages[SEARCH->stage_ind];
            }
            break;
        case(SEARCHSEC_ID):
            // To stage name
            SEARCH->cur_section = SEARCHSEC_NAME;
            break;
        case(SEARCHSEC_NAME):
            // To param name
            SEARCH->cur_section = SEARCHSEC_PARAM_NAME;
            SEARCH->param_ind = 0;
        search_next_param:
            // Continue to next stage if no more parameters
            if (SEARCH->param_ind >= SEARCH->cur_stage->n_params) {
                SEARCH->cur_section = SEARCHSEC_ID;
                SEARCH->stage_ind ++;
                goto search_next_stage;
            } else {
                SEARCH->cur_param = &SEARCH->cur_stage->params[SEARCH->param_ind];
            }
            break;
        case(SEARCHSEC_PARAM_NAME):
            // To param value
            SEARCH->cur_section = SEARCHSEC_PARAM_VALUE;
            break;
        case(SEARCHSEC_PARAM_VALUE):
            // To next param name
            SEARCH->cur_section = SEARCHSEC_PARAM_NAME;
            SEARCH->param_ind ++;
            goto search_next_param;
        case(SEARCHSEC_BEGIN):
            // Begin searching
            SEARCH->cur_section = SEARCHSEC_PC;
            SEARCH->instr_ind = 0;
            SEARCH->trace_ind = 0;
            SEARCH->cur_instr = &TRACES[0]->insts[0];
        default:
            break;
    }
}


void search_prev() {
    // Go to previous string to search
    switch(SEARCH->cur_section) {
        case(SEARCHSEC_PC):
            // To param value
            SEARCH->cur_section = SEARCHSEC_PARAM_VALUE;
            // Move to previous instruction
            SEARCH->cur_y --;
            // Switch between traces
            SEARCH->trace_ind --;
            if (SEARCH->trace_ind < 0) {
                SEARCH->trace_ind = OPTIONS->num_traces-1;
                // Check if loop back to bottom instruction
                if (SEARCH->instr_ind == 0) {
                    SEARCH->instr_ind = TRACES[SEARCH->trace_ind]->n_insts - 1;
                    SEARCH->cur_y = TRACES[SEARCH->trace_ind]->n_insts * OPTIONS->num_traces - OPTIONS->num_traces;
                } else {
                    SEARCH->instr_ind --;
                }
            }
            SEARCH->cur_instr = &TRACES[SEARCH->trace_ind]->insts[SEARCH->instr_ind];
            // Check if no stages
            if (SEARCH->cur_instr->n_stages == 0) {
                // To instruction text
                SEARCH->cur_section = SEARCHSEC_INSTR;
                break;
            }
            // Go to last stage
            SEARCH->stage_ind = SEARCH->cur_instr->n_stages - 1;
            SEARCH->cur_stage = &SEARCH->cur_instr->stages[SEARCH->stage_ind];
        search_to_last_param:
            // Check if no parameters
            if (SEARCH->cur_stage->n_params == 0) {
                // To stage name
                SEARCH->cur_section = SEARCHSEC_NAME;
                break;
            }
            // Go to last parameter
            SEARCH->param_ind = SEARCH->cur_stage->n_params - 1;
            SEARCH->cur_param = &SEARCH->cur_stage->params[SEARCH->param_ind];
            break;
        case(SEARCHSEC_INSTR):
            // To program counter
            SEARCH->cur_section = SEARCHSEC_PC;
            break;
        case(SEARCHSEC_ID):
            // Continue to instruction text if no more stages
            if (SEARCH->stage_ind == 0) {
                // To instruction text
                SEARCH->cur_section = SEARCHSEC_INSTR;
            } else {
                // To parameter value
                SEARCH->cur_section = SEARCHSEC_PARAM_VALUE;
                SEARCH->stage_ind --;
                SEARCH->cur_stage = &SEARCH->cur_instr->stages[SEARCH->stage_ind];
                // Go to last parameter of previous stage
                goto search_to_last_param;
            }
            break;
        case(SEARCHSEC_NAME):
            // To stage id
            SEARCH->cur_section = SEARCHSEC_ID;
            break;
        case(SEARCHSEC_PARAM_NAME):
            // Continue to stage name if no more parameters
            if (SEARCH->param_ind == 0) {
                SEARCH->cur_section = SEARCHSEC_NAME;
            } else {
                // To param value
                SEARCH->cur_section = SEARCHSEC_PARAM_VALUE;
                SEARCH->param_ind --;
                SEARCH->cur_param = &SEARCH->cur_stage->params[SEARCH->param_ind];
            }
            break;
        case(SEARCHSEC_PARAM_VALUE):
            // To next param name
            SEARCH->cur_section = SEARCHSEC_PARAM_NAME;
            break;
        default:
            break;
    }
}

