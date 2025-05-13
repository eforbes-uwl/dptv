#ifndef _DPTL_H_
#define _DPTL_H_

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct parameter_type {
    char *name;
    char *value;
} parameter_t;

typedef struct stage_type {
    uint64_t cycle;
    char identifier;
    uint32_t color;
    char * name;
    struct parameter_type * params;
    uint32_t n_params;
    uint32_t cap_params;
} stage_t;

typedef struct instruction_type {
    uint8_t valid; // to distinguish between instructions, and padding used to align instructions from two traces
    uint8_t tid;
    uint64_t pc;
    char * instruction;
    struct stage_type * stages;
    uint32_t n_stages;
    uint32_t cap_stages;
} instruction_t;

typedef struct trace_type {
    struct instruction_type ** insts;
    uint64_t n_insts;
    uint64_t cap_insts;
    uint64_t starting_inst;
    uint64_t prev_output_inst;
    bool wrote_header;
} trace_t;


typedef struct string_buff {
    size_t cap;
    char * str;
} string_buff_t;



/*
Create a string buffer struct.
This struct will automatically reallocate if adding more data to it would exceed the allocated space.
*/
string_buff_t new_string_buff();
/*
Add a string to a string buffer.
*/
void extend_string_buff(string_buff_t * buff, const char * add);
/*
Add a number to a string buffer.
*/
void extend_string_buff_num(string_buff_t * buff, uint64_t num);
/*
Free a string buffer.
*/
void free_string_buff(string_buff_t buff);


/*
Create a new parameter with a given name and vaue.
If 'param_set' or 'param_free' is run later or if the parameter is added to a stage which is freed
then 'name' and 'value' should be produced from a call to 'malloc' or 'strdup'.
*/
parameter_t new_param(char * name, char * value);
/*
Set the data for this parameter.
This will free the previous value for 'name' and 'value'.
*/
void param_set(parameter_t * param, char * name, char * value);
/*
Output the parameter data to a string in yaml format.
This function does not need to be called if the parameter is being added to a stage struct.
*/
char * param_to_string(parameter_t * param, int indent);
/*
Output the parameter data to a string in yaml format.
This function does not need to be called if the parameter is being added to a stage struct.
*/
void param_write_string(string_buff_t * str, parameter_t * param, int ident);
/*
Output the parameter data to a file in yaml format.
This function does not need to be called if the parameter is being added to a stage struct.
*/
void param_write_file(FILE * file, parameter_t * param, int indent);
/*
Free the parameter name and value.
This does not attempt to free the 'param' pointer.
This function does not need to be called if the parameter is being added to a stage struct.
*/
void param_free(parameter_t * param);


/*
Create a new stage with a given identifier and name.
If 'stage_set' or 'stage_free' is run later or if the stage is added to an instruction which is freed
then 'name' should be produced from a call to 'malloc' or 'strdup'.
*/
stage_t new_stage(uint64_t cycle, char identifier, char * name);
/*
Add a parameter to the end of the stage list for this instruction.
*/
parameter_t * stage_push_param(stage_t * stage, parameter_t param);
/*
Add a parameter to any index in the stage list for this stage.
If a parameter already existed at that index then the old parameter will be freed.
If the index of the new parameter is greater than the current list length then padding parameters
will be added as needed with an empty name and value.
*/
parameter_t * stage_add_param(stage_t * stage, parameter_t param, uint32_t index);
/*
Free the current parameter list and replace it with a new one.
Each parameter will be freed via a call to 'param_free'.
The parameter list should be an array of parameters.
If this function or stage_free is run later then the pointer 'params' should be produced
from a call to 'malloc'.
*/
void stage_set_params(stage_t * stage, uint32_t n_params, parameter_t * params);
/*
Set the data for this stage.
This will free the previous value for 'name'.
*/
void stage_set(stage_t * stage, uint64_t cycle, char identifier, char * name);
/*
Output the stage data to a string in yaml format.
This function does not need to be called if the stage is being added to an instruction struct.
*/
char * stage_to_string(stage_t * stage, int indent);
/*
Output the stage data to a string in yaml format.
This function does not need to be called if the stage is being added to an instruction struct.
*/
void stage_write_string(string_buff_t * str, stage_t * stage, int indent);
/*
Output the stage data to a file in yaml format.
This function does not need to be called if the stage is being added to an instruction struct.
*/
void stage_write_file(FILE * file, stage_t * stage, int indent);
/*
Free the parameter list and stage name.
This does not attempt to free the 'stage' pointer.
This function does not need to be called if the stage is being added to an instruction struct.
*/
void stage_free(stage_t * stage);


/*
Create a new instruction with a given thread id, pc, and instruction text.
The instruction text can be NULL.
If 'inst_set' or 'inst_free' is run later or if the instruction is added to a trace which is freed
then 'instruction' should be produced from a call to 'malloc' or 'strdup'.
*/
instruction_t new_inst(uint8_t valid, uint8_t tid, uint64_t pc, char * instruction);
/*
Add a stage to the end of the stage list for this instruction.
*/
stage_t * inst_push_stage(instruction_t * inst, stage_t stage);
/*
Add a stage to any index in the stage list for this instruction.
If a stage already existed at that index then the old stage will be freed.
If the index of the new stage is greater than the current list length then padding stages
will be added as needed with an empty stage identifier and name.
*/
stage_t * inst_add_stage(instruction_t * inst, stage_t stage, uint32_t index);
/*
Free the current stage list and replace it with a new one.
Each stage will be freed via a call to 'stage_free'.
The stage list should be an array of stages.
If this function or inst_free is run later then the pointer 'stages' should be produced
from a call to 'malloc'.
*/
void inst_set_stages(instruction_t * inst, uint32_t n_stages, stage_t * stages);
/*
Set the data for this instruction.
This will free the previous value for 'instruction'.
*/
void inst_set(instruction_t * inst, uint8_t valid, uint8_t tid, uint64_t pc, char * instruction);
/*
Output the instruction data to a string in yaml format.
This function does not need to be called if the instruction is being added to a trace struct.
*/
char * inst_to_string(instruction_t * inst, int indent);
/*
Output the instruction data to a string in yaml format.
This function does not need to be called if the instruction is being added to a trace struct.
*/
void inst_write_string(string_buff_t * str, instruction_t * inst, int indent);
/*
Output the instruction data to a file in yaml format.
This function does not need to be called if the instruction is being added to a trace struct.
*/
void inst_write_file(FILE * file, instruction_t * inst, int indent);
/*
Free the stage list and instruction text.
This does not attempt to free the 'inst' pointer.
This function does not need to be called if the instruction is being added to a trace struct.
*/
void inst_free(instruction_t * inst);


/*
Create a new trace with no instructions.
*/
trace_t new_trace();
/*
Push an instruction to the end of the instruction list for a trace.
This should NOT be used alongside the trace_add functions.
*/
instruction_t * trace_push_inst(trace_t * trace, instruction_t inst);
/*
Add an instruction to the trace at a specific index.
If that index already has an instruction associated with it, the previous instruction will be freed.
This should NOT be used alongside the trace_push_inst function.
*/
instruction_t * trace_add_inst(trace_t * trace, instruction_t inst, uint64_t index);
/*
Like trace_add_inst, but a pointer to an instruction struct is supplied instead.
The data pointed to should have been created through a call to 'malloc'.
*/
instruction_t * trace_add_inst_ptr(trace_t * trace, instruction_t * inst, uint64_t index);
/*
Free the current instruction list and replace it with a new one.
Each instruction will be freed via a call to 'inst_free'.
The instruction list should be an array of pointers to instructions.
All data behind pointers should have been created through a call to 'malloc'.
*/
void trace_set_insts(trace_t * trace, uint64_t n_insts, instruction_t ** inst);
/*
Like trace_write_string, but outputs a standard c string. This string should be freed after it has been used.
*/
char * trace_to_string(trace_t * trace, int indent);
/*
Write the trace yaml information to a string buffer.
If there is a gap in the instruction list (ex: instructions 0, 1, and 5 have been added but not 2, 3, or 4),
then only the first continuous sequence of instructions will be output (ex: instructions 0 and 1). Subsequent
calls to trace_write_string will attempt to continue outputting where to last call left off if the missing
instructions have been filled in.
*/
void trace_write_string(string_buff_t * str, trace_t * trace, int indent);
/*
Like trace_write_string, but writes directly to a file.
*/
void trace_write_file(FILE * file, trace_t * trace, int indent);
/*
Free a trace structure after it's finished.
This function will try to free all instructions pointed to by the array as well as free the array.
This will not attempt to free the trace struct itself.
*/
void trace_free(trace_t * trace);


#ifdef __cplusplus
}
#endif


#endif