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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "trace_handler.h"
#include "dptv.h"
#include "array.h"
#include "yaml.h"


void insert_position(trace_t * trace, instruction_t * inst, uint64_t ins_pos, size_t * size) {
    // Extend size of array
    size_t req_size = (trace->n_insts + 1) * sizeof(instruction_t);
    if (req_size >= *size) {
        *size = (((trace->n_insts + 1) * 3) / 2) * sizeof(instruction_t);
        trace->insts = realloc(trace->insts, *size);
        assert(trace->insts);
    }
    
    // Shift data after inserted position over
    void * src = trace->insts + ins_pos*sizeof(instruction_t);
    void * dst = src + sizeof(instruction_t);
    size_t shift_am = trace->n_insts - ins_pos;
    memmove(dst, src, shift_am*sizeof(instruction_t));
    
    // Copy data into array
    memmove(src, inst, sizeof(instruction_t));
    trace->n_insts ++;
}


void progress_line(char ** str) {
    while(**str != '\n' && **str != '\0') {
        (*str) ++;
    }
    (*str) ++;
}

trace_t * read_gem5_trace(int trace, char * raw_data, size_t data_len, int tick_mult) {
    
    // For now, assume ExtPipeview
    
    FILE * test_file = fopen("test_extract.yaml", "w");
    
    // Will reuse file buffer for yaml data
    
    char * cur_data = raw_data;
    char * yaml_data = raw_data;
    size_t yaml_data_len = 0;
    
    // Construct YAML file
    while(*cur_data != '\0') {
        if (strncmp(cur_data, "ExtPipeView:", 12) == 0) {
            progress_line(&cur_data);
            assert(*cur_data == '{');
            progress_line(&cur_data);
            // Write to beginning of buffer until '}' is reached
            while(*cur_data != '}') {
                assert(yaml_data < cur_data);
                fprintf(test_file, "%c", *cur_data);
                *yaml_data = *cur_data;
                yaml_data ++;
                cur_data ++;
                yaml_data_len ++;
            }
        }
        progress_line(&cur_data);
    }
    *yaml_data = '\0';
    
    //printf("\n");
    //fflush(stdout);
    fclose(test_file);
    
    // Load YAML file
    return read_yaml_trace_raw(raw_data, yaml_data_len);
    
    /*

    TRACES[trace] = malloc(sizeof(trace_t));

    uint8_t inst_buff[sizeof(instruction_t)];
    instruction_t * inst = (instruction_t*)inst_buff;
    stage_t * stage = NULL;
    size_t inst_size = 0;
    size_t stage_size = 0;
    TRACES[trace]->n_insts = 0;
    TRACES[trace]->insts = NULL;

    while (true) {
        // Parse current line
        char * line_cpy = strdup(line_buff);
        char * cpu_type = strtok(line_cpy, ":");    // cpu_type currently unused, but could possibly be a parameter(?)
        char * stage_text = strtok(NULL, ":");
        char * cycle_text = strtok(NULL, ":");
        char * pc_text = strtok(NULL, ":");         // if there is no pc_text on this line then this and the next are NULL
        char * thread_text = strtok(NULL, ":");
        char * pos_text = strtok(NULL, ":");
        char * inst_text = strtok(NULL, ":");

        if (cycle_text == NULL) {
            fprintf(stderr,"\nERROR reading trace file : missing vital stage parameters : \"%s\"\n",line_buff);
            exit(EXIT_FAILURE);
        }
        
        if (strcmp(stage_text, "fetch") == 0) {
            // Start of new instruction
            if (inst_text == NULL) {
                fprintf(stderr,"\nERROR reading trace file : missing vital instruction parameters : \"%s\"\n",line_buff);
                exit(EXIT_FAILURE);
            }
            uint64_t cycle_pos = strtol(cycle_text, NULL, 10) / tick_mult;
            uint64_t inst_pos = strtol(pos_text, NULL, 10);
            if (cycle_pos != 0 && inst_pos != 0) {
                uint64_t pc = strtol(pc_text, NULL, 10);
                uint8_t tid = atoi(thread_text);
                inst->valid = true;
                inst->tid = tid;
                inst->pc = pc;
                inst->pc_text = strdup(pc_text);
                inst->instruction = strdup(inst_text);
                inst->committed = false;
                inst->n_stages = 0;
                // Add instruction to list
                insert_position(TRACES[trace], inst, inst_pos-1, &inst_size);
                // Setup fetch stage
                stage_size = 0;
                inst->stages = add_array_list_new(NULL, &inst->n_stages, &stage_size, (void*)&stage, sizeof(stage_t));
                stage->cycle = cycle_pos;
                stage->identifier = 'f';
                stage->id_str[0] = 'f'; stage->id_str[1] = '\0';
                stage->color = 0;
                stage->name = strdup("fetch");
                stage->n_params = 0;
                stage->params = NULL;
            }
        } else if (inst != NULL) {
            // New stage
            uint64_t cycle_pos = strtol(cycle_text, NULL, 10) / tick_mult;
            if (cycle_pos != 0) {
                if (cycle_pos > stage->cycle) {
                    inst->stages = add_array_list_new(inst->stages, &inst->n_stages, &stage_size, (void*)&stage, sizeof(stage_t));
                }
                stage->cycle = cycle_pos;
                stage->identifier = stage_text[0];
                if (strcmp(stage_text, "dispatch") == 0) stage->identifier = 'p';
                if (strcmp(stage_text, "rename") == 0) stage->identifier = 'n';
                if (strcmp(stage_text, "retire") == 0) inst->committed = true;
                stage->id_str[0] = stage->identifier;
                stage->id_str[1] = '\0';
                stage->color = 0;
                stage->name = strdup(stage_text);
                stage->n_params = 0;
                stage->params = NULL;
            }
        }
        free(line_cpy);

        // Next line
        if (feof(fd)) {
            break;
        }
        fgets(line_buff, 100, fd);
        
    }
    */

}


