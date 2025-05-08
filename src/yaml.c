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

#include <libdeflate.h>
#include <cyaml/cyaml.h>
#include <stdlib.h>
#include "dptv.h"
#include "yaml.h"
#include <stdio.h>


// Options for how yaml works
static const cyaml_config_t config = {
    .log_fn = cyaml_log,
    .mem_fn = cyaml_mem,
    .log_level = CYAML_LOG_WARNING,
};

// What the layout of our file is going to be


static const cyaml_schema_field_t schema_param[] = {
    CYAML_FIELD_STRING_PTR("name", CYAML_FLAG_POINTER,
                            parameter_t, name,
                            0, CYAML_UNLIMITED),
    CYAML_FIELD_STRING_PTR("value", CYAML_FLAG_POINTER,
                            parameter_t, value,
                            0, CYAML_UNLIMITED),
    CYAML_FIELD_END
};

static const cyaml_schema_value_t schema_param_val = {
    CYAML_VALUE_MAPPING(CYAML_FLAG_DEFAULT,
                            parameter_t,
                            schema_param),
};

static const cyaml_schema_field_t schema_stage[] = {
    CYAML_FIELD_UINT("cycle", CYAML_FLAG_DEFAULT,
                            stage_t, cycle),
    CYAML_FIELD_STRING_PTR("id", CYAML_FLAG_DEFAULT,
                            stage_t, id_str,
                            0, CYAML_UNLIMITED),
    CYAML_FIELD_STRING_PTR("name", CYAML_FLAG_DEFAULT,
                            stage_t, name,
                            0, CYAML_UNLIMITED),
    CYAML_FIELD_UINT("color", CYAML_FLAG_DEFAULT,
                            stage_t, color),
    CYAML_FIELD_SEQUENCE_COUNT("params", CYAML_FLAG_POINTER | CYAML_FLAG_OPTIONAL,
                            stage_t, params, n_params,
                            &schema_param_val,
                            0, CYAML_UNLIMITED),
    CYAML_FIELD_END
};

static const cyaml_schema_value_t schema_stage_val = {
    CYAML_VALUE_MAPPING(CYAML_FLAG_DEFAULT,
                            stage_t,
                            schema_stage),
};

static const cyaml_schema_field_t schema_instr[] = {
    CYAML_FIELD_UINT("tid", CYAML_FLAG_DEFAULT,
                            instruction_t, tid),
    CYAML_FIELD_STRING_PTR("pc", CYAML_FLAG_POINTER,
                            instruction_t, pc_text,
                            0, CYAML_UNLIMITED),
    CYAML_FIELD_STRING_PTR("text", CYAML_FLAG_POINTER,
                            instruction_t, instruction,
                            0, CYAML_UNLIMITED),
    CYAML_FIELD_SEQUENCE_COUNT("stages", CYAML_FLAG_POINTER | CYAML_FLAG_OPTIONAL,
                            instruction_t, stages, n_stages,
                            &schema_stage_val,
                            0, CYAML_UNLIMITED),
    CYAML_FIELD_END
};

static const cyaml_schema_value_t schema_instr_val = {
    CYAML_VALUE_MAPPING(CYAML_FLAG_DEFAULT,
                            instruction_t,
                            schema_instr),
};

static const cyaml_schema_field_t schema_trace[] = {
    CYAML_FIELD_STRING_PTR("name", CYAML_FLAG_POINTER | CYAML_FLAG_OPTIONAL,
                            trace_t, name,
                            0, CYAML_UNLIMITED),
    CYAML_FIELD_SEQUENCE_COUNT("insts", CYAML_FLAG_POINTER,
                            trace_t, insts, n_insts,
                            &schema_instr_val,
                            0, CYAML_UNLIMITED),
    CYAML_FIELD_END
};

static const cyaml_schema_value_t schema_main = {
    CYAML_VALUE_MAPPING(CYAML_FLAG_POINTER,
                            trace_t, 
                            schema_trace),
};




trace_t * read_yaml_trace(char * fname) {
    // Read yaml file
    trace_t * trace;
    cyaml_err_t err = cyaml_load_file(fname, &config,
            &schema_main, (void **) &trace, NULL);
    if (err != CYAML_OK) {
        fprintf(stderr, "ERROR: %s\n", cyaml_strerror(err));
        free(trace);
        return NULL;
    }
    // Setup data not directly from yaml
    setup_yaml_trace(trace);
    return trace;
}

trace_t * read_yaml_trace_raw(void * data, size_t len) {
    // Read yaml file
    trace_t * trace;
    cyaml_err_t err = cyaml_load_data(data, len, &config,
            &schema_main, (void **) &trace, NULL);
    if (err != CYAML_OK) {
        fprintf(stderr, "ERROR: %s\n", cyaml_strerror(err));
        free(trace);
        return NULL;
    }
    // Setup data not directly from yaml
    setup_yaml_trace(trace);
    return trace;
}

trace_t * read_yaml_trace_compressed(char * fname) {
    // Read full file into buffer
    FILE *f = fopen(fname, "rb");
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char * buffer = malloc(sizeof(unsigned char) * fsize);
    fread(buffer, fsize, 1, f);
    fclose(f);
    // Get decompressed size
    size_t out_len = buffer[fsize-1] * 256 * 256 * 256
                   + buffer[fsize-2] * 256 * 256
                   + buffer[fsize-3] * 256
                   + buffer[fsize-4];
    // Decompress into buffer with libdeflate
    struct libdeflate_decompressor * decomp = libdeflate_alloc_decompressor();
    char * out_buffer = malloc(sizeof(char) * (out_len + 1));
    enum libdeflate_result res = libdeflate_gzip_decompress(decomp, buffer, fsize, out_buffer, out_len, NULL);
    if (res != LIBDEFLATE_SUCCESS) {
        printf("ERROR: gz decompression failed: %d\n", (int)res);
        return NULL;
    }
    // Read yaml file
    trace_t * trace = read_yaml_trace_raw(out_buffer, out_len);
    // Free data
    libdeflate_free_decompressor(decomp);
    free(buffer);
    free(out_buffer);
    return trace;
}


void setup_yaml_trace(trace_t * trace) {
    for(uint64_t i = 0; i < trace->n_insts; i++) {
        instruction_t * inst = &trace->insts[i];
        inst->valid = true;
        for(uint32_t s = 0; s < inst->n_stages; s++) {
            stage_t * stage = &inst->stages[s];
            stage->identifier = stage->id_str[0];
            if (stage->identifier == 'R') {
                inst->committed = true;
            }
        }
    }
}



