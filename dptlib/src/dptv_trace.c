#include "dptv_trace.h"
#include <assert.h>



#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))



string_buff_t new_string_buff() {
    string_buff_t buff = (string_buff_t){ .cap = 32, .str = malloc(sizeof(char) * 32) };
    buff.str[0] = '\0';
    return buff;
}

void free_string_buff(string_buff_t buff) {
    free(buff.str);
}

void extend_string_buff(string_buff_t * buff, const char * add) {
    if (add != NULL) {
        size_t add_len = strlen(add);
        size_t cur_len = strlen(buff->str);
        while (cur_len + add_len + 1 > buff->cap) {
            size_t new_cap = MAX(((buff->cap / 2) * 3), cur_len + add_len + 1);
            buff->str = realloc(buff->str, sizeof(char) * new_cap);
            buff->cap = new_cap;
        }
        strcpy(buff->str+cur_len, add);
    }
}

void extend_string_buff_num(string_buff_t * buff, uint64_t num) {
    char add[32];
    snprintf(add, 32, "%" PRIu64 "", num);
    extend_string_buff(buff, add);
}

void extend_string_buff_indent(string_buff_t * buff, int indent) {
    for(int i = 0; i < indent; i++) {
        extend_string_buff(buff, " ");
    }
}

void line_write_string(void * out, int indent, char * lhs, char * rhs, bool to_file) {
    if (to_file) {
        fprintf(out, "%*s: %s\n", indent, lhs, rhs);
    } else {
        extend_string_buff_indent(out, indent);
        extend_string_buff(out, lhs);
        extend_string_buff(out, rhs);
        extend_string_buff(out, "\n");
    }
}

void line_write_string_quoted(void * out, int indent, char * lhs, char * rhs, bool to_file) {
    if (to_file) {
        fprintf(out, "%*s: \"%s\"\n", indent, lhs, rhs);
    } else {
        extend_string_buff_indent(out, indent);
        extend_string_buff(out, lhs);
        extend_string_buff(out, "\"");
        extend_string_buff(out, rhs);
        extend_string_buff(out, "\"\n");
    }
}

void line_write_char(void * out, int indent, char * lhs, char rhs, bool to_file) {
    char rhs_str[2];
    rhs_str[0] = rhs;
    rhs_str[1] = '\0';
    line_write_string(out, indent, lhs, rhs_str, to_file);
}

void line_write_num(void * out, int indent, char * lhs, uint64_t rhs, bool to_file) {
    char rhs_str[32];
    snprintf(rhs_str, 32, "%" PRIu64 "", rhs);
    line_write_string(out, indent, lhs, rhs_str, to_file);
}



/// Prameter construction code

/*
Create a new dptv parameter
*/
parameter_t new_param(char * name, char * value) {
    return (parameter_t){ .name = name, .value = value };
}
/*
Set the parameter values on an existing parameter
The existing name and value strings are freed
*/
void param_set(parameter_t * param, char * name, char * value) {
    free(param->name);
    param->name = name;
    free(param->value);
    param->value = value;
}
/*
Output the paramter data to a string in yaml format
*/
char * param_to_string(parameter_t * param, int indent) {
    string_buff_t buff = new_string_buff();
    param_write_string(&buff, param, indent);
    return buff.str;
}
void param_write_string_or_file(void * out, parameter_t * param, int indent, bool to_file) {
    line_write_string(out, indent, "name: ", param->name, to_file);
    line_write_string(out, indent, "value: ", param->value, to_file);
}
void param_write_string(string_buff_t * buff, parameter_t * param, int indent) {
    param_write_string_or_file(buff, param, indent, false);
}
void param_write_file(FILE * file, parameter_t * param, int indent) {
    param_write_string_or_file(file, param, indent, true);
}
void param_free(parameter_t * param) {
    free(param->name);
    free(param->value);
}



/// Stage construction code

stage_t new_stage(uint64_t cycle, char identifier, char * name) {
    return (stage_t){ .cycle = cycle, .identifier = identifier, .name = name, .color = 7, .n_params = 0, .params = NULL };
}
parameter_t * stage_push_param(stage_t * stage, parameter_t param) {
    uint32_t new_n_params = stage->n_params + 1;
    if (new_n_params > stage->cap_params) {
        stage->cap_params = MAX(((stage->cap_params / 2) * 3), new_n_params);
        stage->params = realloc(stage->params, sizeof(parameter_t) * stage->cap_params);
    }
    stage->params[stage->n_params] = param;
    stage->n_params = new_n_params;
    return &stage->params[new_n_params-1];
}
parameter_t * stage_add_param(stage_t * stage, parameter_t param, uint32_t index) {
    if (index < stage->n_params) {
        // Free existing parameter
        param_free(&stage->params[index]);
        // Add to existing slot
        stage->params[index] = param;
    } else {
        uint32_t new_n_params = index + 1;
        // Reallocate space
        if (new_n_params > stage->cap_params) {
            stage->cap_params = MAX(((stage->cap_params / 2) * 3), new_n_params);
            stage->params = realloc(stage->params, sizeof(parameter_t) * stage->cap_params);
        }
        // Fill in in-between spaces
        for(uint32_t i = stage->n_params; i < index; i++) {
            stage->params[i] = new_param(NULL, NULL);
        }
        // Add to new slot
        stage->params[index] = param;
        stage->n_params = new_n_params;
    }
    return &stage->params[index];
}
void stage_set_params(stage_t * stage, uint32_t n_params, parameter_t * params) {
    for(uint32_t i = 0; i < stage->n_params; i++) {
        param_free(&stage->params[i]);
    }
    free(stage->params);
    stage->params = params;
    stage->n_params = n_params;
    stage->cap_params = n_params;
}
void stage_set(stage_t * stage, uint64_t cycle, char identifier, char * name) {
    stage->cycle = cycle;
    stage->identifier = identifier;
    free(stage->name);
    stage->name = name;
}
char * stage_to_string(stage_t * stage, int indent) {
    string_buff_t buff = new_string_buff();
    stage_write_string(&buff, stage, indent);
    return buff.str;
}
void stage_write_string_or_file(void * out, stage_t * stage, int indent, bool to_file) {
    line_write_num(out, indent, "cycle: ", stage->cycle, to_file);
    line_write_string(out, indent, "name: ", stage->name, to_file);
    line_write_char(out, indent, "id: ", stage->identifier, to_file);
    line_write_num(out, indent, "color: ", stage->color, to_file);
    if (stage->n_params > 0) {
        line_write_string(out, indent, "params: ", NULL, to_file);
        for (uint32_t i = 0; i < stage->n_params; i++) {
            line_write_string(out, indent, "- ", NULL, to_file);
            param_write_string_or_file(out, &stage->params[i], indent+2, to_file);
        }
    }
}
void stage_write_string(string_buff_t * buff, stage_t * stage, int indent) {
    stage_write_string_or_file(buff, stage, indent, false);
}
void stage_write_file(FILE * file, stage_t * stage, int indent) {
    stage_write_string_or_file(file, stage, indent, true);
}
void stage_free(stage_t * stage) {
    free(stage->name);
    for(uint32_t i = 0; i < stage->n_params; i++) {
        param_free(&stage->params[i]);
    }
    free(stage->params);
}



/// Instruction construction code

instruction_t new_inst(uint8_t valid, uint8_t tid, uint64_t pc, char * instruction) {
    return (instruction_t){ .valid = valid, .tid = tid, .pc = pc, .instruction = instruction,
                            .stages = NULL, .n_stages = 0, .cap_stages = 0 };
}
stage_t * inst_push_stage(instruction_t * inst, stage_t stage) {
    uint32_t new_n_stages = inst->n_stages + 1;
    if (new_n_stages > inst->cap_stages) {
        inst->cap_stages = MAX(((inst->cap_stages / 2) * 3), new_n_stages);
        inst->stages = realloc(inst->stages, sizeof(stage_t) * inst->cap_stages);
    }
    inst->stages[inst->n_stages] = stage;
    inst->n_stages = new_n_stages;
    return &inst->stages[new_n_stages-1];
}
stage_t * inst_add_stage(instruction_t * inst, stage_t stage, uint32_t index) {
    if (index < inst->n_stages) {
        // Free existing stage
        stage_free(&inst->stages[index]);
        // Add to existing slot
        inst->stages[index] = stage;
    } else {
        uint32_t new_n_stages = index + 1;
        // Reallocate space
        if (new_n_stages > inst->cap_stages) {
            inst->cap_stages = MAX(((inst->cap_stages / 2) * 3), new_n_stages);
            inst->stages = realloc(inst->stages, sizeof(stage_t) * inst->cap_stages);
        }
        // Fill in in-between spaces
        for(uint32_t i = inst->n_stages; i < index; i++) {
            inst->stages[i] = new_stage(0, ' ', NULL);
        }
        // Add to new slot
        inst->stages[index] = stage;
        inst->n_stages = new_n_stages;
    }
    return &inst->stages[index];
}
void inst_set_stages(instruction_t * inst, uint32_t n_stages, stage_t * stages) {
    for(uint32_t i = 0; i < inst->n_stages; i++) {
        stage_free(&inst->stages[i]);
    }
    free(inst->stages);
    inst->stages = stages;
    inst->n_stages = n_stages;
    inst->cap_stages = n_stages;
}
void inst_set(instruction_t * inst, uint8_t valid, uint8_t tid, uint64_t pc, char * instruction) {
    inst->valid = valid;
    inst->tid = tid;
    inst->pc = pc;
    free(inst->instruction);
    inst->instruction = instruction;
}
char * inst_to_string(instruction_t * inst, int indent) {
    string_buff_t buff = new_string_buff();
    inst_write_string(&buff, inst, indent);
    return buff.str;
}
void inst_write_string_or_file(void * out, instruction_t * inst, int indent, bool to_file) {
    char pc_str[64];
    snprintf(pc_str, 64, "0x%08" PRIX64 "", inst->pc);
    //line_write_num(buff, indent, "valid: ", inst->valid);
    line_write_num(out, indent, "tid: ", inst->tid, to_file);
    line_write_string(out, indent, "pc: ", pc_str, to_file);
    line_write_string_quoted(out, indent, "text: ", inst->instruction, to_file);
    if (inst->n_stages > 0) {
        line_write_string(out, indent, "stages:", NULL, to_file);
        for (uint32_t i = 0; i < inst->n_stages; i++) {
            line_write_string(out, indent, "- ", NULL, to_file);
            stage_write_string_or_file(out, &inst->stages[i], indent+2, to_file);
        }
    }
}
void inst_write_string(string_buff_t * buff, instruction_t * inst, int indent) {
    inst_write_string_or_file(buff, inst, indent, false);
}
void inst_write_file(FILE * file, instruction_t * inst, int indent) {
    inst_write_string_or_file(file, inst, indent, true);
}
void inst_free(instruction_t * inst) {
    free(inst->instruction);
    for (uint32_t i = 0; i < inst->n_stages; i++) {
        stage_free(&inst->stages[i]);
    }
    free(inst->stages);
}



/// Trace construction code

trace_t new_trace() {
    return (trace_t){ .insts = NULL, .n_insts = 0, .cap_insts = 0, .starting_inst = 0, .prev_output_inst = 0, .wrote_header = false };
}
instruction_t * trace_push_inst(trace_t * trace, instruction_t inst) {
    uint64_t new_n_insts = trace->n_insts + 1;
    if (new_n_insts > trace->cap_insts) {
        trace->cap_insts = MAX(((trace->cap_insts / 2) * 3), new_n_insts);
        trace->insts = realloc(trace->insts, sizeof(instruction_t *) * trace->cap_insts);
    }
    instruction_t * inst_slot = malloc(sizeof(instruction_t));
    *inst_slot = inst;
    trace->insts[trace->n_insts] = inst_slot;
    trace->n_insts = new_n_insts;
    return trace->insts[trace->n_insts-1];
}
instruction_t * trace_add_inst(trace_t * trace, instruction_t inst, uint64_t index) {
    instruction_t * inst_slot = malloc(sizeof(instruction_t));
    *inst_slot = inst;
    return trace_add_inst_ptr(trace, inst_slot, index);
}
instruction_t * trace_add_inst_ptr(trace_t * trace, instruction_t * inst, uint64_t index) {
    //printf("Adding instruction " PRIu64 "\n", index);
    assert(index >= trace->starting_inst);
    uint64_t array_index = index - trace->starting_inst;
    if (array_index < trace->n_insts) {
        // Free existing instruction
        if (trace->insts[array_index] != NULL) {
            inst_free(trace->insts[array_index]);
            free(trace->insts[array_index]);
        }
        // Add to existing slot
        trace->insts[array_index] = inst;
    } else {
        uint64_t new_n_insts = array_index + 1;
        // Reallocate space
        if (new_n_insts > trace->cap_insts) {
            trace->cap_insts = MAX(((trace->cap_insts / 2) * 3), new_n_insts);
            trace->insts = realloc(trace->insts, sizeof(instruction_t *) * trace->cap_insts);
        }
        // Fill in in-between spaces
        for(uint64_t i = trace->n_insts; i < array_index; i++) {
            trace->insts[i] = NULL;
        }
        // Add to new slot
        trace->insts[array_index] = inst;
        trace->n_insts = new_n_insts;
    }
    return trace->insts[array_index];
}
void trace_set_insts(trace_t * trace, uint64_t n_insts, instruction_t ** insts) {
    for(uint64_t i = 0; i < trace->n_insts; i++) {
        if (trace->insts[i] != NULL) {
            inst_free(trace->insts[i]);
            free(trace->insts[i]);
        }
    }
    free(trace->insts);
    trace->insts = insts;
    trace->n_insts = n_insts;
    trace->cap_insts = n_insts;
}
char * trace_to_string(trace_t * trace, int indent) {
    string_buff_t buff = new_string_buff();
    trace_write_string(&buff, trace, indent);
    if (strlen(buff.str) == 0) {
        free(buff.str);
        return NULL;
    } else {
        return buff.str;
    }
}
void trace_write_string_or_file(void * out, trace_t * trace, int indent, bool to_file) {
    // Only write header on first call
    if (trace->wrote_header == false) {
        line_write_string(out, indent, "insts:", NULL, to_file);
        trace->wrote_header = true;
    }
    if (trace->n_insts > 0) {
        // Write instructions until empty slot is reached
        for (uint64_t i = 0; ; i++) {
            if (i == trace->n_insts || trace->insts[i] == NULL) {
                //printf("Shifting array\n");
                //printf("  Old starting point: %" PRIu64 "\n", trace->starting_inst);
                // Shift array left to fill slots which have been written
                for(uint64_t j = 0; j < i; j++) {
                    inst_free(trace->insts[j]);
                }
                memcpy(&trace->insts[0], &trace->insts[i], (trace->n_insts - i) * sizeof(instruction_t *));
                trace->n_insts -= i;
                trace->starting_inst += i;
                //printf("  New starting point: %" PRIu64 "\n", trace->starting_inst);
                break;
            } else {
                instruction_t * inst_slot = trace->insts[i];
                //printf("Outputing instruction %" PRIu64 "\n", i + trace->starting_inst);
                assert(i + trace->starting_inst == trace->prev_output_inst + 1 || (trace->prev_output_inst == 0 && i + trace->starting_inst == 0));
                trace->prev_output_inst = i + trace->starting_inst;
                line_write_string(out, indent, "- ", NULL, to_file);
                inst_write_string_or_file(out, inst_slot, indent+2, to_file);
            }
        }
    }
}
void trace_write_string(string_buff_t * buff, trace_t * trace, int indent) {
    trace_write_string_or_file(buff, trace, indent, false);
}
void trace_write_file(FILE * file, trace_t * trace, int indent) {
    trace_write_string_or_file(file, trace, indent, true);
}
void trace_free(trace_t * trace) {
    for(uint64_t i = 0; i < trace->n_insts; i++) {
        if (trace->insts[i] != NULL) {
            inst_free(trace->insts[i]);
            free(trace->insts[i]);
        }
    }
    free(trace->insts);
}










