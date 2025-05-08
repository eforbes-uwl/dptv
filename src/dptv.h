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

#ifndef _DPTV_H_
#define _DPTV_H_

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#define PROJECT_NAME "Dual Pipetrace Viewer"


typedef struct parameter_type {
    char *name;
    char *value;
} parameter_t;

typedef struct stage_type {
    uint64_t cycle;
    char identifier;
    char * id_str;
    uint32_t color;
    char * name;
    struct parameter_type * params;
    uint32_t n_params;
} stage_t;

typedef struct instruction_type {
    uint8_t valid; // to distinguish between instructions, and padding used to align instructions from two traces
    uint8_t tid;
    uint64_t pc;
    char * pc_text;
    char * instruction;
    uint8_t committed;
    struct stage_type * stages;
    uint32_t n_stages;
} instruction_t;

typedef struct trace_type {
    char *name;
    struct instruction_type * insts;
    uint64_t n_insts;
} trace_t;


extern bool quit;

#endif
