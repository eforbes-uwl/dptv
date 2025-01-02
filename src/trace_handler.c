#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "dptv.h"
#include "options.h"
#include "trace_handler.h"
#include "trace_gem.h"
#include "yaml.h"

// array of traces
trace_t **TRACES = NULL;

// prototypes for helper functions
static trace_t * read_trace_file_compressed(FILE *);
static void read_trace_file(int);
static void post_process_trace(int);
static void align_multi_trace();
static char * get_file_ext(char *);

long trace_find_common(int*, int*, int*, int*);
long trace_find_common_sub(int*, int*, int*, int*, trace_t*, trace_t*);
int find_commited(trace_t*, int*);
int find_commited_prev(trace_t*, int*);
static void add_dummy_nodes(trace_t*, int, int);
static void fill_dummy(instruction_t *);
static void remove_start(trace_t*, int);
static void remove_end(trace_t*, int);
static void shift_add_dummy(trace_t*, int, int, int, int);

// initialize array of traces
void init_traces(){
    int i;

    TRACES = (trace_t**) malloc(sizeof(trace_t*)*OPTIONS->num_traces);
    assert(TRACES);
    for (i=0;i<OPTIONS->num_traces;i++){
        TRACES[i] = NULL;
    }


    for (i=0;i<OPTIONS->num_traces;i++){
        read_trace_file(i);
        post_process_trace(i);
    }

    if (OPTIONS->num_traces > 1) {
        align_multi_trace();
    }


    /*for (i=0;i<OPTIONS->num_traces;i++){
        dump_trace(i);
    }*/

    /*for (i=0;i<OPTIONS->num_traces;i++){
        remove_uncommited(TRACES[i]);
    }*/
}

// print trace info to console
void dump_trace(int trace){
    instruction_t *working_inst = NULL;
    stage_t *working_stage = NULL;
    parameter_t *working_parameter = NULL;

    if (trace < OPTIONS->num_traces){
        printf("------------trace %s--------------\n",TRACES[trace]->name);
        for(int i = 0; i < TRACES[trace]->n_insts; i++) {
            working_inst = &TRACES[trace]->insts[i];
            if (strstr(working_inst->instruction,"BCAST") != NULL)
                printf("I]%" PRIu8 " %s\n",working_inst->tid, working_inst->instruction);
            else
                printf("I]%" PRIu8 " 0x%09" PRIx64 " %s\n",working_inst->tid, working_inst->pc, working_inst->instruction);
            for(int j = 0; j < working_inst->n_stages; j++) {
                working_stage = &working_inst->stages[j];
                printf("S]%" PRIu64 " %c %" PRIu32 " %s\n",working_stage->cycle, working_stage->identifier, working_stage->color, working_stage->name);
                for(int k = 0; k < working_stage->n_params; k++) {
                    working_parameter = &working_stage->params[k];
                    printf("P]%s\n", working_parameter->name);
                    printf("V]%s\n", working_parameter->value);
                }
            }
        }
    }
}

// open and parse a .trace file
static void read_trace_file(int trace_id) {
    trace_t * trace = NULL;
    FILE * fd = fopen(OPTIONS->trace_filenames[trace_id], "r");
    assert(fd);

    if (trace_id < OPTIONS->num_traces) {
        
        // Read first 64 bytes
        char line_buff[64];
        size_t read_size = fread(line_buff, sizeof(char), 64, fd);
        
        // Check if this file is gz compressed
        if (line_buff[0] == 0x1F && line_buff[1] == 0x8B) {
            trace = read_trace_file_compressed(fd);
        } else {
            // Check type of trace file.
            // If first character is upper case it's gem5. Otherwise it's dptv.
            if (line_buff[0] >= 'A' && line_buff[0] <= 'Z') {
                // Read full gem5 trace
                printf("reading gem5 trace %s...",OPTIONS->trace_filenames[trace_id]);
                fflush(stdout);
                char * file_buff = NULL;
                size_t file_buff_size = 0;
                while(read_size > 0) {
                    file_buff = realloc(file_buff, sizeof(char) * (file_buff_size + read_size + 1));
                    assert(file_buff);
                    memcpy(file_buff+file_buff_size, line_buff, read_size);
                    file_buff_size += read_size;
                    read_size = fread(line_buff, sizeof(char), 64, fd);
                }
                file_buff[file_buff_size] = '\0';
                trace = read_gem5_trace(trace_id, file_buff, file_buff_size, 500);
                free(file_buff);
            } else if (line_buff[0] >= 'a' && line_buff[0] <= 'z') {
                // DPTV
                fclose(fd);
                printf("reading dptv trace %s...",OPTIONS->trace_filenames[trace_id]);
                fflush(stdout);
                trace = read_yaml_trace(OPTIONS->trace_filenames[trace_id]);
            } else {
                fprintf(stderr, "Failed to detect trace type\n");
            }
        }
        
        if (trace == NULL) {
            fprintf(stderr, "Error loading trace file %d, see above.\n", trace_id);
            exit(-1);
        }
        TRACES[trace_id] = trace;
        assert(TRACES[trace_id]);

        printf("done.\n");
        fflush(stdout);
    }
}

static trace_t * read_trace_file_compressed(FILE * fd) {
    fprintf(stderr, "TODO");
    return NULL;
}

static void align_multi_trace(){
    
    printf("aligning traces...");
    fflush(stdout);

    // Find position in traces where dynamic instruction streams match up
    int start_a, start_b, end_a, end_b;
    long length = trace_find_common(&start_a, &start_b, &end_a, &end_b);
    if (length == 0) {
        fprintf(stderr, "Error: Could not find common stream of instructions, traces do not match\n");
        exit(1);
    }
    
    // Either remove nodes before & after matching section, or add padding to begining / end depending on options
    if (OPTIONS->trace_disable_cutoff) {
        // Add dummy nodes
        int add_start_a = (start_b > start_a) ? start_b - start_a : 0;
        int add_start_b = (start_a > start_b) ? start_a - start_b : 0;
        add_dummy_nodes(TRACES[0], 0, add_start_a);
        add_dummy_nodes(TRACES[1], 0, add_start_b);
        end_a += add_start_a;
        end_b += add_start_b;
        int add_end_a = (end_b > end_a) ? end_b - end_a : 0;
        int add_end_b = (end_a > end_b) ? end_a - end_b : 0;
        add_dummy_nodes(TRACES[0], end_a, add_end_a);
        add_dummy_nodes(TRACES[1], end_b, add_end_b);
    } else {
        // Cutoff start/end
        remove_end(TRACES[0], end_a);
        remove_end(TRACES[1], end_b);
        remove_start(TRACES[0], start_a);
        remove_start(TRACES[1], start_b);
    }
    
    if (OPTIONS->trace_disable_dummy) {
        printf("done.\n");
        fflush(stdout);
        return;
    }

    
    // Add dummy nodes between commited instructions
    int i0 = 0;
    int i1 = 0;
    trace_t * trace0 = TRACES[0];
    trace_t * trace1 = TRACES[1];
    // Find how long the new instruction array needs to be
    long l = 0;
    while(true) {
        int m0 = find_commited(trace0, &i0) + 1;
        int m1 = find_commited(trace1, &i1) + 1;
        if (m0 > m1)    { l += (long)m0; }
        else            { l += (long)m1; }
        i0 ++;  i1 ++;
        if (i0 >= trace0->n_insts || i1 >= trace1->n_insts) break;
    }
    // Resize instruction arrays
    trace0->insts = realloc(trace0->insts, l*sizeof(instruction_t));
    trace1->insts = realloc(trace1->insts, l*sizeof(instruction_t));
    // Shift instructions and add dummy instructions
    i0 --;  i1 --;
    int shift_to = l - 1;
    while(true) {
        int m0 = find_commited_prev(trace0, &i0) + 1;
        int m1 = find_commited_prev(trace1, &i1) + 1;
        if (i0 < 0 || i1 < 0) break;
        int m = (m0 < m1) ? m1 : m0;
        // Shift & Add dummys
        shift_add_dummy(trace0, shift_to, i0+m0-1, m-m0, m0);
        shift_add_dummy(trace1, shift_to, i1+m1-1, m-m1, m1);
        shift_to -= m;
        i0 --;
        i1 --;
    }
    trace0->n_insts = l;
    trace1->n_insts = l;

    

    printf("done.\n");
    fflush(stdout);

}

static void shift_add_dummy(trace_t * trace, int dest_end, int src_end, int num_dummy, int num_shift) {
    int to = dest_end;
    for(int i = 0; i < num_dummy; i++) {
        fill_dummy(&trace->insts[to--]);
    }
    int from = src_end;
    for(int i = 0; i < num_shift; i++) {
        trace->insts[to--] = trace->insts[from--];
    }
}




long trace_find_common(int *start_a, int *start_b, int *end_a, int *end_b) {
    // Find point in traces where the dynamic instructions line up
    // Finds the largest sub-list between the traces
    // Usually this takes best-case O(n^2), but since one trace will always start with a
    // shared instruction, we can do it in best-case O(n) by only comparing from the start
    // of each trace (worst case is still O(n^2) I think, but average case should be closer to O(n)
    
    // Try with each trace as the start of the comparison
    int a_start_a, a_start_b, a_end_a, a_end_b;
    int b_start_a, b_start_b, b_end_a, b_end_b;
    long length_a = trace_find_common_sub(&a_start_a, &a_start_b, &a_end_a, &a_end_b, TRACES[0], TRACES[1]);
    long length_b = trace_find_common_sub(&b_start_b, &b_start_a, &b_end_b, &b_end_a, TRACES[1], TRACES[0]);

    if (length_a > length_b) {
        *start_a = a_start_a;
        *start_b = a_start_b;
        *end_a = a_end_a;
        *end_b = a_end_b;
        return length_a;
    } else {
        *start_a = b_start_a;
        *start_b = b_start_b;
        *end_a = b_end_a;
        *end_b = b_end_b;
        return length_b;
    }
}

long trace_find_common_sub(int *trace_a_start, int *trace_b_start, int *trace_a_end, int *trace_b_end, trace_t *trace_a, trace_t *trace_b) {
    /// Find where to start b such that the instructions match from the start of a to the end of b
    
    // Find the first commited instruction
    *trace_a_start = 0;
    *trace_b_start = 0;
    find_commited(trace_a, trace_a_start);
    find_commited(trace_b, trace_b_start);
    while(*trace_b_start < trace_b->n_insts) {
        long length = 0;
        int i0 = *trace_a_start;
        int i1 = *trace_b_start;
        while(i0 < trace_a->n_insts && i1 < trace_b->n_insts) {
            // Check if pc of instruction a & b are the same
            if (strcmp(trace_a->insts[i0].pc_text, trace_b->insts[i1].pc_text) == 0) {
                length ++;
            } else {
                break;
            }
            // Move a & b to the next commited instruction
            i0 ++;  i1 ++;
            find_commited(trace_a, &i0);
            find_commited(trace_b, &i1);
        }
        // Check if b reached the end
        if (i1 >= trace_b->n_insts) {
            *trace_b_end = i1;
            *trace_a_end = i0;
            return length;
        }
        // Move head b foreward
        (*trace_b_start) ++;
        find_commited(trace_b, trace_b_start);
    }
    return 0;
}

int find_commited(trace_t * trace, int * ind) {
    int len = 0;
    while(*ind < trace->n_insts) {
        if (trace->insts[*ind].committed)
            break;
        (*ind) ++;
        len ++;
    }
    return len;
}

int find_commited_prev(trace_t * trace, int * ind) {
    int len = 0;
    while(*ind >= 0) {
        if (trace->insts[*ind].committed)
            break;
        (*ind) --;
        len ++;
    }
    return len;
}



trace_t * new_trace(char *name){
    trace_t * t = (trace_t*) malloc(sizeof(trace_t));
    assert(t);
    t->name = strdup(name);
    t->n_insts = 0;
    t->insts = NULL;

    return t;
}



static void remove_start(trace_t* trace, int len) {
    // TODO: Free data in start instructions
    int new_len = trace->n_insts - len;
    memmove(trace->insts, trace->insts + len, new_len*sizeof(instruction_t));
    trace->n_insts = new_len;
}
static void remove_end(trace_t* trace, int len) {
    // TODO: Free data in end instructions
    trace->n_insts = len;
}

static void add_dummy_nodes(trace_t * trace, int len, int pos) {
    uint64_t new_size = trace->n_insts + len;
    trace->insts = realloc(trace->insts, new_size*sizeof(instruction_t));
    // Shift current instructions
    memmove(trace->insts + (pos+len), trace->insts + pos, (trace->n_insts-pos)*sizeof(instruction_t));
    trace->n_insts = new_size;
    // Add dummy nodes
    for(int i = pos; i < (pos+len); i++) {
        instruction_t * inst = &trace->insts[i];
        fill_dummy(inst);
    }
}

static void fill_dummy(instruction_t * inst) {
    inst->stages = NULL;
    inst->n_stages = 0;
    inst->pc_text = "";
    inst->valid = 0;
    inst->instruction = "";
    inst->committed = 0;
    inst->tid = 0;
}


static char * get_file_ext(char * str) {
    char * str_search = str;
    // Find end of string
    while (*str_search != '\0') {
        str_search ++;
    }
    // Back up to file extention
    while (*str_search != '.') {
        str_search --;
        // Protect against filenames without extentions
        if (str_search == str) {
            return str_search;
        }
    }
    str_search ++;
    return str_search;
}


static void post_process_trace(int trace_id) {
    
    trace_t * trace = TRACES[trace_id];
    
    // Remove leading spaces from instruction names
    for(uint64_t i = 0; i < trace->n_insts; i++) {
        
        instruction_t * inst = &trace->insts[i];
        while(*inst->instruction == ' ') {
            inst->instruction ++;
        }
        
    }
}

