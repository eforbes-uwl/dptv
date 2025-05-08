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
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dptv.h"
#include "options.h"

// "private" helper function prototypes
static void report_cmd_error();
static void init_options();

// "private" variables
static int cmd_errno = CMD_ERR_UNINITIALIZED;
static int cmd_err_idx = -1;
static char *cmd_err_arg = NULL;

// "public" variables
options_t *OPTIONS = NULL;
colors_t *COLORS = NULL;

// allocate options object, and assign default values
static void init_options() {
    OPTIONS = (options_t*) malloc(sizeof(options_t));
    assert(OPTIONS);

    OPTIONS->win_width = 1024;
    OPTIONS->win_height = 768;
    OPTIONS->win_xpos = 0;
    OPTIONS->win_ypos = 0;
    OPTIONS->win_title = strdup(PROJECT_NAME);
    // Set font path to a default value if DPTVFONT is defined
    char * font_env = getenv("DPTVFONT");
    if (font_env) {
        const char * font_text = "/LiberationMono-Bold.ttf";
        int font_text_len = strlen(font_text);
        int size = strlen(font_env);
        size += font_text_len;
        char * buff = (char*) malloc(sizeof(char) * (size + 2));
        bzero(buff, size);
        strncpy(buff, font_env, (size - font_text_len));
        strncat(buff, font_text, font_text_len+1);
        OPTIONS->font_path = buff;
        //OPTIONS->font_path = strdup(font_env);
    }
    else {
        OPTIONS->font_path = NULL;
    }
    OPTIONS->num_traces = 0;
    OPTIONS->trace_filenames = NULL;
    OPTIONS->commit_stage = strdup("retire");
    OPTIONS->lines = NULL;
    OPTIONS->scale = malloc(sizeof(double) * 2);
    OPTIONS->scale[0] = 1;
    OPTIONS->scale[1] = 1;
    OPTIONS->main_trace = 0;
    OPTIONS->trace_remove_squash = 0;
    OPTIONS->trace_disable_dummy = 0;
    OPTIONS->trace_disable_cutoff = 0;
    OPTIONS->arg_command = NULL;
    OPTIONS->instr_window_width = 0;

    COLORS = (colors_t*) malloc(sizeof(colors_t));
    assert(COLORS);

    COLORS->bg =         (gfx_color_t){0, (SDL_Color){0x05, 0x05, 0x0C}};
    COLORS->ui =         (gfx_color_t){0, (SDL_Color){0x00, 0xAE, 0xF0}};
    COLORS->trace_a =    (gfx_color_t){0, (SDL_Color){0x00, 0xAE, 0xF0}};
    COLORS->trace_b =    (gfx_color_t){0, (SDL_Color){0xB7, 0x00, 0xE5}};
    COLORS->highlight =  (gfx_color_t){0, (SDL_Color){0xF0, 0xE0, 0x80}};
    COLORS->cur_search = (gfx_color_t){0, (SDL_Color){0xFF, 0xFF, 0xFF}};
}

// Figure out the font path
char * get_font_path(char * exe_path) {
    const char * font_text = "../assets/LiberationMono-Bold.ttf";
    int font_text_len = strlen(font_text);
    int len = strlen(exe_path);
    if (len < 7) {
        return NULL;
    }
    // Verify the last characters of the path are "dptview"
    if (strcmp(exe_path+len-7, "dptview") != 0) {
        return NULL;
    }
    int new_len = len - 7 + font_text_len;
    char * new_str = malloc(sizeof(char) * (new_len + 1));
    strncpy(new_str, exe_path, len-7);
    strcpy(new_str+len-7, font_text);
    return new_str;
}

// parse the command line arguments
int process_cmd_line(int argc, char *argv[]){
    init_options();
    cmd_err_arg = strdup("");

    int i;
    int ret = CMD_OPTIONS_OK;

    printf("processing command line args...\n");
    fflush(stdout);
    
    if (argc < 1) {
        return CMD_ERR_NO_ARGS;
    }
    
    OPTIONS->arg_command = strdup(argv[0]);

    for (i=1;i<argc;i++){
        if (argv[i][0] == '-'){
            if (strcmp(argv[i],"-help") == 0){
                ret = CMD_HELP;
                break;
            }
            else if (strcmp(argv[i],"-width") == 0 || strcmp(argv[i],"-w") == 0){
                if (((i+1)>=argc) || (argv[i+1][0] == '-')){
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                if ((argv[i+1][0]<'0') || (argv[i+1][0]>'9')){
                    cmd_err_idx = i+1;
                    ret = CMD_ERR_BAD_VALUE;
                    break;
                }
                OPTIONS->win_width = atoi(argv[i+1]);
                ++i;
            }
            else if (strcmp(argv[i],"-height") == 0 || strcmp(argv[i],"-h") == 0){
                if (((i+1)>=argc) || (argv[i+1][0] == '-')){
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                if ((argv[i+1][0]<'0') || (argv[i+1][0]>'9')){
                    cmd_err_idx = i+1;
                    ret = CMD_ERR_BAD_VALUE;
                    break;
                }
                OPTIONS->win_height = atoi(argv[i+1]);
                ++i;
            }
            else if (strcmp(argv[i],"-posx") == 0 || strcmp(argv[i],"-x") == 0){
                if (((i+1)>=argc) || (argv[i+1][0] == '-')){
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                if ((argv[i+1][0]<'0') || (argv[i+1][0]>'9')){
                    cmd_err_idx = i+1;
                    ret = CMD_ERR_BAD_VALUE;
                    break;
                }
                OPTIONS->win_xpos = atoi(argv[i+1]);
                ++i;
            }
            else if (strcmp(argv[i],"-posy") == 0 || strcmp(argv[i],"-y") == 0){
                if (((i+1)>=argc) || (argv[i+1][0] == '-')){
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                if ((argv[i+1][0]<'0') || (argv[i+1][0]>'9')){
                    cmd_err_idx = i+1;
                    ret = CMD_ERR_BAD_VALUE;
                    break;
                }
                OPTIONS->win_ypos = atoi(argv[i+1]);
                ++i;
            }
            else if (strcmp(argv[i],"-commit") == 0){
                if (((i+1)>=argc) || (argv[i+1][0] == '-')){
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                OPTIONS->commit_stage = strdup(argv[i+1]);
                ++i;
            }
            else if (strcmp(argv[i],"-line") == 0 || strcmp(argv[i],"-l") == 0){
                if (((i+1)>=argc) || (argv[i+1][0] == '-')) {
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                if (OPTIONS->lines == NULL) {
                    OPTIONS->lines = malloc(sizeof(line_sec_t));
                    *OPTIONS->lines = (line_sec_t){argv[i+1][0], 255, NULL};
                } else {
                    line_sec_t* cur_line = OPTIONS->lines;
                    while(cur_line->next != NULL) {
                        cur_line = cur_line->next;
                    }
                    cur_line->next = malloc(sizeof(line_sec_t));
                    *cur_line->next = (line_sec_t){argv[i+1][0], 255, NULL};
                }
                ++i;
            }
            else if (strcmp(argv[i],"-fcolor") == 0 || strcmp(argv[i],"-fc") == 0){
                if (((i+3)>=argc) || (argv[i+1][0] == '-') || (argv[i+2][0] == '-') || (argv[i+3][0] == '=')) {
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                int red = (int)strtol(argv[i+1], NULL, 10);
                int green = (int)strtol(argv[i+2], NULL, 10);
                int blue = (int)strtol(argv[i+3], NULL, 10);
                COLORS->trace_a = (gfx_color_t){0, (SDL_Color){red, green, blue}};
                COLORS->ui = (gfx_color_t){0, (SDL_Color){red, green, blue}};
                i += 3;
            }
            else if (strcmp(argv[i],"-bcolor") == 0 || strcmp(argv[i],"-bc") == 0){
                if (((i+3)>=argc) || (argv[i+1][0] == '-') || (argv[i+2][0] == '-') || (argv[i+3][0] == '=')) {
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                int red = (int)strtol(argv[i+1], NULL, 10);
                int green = (int)strtol(argv[i+2], NULL, 10);
                int blue = (int)strtol(argv[i+3], NULL, 10);
                COLORS->bg = (gfx_color_t){0, (SDL_Color){red, green, blue}};
                i += 3;
            }
            else if (strcmp(argv[i],"-hcolor") == 0 || strcmp(argv[i],"-hc") == 0){
                if (((i+3)>=argc) || (argv[i+1][0] == '-') || (argv[i+2][0] == '-') || (argv[i+3][0] == '=')) {
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                int red = (int)strtol(argv[i+1], NULL, 10);
                int green = (int)strtol(argv[i+2], NULL, 10);
                int blue = (int)strtol(argv[i+3], NULL, 10);
                COLORS->highlight = (gfx_color_t){0, (SDL_Color){red, green, blue}};
                i += 3;
            }
            else if (strcmp(argv[i],"-ocolor") == 0 || strcmp(argv[i],"-oc") == 0){
                if (((i+3)>=argc) || (argv[i+1][0] == '-') || (argv[i+2][0] == '-') || (argv[i+3][0] == '=')) {
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                int red = (int)strtol(argv[i+1], NULL, 10);
                int green = (int)strtol(argv[i+2], NULL, 10);
                int blue = (int)strtol(argv[i+3], NULL, 10);
                COLORS->trace_b = (gfx_color_t){0, (SDL_Color){red, green, blue}};
                i += 3;
            }
            else if (strcmp(argv[i],"-freq") == 0 || strcmp(argv[i],"-f") == 0) {
                if (((i+2)>=argc) || (argv[i+1][0] == '-') || (argv[i+2][0] == '-')) {
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                // Set the frequency scale
                double scaleA = strtod(argv[i+1], NULL);
                double scaleB = strtod(argv[i+2], NULL);
                if (scaleA > scaleB) {
                    OPTIONS->scale[0] = 1;
                    OPTIONS->scale[1] = scaleA / scaleB;
                    OPTIONS->main_trace = 0;
                } else {
                    OPTIONS->scale[0] = scaleB / scaleA;
                    OPTIONS->scale[1] = 1;
                    OPTIONS->main_trace = 1;
                }
                i += 2;
            }
            else if (strcmp(argv[i],"-rsquash") == 0 || strcmp(argv[i],"-rs") == 0) {
                OPTIONS->trace_remove_squash = true;
            }
            else if (strcmp(argv[i],"-ddummy") == 0 || strcmp(argv[i],"-dd") == 0) {
                OPTIONS->trace_disable_dummy = true;
            }
            else if (strcmp(argv[i],"-dcutoff") == 0 || strcmp(argv[i],"-dc") == 0) {
                OPTIONS->trace_disable_cutoff = true;
            }
            else if (strcmp(argv[i],"-fontfile") == 0 || strcmp(argv[i],"-ff") == 0) {
                if (((i+1)>=argc) || (argv[i+1][0] == '-')) {
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                if (OPTIONS->font_path) {
                    free(OPTIONS->font_path);
                }
                OPTIONS->font_path = strdup(argv[i+1]);
                ++i;
            }
            else if (strcmp(argv[i],"-iwidth") == 0 || strcmp(argv[i],"-iw") == 0) {
                if (((i+1)>=argc) || (argv[i+1][0] == '-')) {
                    cmd_err_idx = i;
                    ret = CMD_ERR_BAD_ARG;
                    break;
                }
                OPTIONS->instr_window_width = strtol(argv[i+1], NULL, 10);
                ++i;
            }
            else {
                cmd_err_idx = i;
                ret = CMD_ERR_BAD_OPTION;
                break;
            }
        }
        else{
            // this arg must be a file name
            int j;
            char **new_filenames;

            // create a new array of char buffers
            new_filenames = (char**)malloc(sizeof(char*)*(OPTIONS->num_traces+1));
            
            // copy the file names from the current buffers to the new
            for (j=0;j<OPTIONS->num_traces;j++){
                new_filenames[j] = strdup(OPTIONS->trace_filenames[j]);
            }
            // add the new file name to the new buffers
            new_filenames[OPTIONS->num_traces] = strdup(argv[i]);

            // free up memory from the current buffers
            for (j=0;j<OPTIONS->num_traces;j++){
                free(OPTIONS->trace_filenames[j]);
                OPTIONS->trace_filenames[j] = NULL;
            }
            free(OPTIONS->trace_filenames);
            
            // copy new buffer to current buffer, update count
            OPTIONS->trace_filenames = new_filenames;
            OPTIONS->num_traces++;

        }
    }
    
    if (OPTIONS->font_path == NULL) {
        OPTIONS->font_path = get_font_path(OPTIONS->arg_command);
        if (OPTIONS->font_path == NULL) {
            ret = CMD_ERR_FONT_PATH;
            cmd_err_idx = 0;
        } else {
            fprintf(stdout, "font path: %s\n", OPTIONS->font_path);
            fflush(stdout);
        }
    } else {
        fprintf(stdout, "font path: %s\n", OPTIONS->font_path);
        fflush(stdout);
    }

    if (ret != CMD_OPTIONS_OK){
        if (ret != CMD_HELP)
            cmd_err_arg = strdup(argv[cmd_err_idx]);
    }
    else if (OPTIONS->num_traces == 0){
        ret = CMD_ERR_NO_TRACES;
        cmd_err_idx = 0;
    }
    else if (OPTIONS->num_traces > 2){
        ret = CMD_ERR_TOO_MANY_TRACES;
        cmd_err_idx = 0;
    }
    cmd_errno = ret;
    report_cmd_error();

    fflush(stdout);
    
    return ret;
}

// dump error message for any bad command line arguments detected
static void report_cmd_error(){
    if (cmd_errno != CMD_OPTIONS_OK){
        switch(cmd_errno){
            case CMD_HELP:
                break;
            case CMD_ERR_UNINITIALIZED:
                fprintf(stderr,"options failed to initialize");
                break;
            case CMD_ERR_BAD_OPTION:
                fprintf(stderr,"ERROR parsing command line option %u \"%s\": ",cmd_err_idx,cmd_err_arg);
                fprintf(stderr,"unidentified option");
                break;
            case CMD_ERR_BAD_ARG:
                fprintf(stderr,"ERROR parsing command line option %u \"%s\": ",cmd_err_idx,cmd_err_arg);
                fprintf(stderr,"badly formed option");
                break;
            case CMD_ERR_BAD_VALUE:
                fprintf(stderr,"ERROR parsing command line option %u \"%s\": ",cmd_err_idx,cmd_err_arg);
                fprintf(stderr,"bad value");
                break;
            case CMD_ERR_NO_TRACES:
                fprintf(stderr,"ERROR parsing command line option %u \"%s\": ",cmd_err_idx,cmd_err_arg);
                fprintf(stderr,"no trace files specified");
                break;
            case CMD_ERR_TOO_MANY_TRACES:
                fprintf(stderr,"ERROR parsing command line option %u \"%s\": ",cmd_err_idx,cmd_err_arg);
                fprintf(stderr,"too many trace files specified");
                break;
            case CMD_ERR_NO_ARGS:
                fprintf(stderr,"no arguments provided");
                break;
            case CMD_ERR_FONT_PATH:
                fprintf(stderr,"ERROR: failed to derive font path from the executable path: %s\n", OPTIONS->arg_command);
                break;
            default:
                fprintf(stderr,"ERROR parsing command line option %u \"%s\": ",cmd_err_idx,cmd_err_arg);
                fprintf(stderr,"god knows, something is truely fucked up");
                break;
        }
    }
}

// dump the available command line options, usage info
void dump_cmd_opts(){
    fprintf(stderr,"\n\nusage: dptview [flags] <trace1> [<trace2>]\n\n");
    fprintf(stderr,"flags:\n");
    fprintf(stderr,"        -help                 Display this message and quit.\n");
    fprintf(stderr,"        -width <w>            Width of the output window (default 1024)\n");
    fprintf(stderr,"        -height <h>           Height of the output window (default 768)\n");
    fprintf(stderr,"        -posx <x>             X coordinate of the output window (default 0)\n");
    fprintf(stderr,"        -posy <y>             Y coordinate of the output window (default 0)\n");
    fprintf(stderr,"        -commit <n>           Name of the commit stage of the pipeline, as\n");
    fprintf(stderr,"                              it will appear in the trace file (default\n");
    fprintf(stderr,"                              \"retire\")\n");
    fprintf(stderr,"        -freq <f1> <f2>       Sets the frequency for each trace in two-trace\n");
    fprintf(stderr,"                              mode, slower frequency will be scaled relative\n");
    fprintf(stderr,"                              to the faster frequency\n");
    fprintf(stderr,"        -line <c>             Add a line connecting stages of type c when\n");
    fprintf(stderr,"                              zoomed out (default connect 'f' and 'R')\n");
    fprintf(stderr,"        -fcolor <r> <g> <b>   Set the color of the diplayed text from given\n");
    fprintf(stderr,"                              red, green, and blue values (default cyan)\n");
    fprintf(stderr,"        -bcolor <r> <g> <b>   Set the color of the background from given\n");
    fprintf(stderr,"                              red, green, and blue values (default black)\n");
    fprintf(stderr,"        -hcolor <r> <g> <b>   Set the color of the highlighted text from\n");
    fprintf(stderr,"                              red, green, and blue values (default yellow)\n");
    fprintf(stderr,"        -ocolor <r> <g> <b>   Set the color of the second trace text from\n");
    fprintf(stderr,"                              red, green, and blue values (default magenta)\n");
    fprintf(stderr,"        -rsquash              Remove all squashed instructions\n");
    fprintf(stderr,"        -ddummy               Disable dummy node insertion\n");
    fprintf(stderr,"        -dcutoff              Disable start/end cutoff\n");
    fprintf(stderr,"        -fontfile <file>      Sets which font file to use, overwriting the\n");
    fprintf(stderr,"                              default font file\n");
    fprintf(stderr,"        -iwidth <width>       Sets the width of the instruction window\n");
    fprintf(stderr,"\n<traceN>:\n");
    fprintf(stderr,"                              Name of each trace file, either one or two\n");
    fprintf(stderr,"                              traces, no default names.\n");
    fprintf(stderr,"\n\n");
}

