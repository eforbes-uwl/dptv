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

#include <SDL2/SDL.h>
#include "dptv.h"
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_ttf.h>
#include "trace_handler.h"
#include "gfx.h"
#include "event.h"
#include "search.h"
#include "options.h"
#include "help_text.h"



/*
 * Setup global variables
 */
SDL_Window *window = NULL;
SDL_Surface *screen_surface = NULL;
SDL_Renderer *stage_render = NULL;
int gfx_win_height;
int gfx_win_width;
TTF_Font* cp_mono;
SDL_Rect font_size;
int64_t x_pos = 0;
int64_t y_pos = 0;
double scale = 0.75;
const double line_cutoff = 0.2;
const double draw_instr_cutoff = 0.2;
const int line_draw_precision = 1;
bool info_on = true;
int info_width = 32;
int info_height = 32;
uint64_t x_check = 0;
uint64_t y_check = 0;
line_sec_t* gfx_lines;
int num_lines = 0;
bool first = true;
bool help = false;
int input_mode = INMODE_CAM;
bool force_snap = false;
int focus = 0;
int trace_off;
SDL_Surface** char_surfaces;
double txt_base_scale = 0.5;
bool is_dragging;
int drag_orig_mx;
int drag_orig_my;
int64_t drag_orig_x_pos;
uint64_t drag_orig_y_pos;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int rmask = 0xff000000;
    int gmask = 0x00ff0000;
    int bmask = 0x0000ff00;
    int amask = 0x000000ff;
#else
    int rmask = 0x000000ff;
    int gmask = 0x0000ff00;
    int bmask = 0x00ff0000;
    int amask = 0xff000000;
#endif




// Surfaces:
// - Main Surface
// - Instructions (left sidebar)
// - Stages (middle)
// - Stage Information (top right)
// - Info Text (bottom)
// - Help Text

SDL_Surface* instr_surf = NULL;
SDL_Surface* stage_surf = NULL;
SDL_Surface* info_surf = NULL;
SDL_Surface* cmd_surf = NULL;
SDL_Surface* help_surf = NULL;

const int default_instr_surf_width = 512-64;
int instr_surf_width;
const int cmd_surf_height = 32;


/*
 * Initialize GFX & Main loop
 */

void init_gfx() {
    printf("initializing gfx system...");
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        fprintf(stderr,"ERROR: failed to initialize video. %s\n",SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    if (OPTIONS->instr_window_width == 0) {
        instr_surf_width = default_instr_surf_width;
    } else {
        instr_surf_width = OPTIONS->instr_window_width;
    }
    
    gfx_win_height = OPTIONS->win_height;
    gfx_win_width = OPTIONS->win_width;
    
    window = SDL_CreateWindow(OPTIONS->win_title, OPTIONS->win_xpos, OPTIONS->win_ypos, OPTIONS->win_width, OPTIONS->win_height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr,"ERROR: failed to create window. %s\n",SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    // Initialize text renderer
    if (TTF_Init() == 1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(2);
    }
    cp_mono = TTF_OpenFont(OPTIONS->font_path, 32);
    if (cp_mono == NULL) {
        fprintf(stderr, "ERROR: failed to load font.");
        exit(EXIT_FAILURE);
    }
    font_size = gfx_get_font_size();
        
    screen_surface = SDL_GetWindowSurface(window);
    // Setup colors
    gfx_setup_int_color(screen_surface, &COLORS->bg);
    gfx_setup_int_color(screen_surface, &COLORS->ui);
    gfx_setup_int_color(screen_surface, &COLORS->trace_a);
    gfx_setup_int_color(screen_surface, &COLORS->trace_b);
    gfx_setup_int_color(screen_surface, &COLORS->highlight);
    gfx_setup_int_color(screen_surface, &COLORS->trace_a);
    
    // Setup surfaces
    gfx_win_resize(OPTIONS->win_width, OPTIONS->win_height);
    
    // Check if options defined the lines
    if (OPTIONS->lines == NULL) {
        // Default lines
        num_lines = 2;
        line_sec_t* line_a = malloc(sizeof(line_sec_t));    *line_a = (line_sec_t){'f', COLORS->trace_a.int_color, NULL};
        gfx_lines = malloc(sizeof(line_sec_t));             *gfx_lines = (line_sec_t){'R', COLORS->trace_b.int_color, line_a};
    } else {
        gfx_lines = OPTIONS->lines;
        // Count lines
        num_lines = 0;
        line_sec_t* cur_line = gfx_lines;
        while(cur_line != NULL) {
            num_lines ++;
            cur_line = cur_line->next;
        }
    }
    
    // Generate surfaces containing characters
    gfx_gen_char_surfs();
    
    gfx_reset();
    
    help = false;
    
    printf("GFX Initialized\n");
    
    SDL_StartTextInput();
}

void gfx_reset() {
    y_pos = 0;
    scale = 0.75;
    // Snap at start
    gfx_snap();
    gfx_snap_to_cycle(instr_surf_width, 0);
}


void gfx_update() {
    // Fill Rectangle
    SDL_FillRect(screen_surface, NULL, COLORS->ui.int_color);
    SDL_FillRect(instr_surf, NULL, COLORS->bg.int_color);
    SDL_FillRect(stage_surf, NULL, COLORS->bg.int_color);
    
    // Get stage checked
    cycle_pos_t cycle = gfx_get_mouse_stage_position(mx, my);
    x_check = cycle.x;      y_check = cycle.y;
    
    gfx_color_t color;
    // Draw data about traces
    for(int i = 0; i < OPTIONS->num_traces; i++) {
        // Get what position to begin drawing
        int64_t pos = y_pos - i;
        uint64_t y_draw = (pos + OPTIONS->num_traces - 1) / OPTIONS->num_traces;    // ceiling division
        uint64_t y_start = (pos + OPTIONS->num_traces) % OPTIONS->num_traces;
        int off = trace_off;
        // Set color based on drawn trace
        color = COLORS->trace_b;
        if (i == focus) {
            color = COLORS->trace_a;
            off = 0;
        }
        SDL_SetRenderDrawColor(stage_render, color.sdl_color.r, color.sdl_color.g, color.sdl_color.b, 0xFF);
        // Draw visible instructions and their stages
        gfx_draw_trace_pos(y_draw, color, scale, screen_surface->h / font_size.h / scale / OPTIONS->num_traces + 3, i, OPTIONS->scale[i], OPTIONS->num_traces, y_start, off);
    }
    
    // Draw box for scelected stage
    color = COLORS->trace_b;
    if (cycle.y % OPTIONS->num_traces == focus) {
        color = COLORS->trace_a;
    }
    gfx_draw_stage_box(color, cycle, stage_render);
    
    // Setup info surface
    setup_info(COLORS->ui);
    
    // Combine surfaces
    // Instruction sidebar
    SDL_Rect pos = (SDL_Rect){0, 0, instr_surf_width, screen_surface->h};
    SDL_BlitSurface(instr_surf, &pos, screen_surface, &pos);
    // Instruction separating line
    pos = (SDL_Rect){instr_surf_width-4, 0, 4, screen_surface->h};
    SDL_FillRect(screen_surface, &pos, COLORS->ui.int_color);
    // Stage area
    pos = (SDL_Rect){instr_surf_width, 0, stage_surf->w, screen_surface->h};
    SDL_BlitSurface(stage_surf, NULL, screen_surface, &pos);
    // Cmd info area
    pos = (SDL_Rect){0, screen_surface->h-cmd_surf->h, cmd_surf->w, cmd_surf->h};
    SDL_BlitSurface(cmd_surf, NULL, screen_surface, &pos);
    // Stage info
    if (info_on) {
        // Info separator
        pos = (SDL_Rect){screen_surface->w - info_width - 4, 0, info_width + 4, info_height + 4};
        SDL_FillRect(screen_surface, &pos, COLORS->ui.int_color);
        // Draw info surface
        pos = (SDL_Rect){screen_surface->w - info_width, 0, info_width, info_height};
        SDL_BlitSurface(info_surf, NULL, screen_surface, &pos);
    }
    // Help menu
    if (help) {
        pos = (SDL_Rect){(screen_surface->w - help_surf->w) / 2, (screen_surface->h - help_surf->h) / 2, help_surf->w, help_surf->h};
        SDL_BlitSurface(help_surf, NULL, screen_surface, &pos);
    }
    
    
    
    SDL_UpdateWindowSurface(window);

    first = false;
    
}


/*
 * Functions used for drawing in general
 */

SDL_Rect gfx_get_font_size() {
    SDL_Surface* txt_surface = TTF_RenderText_Solid(cp_mono, "X", COLORS->ui.sdl_color);
    SDL_Rect size = {0, 0, txt_surface->w * txt_base_scale, txt_surface->h * txt_base_scale};
    SDL_FreeSurface(txt_surface);
    return size;
}

void gfx_draw_box(gfx_color_t color, SDL_Rect pos, SDL_Renderer* rend) {
    SDL_SetRenderDrawColor(rend, color.sdl_color.r, color.sdl_color.g, color.sdl_color.b, 0xFF);
    SDL_RenderDrawLine(rend, pos.x, pos.y, pos.x+pos.w, pos.y);
    SDL_RenderDrawLine(rend, pos.x, pos.y, pos.x, pos.y+pos.h);
    SDL_RenderDrawLine(rend, pos.x, pos.y+pos.h, pos.x+pos.w, pos.y+pos.h);
    SDL_RenderDrawLine(rend, pos.x+pos.w, pos.y, pos.x+pos.w, pos.y+pos.h);
}

void gfx_draw_text_scaled(SDL_Surface* surf, const char* text, SDL_Rect* text_pos, SDL_Color color, double x_scale, double y_scale, int l_clip, int r_clip) {
    // Text Renderer breaks if passed an empty string
    if (text == NULL || *text == '\0') return;
    // Check bounds before we do any actual rendering
    if (l_clip != -1 && (text_pos->x < l_clip || text_pos->x >= r_clip)) return;
    
    SDL_Surface* txt_surface = TTF_RenderText_Solid(cp_mono, text, color);
    // Copy to intermediary surface
    SDL_Surface* temp = SDL_CreateRGBSurface(0, txt_surface->w, txt_surface->h, 32, 0, 0, 0, 0);
    SDL_FillRect(temp, NULL, COLORS->bg.int_color);
    SDL_BlitSurface(txt_surface, NULL, temp, NULL);
    // Display text scaled
    text_pos->w = txt_surface->w * x_scale * txt_base_scale;
    text_pos->h = txt_surface->h * y_scale * txt_base_scale;
    SDL_BlitScaled(temp, NULL, surf, text_pos);
    // Free created surfaces
    SDL_FreeSurface(temp);
    SDL_FreeSurface(txt_surface);
    // Increment position
    text_pos->x += text_pos->w;
}
void gfx_draw_char_scaled(SDL_Surface* surf, char c, SDL_Rect* text_pos, SDL_Color color, double x_scale, double y_scale, int l_clip, int r_clip) {
    if (c < ' ' || c > '~') return;
    if (l_clip != -1 && (text_pos->x < l_clip || text_pos->x >= r_clip)) return;
    // Use already-created surface containing the character we want to draw
    SDL_Surface* char_surf = char_surfaces[c-' '];
    // Set desired draw color
    SDL_SetSurfaceColorMod(char_surf, color.r, color.g, color.b);
    // Display character scaled
    text_pos->w = floor(((double)char_surf->w) * x_scale * txt_base_scale);
    text_pos->h = floor(((double)char_surf->h) * y_scale * txt_base_scale);
    SDL_BlitScaled(char_surf, NULL, surf, text_pos);
}
void gfx_gen_char_surfs() {
    char_surfaces = malloc(sizeof(SDL_Surface*) * ('~' - ' ' + 1));
    char c_ar[] = {' ', '\0'};
    SDL_Color color = {255, 255, 255, 255};
    for(char c = ' '; c <= '~'; c++) {
        // Draw this character
        c_ar[0] = c;
        SDL_Surface* txt_surf = TTF_RenderText_Solid(cp_mono, c_ar, color);
        // Copy to RGB Surface
        SDL_Surface* rgb_surf = SDL_CreateRGBSurface(0, txt_surf->w, txt_surf->h, 32, rmask, gmask, bmask, amask);
        SDL_BlitSurface(txt_surf, NULL, rgb_surf, NULL);
        // Save result
        char_surfaces[c-' '] = rgb_surf;
        free(txt_surf);
        //char_surfaces[c-' '] = txt_surf;
    }
}
void gfx_draw_text_colors_scaled(SDL_Surface* surf, const char* text, SDL_Rect* text_pos, gfx_color_t* colors, double x_scale, double y_scale, int l_clip, int r_clip) {
    if (text == NULL) return;
    for(int i = 0; true; i++) {
        // Draw character with correcsponding color
        char c = text[i];
        if (c == '\0') return;
        SDL_Color col = colors[i].sdl_color;
        gfx_draw_char_scaled(surf, c, text_pos, col, x_scale, y_scale, l_clip, r_clip);
        // Increment position
        text_pos->x += text_pos->w;
    }    
}
void gfx_draw_text_highlight_scaled(SDL_Surface* surf, const char* text, SDL_Rect* text_pos, gfx_color_t def, double x_scale, double y_scale, int l_clip, int r_clip, int sec, char* param_name) {
    gfx_color_t* colors = search_highlight(text, def, sec, param_name);
    gfx_draw_text_colors_scaled(surf, text, text_pos, colors, x_scale, y_scale, l_clip, r_clip);
    free(colors);
}



/*
 * Functions used for the main instruction window
 */

instruction_t* get_instr_at_pos(uint64_t pos, int trace) {
    if (pos >= TRACES[trace]->n_insts)  return NULL;
    instruction_t* instr = &TRACES[trace]->insts[pos];
    return instr;
}

instruction_t* get_valid_instr_at_pos(uint64_t pos, int trace) {
    instruction_t* inst = NULL;
    for(;; pos ++) {
        if (pos >= TRACES[trace]->n_insts)  return NULL;
        inst = get_instr_at_pos(pos, trace);
        if (inst->valid && inst->n_stages > 0)   break;
    }
    return inst;
}

stage_t* gfx_get_stage(uint64_t x, uint64_t y, int trace) {
    instruction_t* instr = get_instr_at_pos(y, trace);
    if (instr == NULL)  return NULL;
    // Get stage
    for(int s = 0; s < instr->n_stages; s++) {
        stage_t * stage = &instr->stages[s];
        if (x == stage->cycle || stage == NULL) {
            return stage;
        }
    }
    return NULL;
}

void gfx_draw_trace_pos(uint64_t y, gfx_color_t color, double scale, int num_disp, int trace, double trace_scale, int num_trace, int y_start, int off) {
    double draw_scale_x = scale * trace_scale;
    double draw_scale_y = scale;
    // Setup line position rects
    SDL_Rect* line_pos = malloc(sizeof(SDL_Rect)*num_lines);
    bool* line_prev_skip = malloc(sizeof(bool)*num_lines);
    for(int i = 0; i < num_lines; i++) {
        line_pos[i] = (SDL_Rect){-1, 0, 0, 0};
        line_prev_skip[i] = false;
    }
    SDL_Rect text_pos;
    y_start = y_start * (draw_scale_y * font_size.h);
    for(int i = 0; i < num_disp; i++) {
        uint64_t inst_pos = y + i;
        instruction_t * inst = get_instr_at_pos(inst_pos, trace);
        if (inst == NULL) {
            break;
        }
        if (inst->valid == false) {
            continue;
        }
        text_pos.x = 0;
        text_pos.y = i * draw_scale_y * font_size.h * num_trace + y_start;
        if (text_pos.y < 0) {
            continue;
        }
        
        if (draw_scale_y >= draw_instr_cutoff) {
            // Draw program counter
            char* pc_text = inst->pc_text;
            // Color program counter based on results of search
            gfx_color_t* colors = search_highlight(pc_text, color, SEARCHSEC_PC, NULL);
            gfx_draw_text_colors_scaled(instr_surf, pc_text, &text_pos, colors, 1, draw_scale_y, -1, -1);
            free(colors);
            
            // Draw trace instruction
            text_pos.x += 10;
            char* instr_text = inst->instruction;
            double trace_x_scale = ((double)(instr_surf_width - text_pos.x)) / ((double)((strlen(instr_text) + 1) * font_size.w));
            //if (trace_x_scale > 1) {
                trace_x_scale = 1;
            //}
            // Color trace instruction based on results of search
            colors = search_highlight(instr_text, color, SEARCHSEC_INSTR, NULL);
            gfx_draw_text_colors_scaled(instr_surf, instr_text, &text_pos, colors, trace_x_scale, draw_scale_y, -1, -1);
            free(colors);
        }
        
        // Draw cycle stages
        if (scale >= line_cutoff && inst->n_stages > 0) {
            if (inst->n_stages > 0) {
                int cur_cycle = inst->stages[0].cycle;
                stage_t * cur_stage = NULL;
                while(true) {
                    // Search for first stage with current cycle
                    bool all_less = true;
                    for(int j = 0; j < inst->n_stages; j++) {
                        if (inst->stages[j].cycle >= cur_cycle) {
                            all_less = false;
                        }
                        if (inst->stages[j].cycle == cur_cycle) {
                            cur_stage = &inst->stages[j];
                            break;
                        }
                    }
                    // Check if done
                    if (all_less) {
                        break;
                    }
                    // Setup color based on stage parameters
                    gfx_color_t stage_color = gfx_get_overall_stage_color(cur_stage, color);
                    // Get character string to put (either the stage symbol or -)
                    char c = '-';
                    SDL_Color char_color = stage_color.sdl_color;
                    if (cur_stage != NULL) {
                        c = cur_stage->identifier;
                    } else {
                        // Half-brightness if no stage
                        char_color.r = char_color.r / 2;
                        char_color.g = char_color.g / 2;
                        char_color.b = char_color.b / 2;
                    }
                    // Draw stage
                    double cycle_pos = cur_cycle - ((double)x_pos / trace_scale) + off;
                    text_pos.x = (cycle_pos * font_size.w * draw_scale_x);
                    gfx_draw_char_scaled(stage_surf, c, &text_pos, char_color, draw_scale_x, draw_scale_y, (int)(-draw_scale_x) << 8, screen_surface->w);
                    // Setup next loop
                    cur_cycle ++;
                    cur_stage = NULL;
                }
            }
            
            /*int s = 0;
            stage_t * cur_stage = &inst->stages[s];
            int cur_cycle = cur_stage->cycle;
            while(cur_stage != NULL) {
                // Setup color based on stage parameters
                gfx_color_t stage_color = gfx_get_overall_stage_color(cur_stage, color);
                //gfx_color_t stage_color = color;
                // Get character string to put (either the stage symbol or -)
                char c = '-';
                SDL_Color char_color = stage_color.sdl_color;
                if (cur_cycle >= cur_stage->cycle) {
                    c = cur_stage->identifier;
                    // Next coming stage
                    s++;
                    if (s >= inst->n_stages) cur_stage = NULL;
                    else cur_stage = &inst->stages[s];
                } else {
                    // Half-brightness if no stage
                    char_color.r = char_color.r / 2;
                    char_color.g = char_color.g / 2;
                    char_color.b = char_color.b / 2;
                }
                // Draw stage
                double cycle_pos = cur_cycle - ((double)x_pos / trace_scale) + off;
                text_pos.x = (cycle_pos * font_size.w * draw_scale_x);
                gfx_draw_char_scaled(stage_surf, c, &text_pos, char_color, draw_scale_x, draw_scale_y, (int)(-draw_scale_x) << 8, screen_surface->w);
                cur_cycle ++;
            }
            for(int j = 0; j < n_stages; j++) {
                double cycle_pos = cur_stage->cycle - ((double)x_pos / trace_scale) + off;
                text_pos.x = (cycle_pos * font_size.w * draw_scale_x);
                char c_ar[] = {cur_stage->identifier, '\0'};
                gfx_draw_text_scaled(stage_surf, c_ar, &text_pos, color, draw_scale_x, draw_scale_y, 0, screen_surface->w);
                cur_stage = cur_stage->next;
            }*/
        } else {
            // Line render
            // Check if moved far enough to draw another line
            if (text_pos.y > (line_pos[0].y + line_draw_precision - 1)) {
                // Loop through line definitions
                line_sec_t * cur_line = gfx_lines;
                for(int i = 0; i < num_lines; i++) {
                    // Loop through stages
                    // Check if any stage ids match line
                    bool matched_none = true;
                    for(int s = 0; s < inst->n_stages; s++) {
                        stage_t * cur_stage = &inst->stages[s];
                        if (cur_line->connect == cur_stage->identifier) {
                            // Draw line
                            SDL_Rect line_pos2 = {(cur_stage->cycle - ((double)x_pos / trace_scale) + (double)off) * font_size.w * draw_scale_x, text_pos.y, 0, 0};
                            if (line_pos[i].x != -1) {
                                if (line_prev_skip[i]) {
                                    SDL_SetRenderDrawColor(stage_render, color.sdl_color.r / 3, color.sdl_color.g / 3, color.sdl_color.b / 3, 0xFF);
                                } else {
                                    SDL_SetRenderDrawColor(stage_render, color.sdl_color.r, color.sdl_color.g, color.sdl_color.b, 0xFF);
                                }
                                SDL_RenderDrawLine(stage_render, line_pos[i].x, line_pos[i].y, line_pos2.x, line_pos2.y);
                                matched_none = false;
                            }
                            line_pos[i] = line_pos2;
                            break;
                        }
                    }
                    line_prev_skip[i] = matched_none;
                    cur_line = cur_line->next;
                }
                
                // Loop through stages
                // for(int s = 0; s < inst->n_stages; s++) {
                //     stage_t * cur_stage = &inst->stages[s];
                //     // Check if stage id matches any lines
                //     line_sec_t* cur_line = gfx_lines;
                //     bool matched_none = true;
                //     for(int i = 0; i < num_lines; i++) {
                //         if (cur_line->connect == cur_stage->identifier) {
                //             // Draw line
                //             SDL_Rect line_pos2 = {(cur_stage->cycle - ((double)x_pos / trace_scale) + (double)off) * font_size.w * draw_scale_x, text_pos.y, 0, 0};
                //             if (line_pos[i].x != -1) {
                //                 SDL_RenderDrawLine(stage_render, line_pos[i].x, line_pos[i].y, line_pos2.x, line_pos2.y);
                //             }
                //             line_pos[i] = line_pos2;
                //             matched_none = false;
                //             break;
                //         }
                //         if (matched_none) {
                //             line_pos[i].x = -1;
                //         }
                //         cur_line = cur_line->next;
                //     }
                // }
                
            }
        }
        
    }

    free(line_pos);
    free(line_prev_skip);
}

void gfx_draw_stage_box(gfx_color_t color, cycle_pos_t pos, SDL_Renderer* rend) {
    SDL_Rect rect;
    rect.x = ((pos.x * OPTIONS->scale[pos.trace]- x_pos) * scale  * font_size.w);
    rect.y = ((pos.y - y_pos) * scale * font_size.h);
    rect.w = scale * font_size.w * OPTIONS->scale[pos.trace];
    rect.h = scale * font_size.h;
    gfx_draw_box(color, rect, rend);
}




SDL_Rect gfx_get_mouse_world_position(int mx, int my) {
    int64_t x = (((double)(mx - instr_surf_width)) / scale / (double)font_size.w) + x_pos;
    int64_t y = (((double)my) / scale / (double)font_size.h) + y_pos;
    return (SDL_Rect){x, y, 0, 0};
}
cycle_pos_t gfx_get_mouse_stage_position(int mx, int my) {
    uint64_t y = (((double)my) / scale / (double)font_size.h) + y_pos;
    uint64_t trace = gfx_get_trace_num(y);
    uint64_t x = ((((double)(mx - instr_surf_width)) / scale / (double)font_size.w) + x_pos) / OPTIONS->scale[trace];
    uint64_t iy = gfx_get_instr_pos(y) * OPTIONS->num_traces + trace;
    return (cycle_pos_t){x, iy, trace};
}

uint64_t gfx_get_instr_pos(uint64_t y_pos) {
    return y_pos / OPTIONS->num_traces;
}
int gfx_get_trace_num(uint64_t y_pos) {
    return y_pos % OPTIONS->num_traces;
}

void gfx_move(int xv, int yv, bool fast_move) {
    if (fast_move && scale < 8) {
        xv = (8 * xv) / scale;
        yv = (8 * yv) / scale;
    }
    x_pos += xv;
    y_pos += yv;
    if (y_pos < 0) {
        y_pos = 0;
    }
    gfx_snap_if_forced();
}
void gfx_snap_if_forced() {
    if (force_snap) {
        gfx_snap();
    }
}
void gfx_snap() {
    // Set x-position of the camera to the x-position of the top trace
    // Loop through all traces and snap to the left-most one
    int64_t min_x = INT64_MAX;
    for(int i = 0; i < OPTIONS->num_traces; i++) {
        instruction_t* instr = get_instr_at_pos(gfx_get_instr_pos(y_pos), i);
        if (instr != NULL && instr->valid == true) {
            int64_t x = (instr->stages->cycle * OPTIONS->scale[i]) - 1;
            // Add trace offset to camera shift if this trace gets offset
            if (i != focus) {
                x += (trace_off * OPTIONS->scale[i]);
            }
            if (x < min_x) {
                min_x = x;
            }
        }
    }
    x_pos = min_x;
}
void gfx_toggle_force_snap() {
    force_snap = !force_snap;
}
void gfx_move_to_first() {
    y_pos = 0;
    gfx_snap();
}
void gfx_move_to_last() {
    y_pos = TRACES[0]->n_insts - 1;
    gfx_snap();
}

int gfx_get_first_line_stage_pos(instruction_t* instr) {
    if (instr->n_stages == 0)   return 0;
    uint64_t start_pos = instr->stages[0].cycle;
    for(int s = 0; s < instr->n_stages; s++) {
        stage_t * cur_stage = &instr->stages[s];
        char id = cur_stage->identifier;
        // Check if any of our line segments match this id
        line_sec_t* cur_line = gfx_lines;
        while(cur_line != NULL) {
            if (id == cur_line->connect) {
                // Found a match, return it's position in the instruction
                return cur_stage->cycle - start_pos;
            }
            cur_line = cur_line->next;
        }
    }
    return 0;
}

void gfx_snap_to_cycle(int mx, int my) {
    // Get mouse position
    SDL_Rect pos = gfx_get_mouse_world_position(mx, my);
    
    // Step 1: Move camera so main stage of first is on the mouse
    // Get first instrution after this position which is valid
    instruction_t* instr = get_valid_instr_at_pos(gfx_get_instr_pos(pos.y), 0);
    if (instr == NULL)  return;
    // Get current position of that instruction
    int64_t stage_x = instr->stages[0].cycle;
    if (scale < line_cutoff) {
        // Add position of first stage drawn as a line
        stage_x += gfx_get_first_line_stage_pos(instr);
    }
    // Scale position & shift camera
    int64_t x = round(((double)stage_x) * OPTIONS->scale[0]);
    x_pos -= pos.x - x;
    
    if (OPTIONS->num_traces <= 1) return;

    // Step 2: Set trace shift so second trace meets it
    // Get first instruction after this position which is valid
    pos = gfx_get_mouse_world_position(mx, my);
    instr = get_valid_instr_at_pos(gfx_get_instr_pos(pos.y), 1);
    if (instr == NULL) return;
    // Get current position of that instruction
    stage_x = instr->stages[0].cycle;
    if (scale < line_cutoff) {
        // Add position of first stage drawn as a line
        stage_x += gfx_get_first_line_stage_pos(instr);
    }
    // Scale position & shift trace
    double fx = ((double)stage_x) * OPTIONS->scale[1];
    double offset = (double)pos.x - fx;
    trace_off = offset / OPTIONS->scale[1];

    gfx_snap_if_forced();
}

void gfx_inc_scale(int mx, int my) {
    // Get mouse position
    SDL_Rect pos = gfx_get_mouse_world_position(mx, my);
    // Scale
    scale *= 0.75;
    // Correct position
    int64_t x = pos.x - round(((double)mx - (double)instr_surf_width) / scale / (double)font_size.w);
    int64_t y = pos.y - round(((double)my) / scale / (double)font_size.h);
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    x_pos = x;
    y_pos = y;
    gfx_snap_if_forced();
}
void gfx_dec_scale(int mx, int my) {
    // Get mouse position
    SDL_Rect pos = gfx_get_mouse_world_position(mx, my);
    // Scale
    scale *= 1.333333333333333333333333;
    // Correct position
    int64_t x = pos.x - round(((double)mx - (double)instr_surf_width) / scale / (double)font_size.w);
    int64_t y = pos.y - round(((double)my) / scale / (double)font_size.h);
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    x_pos = x;
    y_pos = y;
    gfx_snap_if_forced();
}

void gfx_shift_trace(int m, bool fast) {
    if (fast) {
        m = (4 * m) / scale;
    }
    trace_off += m;
    gfx_snap_if_forced();
}



/*
 * Functions used for drawing the other surfaces
 */


void make_instr_surf() {
    if (instr_surf != NULL) {
        SDL_FreeSurface(instr_surf);
    }
    instr_surf = SDL_CreateRGBSurface(0, instr_surf_width, gfx_win_height, 32, 0, 0, 0, 0);
}
void make_stage_surf() {
    if (stage_surf != NULL) {
        SDL_FreeSurface(stage_surf);
    }
    int w = gfx_win_width - instr_surf_width;
    if (w < 16) {
        w = 16;
    }
    stage_surf = SDL_CreateRGBSurface(0, w, gfx_win_height, 32, 0, 0, 0, 0);
}
void make_cmd_surf(int w, int h) {
    if (cmd_surf != NULL) {
        SDL_FreeSurface(cmd_surf);
    }
    cmd_surf = SDL_CreateRGBSurface(0, gfx_win_width, cmd_surf_height, 32, 0, 0, 0, 0);
    setup_cmd();
}

void gfx_win_refresh() {
    gfx_win_resize(gfx_win_width, gfx_win_height);
}

void gfx_win_resize(int w, int h) {
    SDL_DestroyRenderer(stage_render);
    screen_surface = SDL_GetWindowSurface(window);
    gfx_win_width = w;
    gfx_win_height = h;
    make_instr_surf();
    make_stage_surf();
    make_cmd_surf(w, h);
    stage_render = SDL_CreateSoftwareRenderer(stage_surf);
}

gfx_color_t gfx_get_overall_stage_color(stage_t* stage, gfx_color_t def) {
    if (stage == NULL) {
        return def;
    }
    // Go through the fields in this stage and check if any of them match the current search
    gfx_color_t color = def;
    color = search_highlight_overall(stage->id_str, color, SEARCHSEC_ID, NULL);
    color = search_highlight_overall(stage->name, color, SEARCHSEC_NAME, NULL);
    for(int i = 0; i < stage->n_params; i++) {
        parameter_t * param = &stage->params[i];
        color = search_highlight_overall(param->name, color, SEARCHSEC_PARAM_NAME, param->name);
        color = search_highlight_overall(param->value, color, SEARCHSEC_PARAM_VALUE, param->name);
    }
    return color;
}

void setup_cmd() {
    SDL_FillRect(cmd_surf, NULL, COLORS->bg.int_color);
    // Draw text to cmd surface
    SDL_Rect pos = {0, 8, 0, 0};
    const double cmd_scale = 1;
    if (input_mode == INMODE_CAM || input_mode == INMODE_HELP) {
        char text_buff[32];
        // Draw basic help text
        gfx_draw_text_scaled(cmd_surf, "Dual Pipetrace Viewer v0.1     ", &pos, COLORS->ui.sdl_color, cmd_scale, cmd_scale, -1, -1);
        gfx_draw_text_scaled(cmd_surf, "Press H to display help menu     ", &pos, COLORS->ui.sdl_color, cmd_scale, cmd_scale, -1, -1);
        snprintf(text_buff, 32, "%d", gfx_win_width);
        gfx_draw_text_scaled(cmd_surf, "Screen Size: ", &pos, COLORS->ui.sdl_color, cmd_scale, cmd_scale, -1, -1);
        gfx_draw_text_scaled(cmd_surf, text_buff, &pos, COLORS->ui.sdl_color, cmd_scale, cmd_scale, -1, -1);
        snprintf(text_buff, 32, "%d", gfx_win_height);
        gfx_draw_text_scaled(cmd_surf, "x", &pos, COLORS->ui.sdl_color, cmd_scale, cmd_scale, -1, -1);
        gfx_draw_text_scaled(cmd_surf, text_buff, &pos, COLORS->ui.sdl_color, cmd_scale, cmd_scale, -1, -1);
        pos.x = 1024;
        snprintf(text_buff, 32, "%d", instr_surf_width);
        gfx_draw_text_scaled(cmd_surf, "Instruction Window Width: ", &pos, COLORS->ui.sdl_color, cmd_scale, cmd_scale, -1, -1);
        gfx_draw_text_scaled(cmd_surf, text_buff, &pos, COLORS->ui.sdl_color, cmd_scale, cmd_scale, -1, -1);
    } else if (input_mode == INMODE_SEARCH) {
        // Draw contents of search / colol jump
        char* pre_search_text = "(search) /";
        if (SEARCH->is_colon) {
              pre_search_text = "(jumpto) :";
        }
        gfx_draw_text_scaled(cmd_surf, pre_search_text, &pos, COLORS->ui.sdl_color, cmd_scale, cmd_scale, -1, -1);
        gfx_draw_text_scaled(cmd_surf, SEARCH->input, &pos, COLORS->ui.sdl_color, cmd_scale, cmd_scale, -1, -1);
    }
    // Draw top border
    pos = (SDL_Rect){0, 0, cmd_surf->w, 4};
    SDL_FillRect(cmd_surf, &pos, COLORS->ui.int_color);
}
void setup_info(gfx_color_t color) {
    char text_buff[32];
    // Get checked stage
    int y = y_check / OPTIONS->num_traces;
    int trace = y_check % OPTIONS->num_traces;
    int x = x_check;
    if (trace != focus) {
        x -= trace_off;
    }
    stage_t* stage = gfx_get_stage(x, y, trace);
    if (stage == NULL) {
        info_on = false;
        // Free surface
        if (info_surf != NULL) {
            SDL_FreeSurface(info_surf);
            info_surf = NULL;
        }
        return;
    }
    // Get size it will take up
    int new_w = 512 - 32;
    int new_h = font_size.h * (5 + stage->n_params);
    // Check if different size than before
    if (!info_on || new_w != info_width || new_h != info_height) {
        // Recreate surface
        SDL_FreeSurface(info_surf);
        info_surf = SDL_CreateRGBSurface(0, new_w, new_h, 32, 0, 0, 0, 0);
        info_width = new_w;     info_height = new_h;
        info_on = true;
    }
    
    // Draw to surface
    int off_a = 0 * font_size.w;
    int off_b = 4 * font_size.w;
    int off_c = 20 * font_size.w;
    int off_d = 20 * font_size.w;
    // Fill with bgcolor
    SDL_FillRect(info_surf, NULL, COLORS->bg.int_color);
    // Draw address & instruction name
    SDL_Rect pos = {off_a, 0, 0, 0};
    instruction_t* instr = get_instr_at_pos(y, focus);
    gfx_draw_text_highlight_scaled(info_surf, instr->pc_text, &pos, color, 1, 1, -1, -1, SEARCHSEC_PC, NULL);
    pos.x = off_d;
    gfx_draw_text_highlight_scaled(info_surf, instr->instruction, &pos, color, 1, 1, -1, -1, SEARCHSEC_INSTR, NULL);
    // Draw cycle position
    pos.x = off_b;  pos.y += font_size.h;
    gfx_draw_text_scaled(info_surf, "cycle num:", &pos, color.sdl_color, 1, 1, -1, -1);
    pos.x = off_c;
    snprintf(text_buff, 32, "%ld", x_check);
    gfx_draw_text_scaled(info_surf, text_buff, &pos, color.sdl_color, 1, 1, -1, -1);
    // Draw instruction position
    pos.x = off_b; pos.y += font_size.h;
    gfx_draw_text_scaled(info_surf, "instr num:", &pos, color.sdl_color, 1, 1, -1, -1);
    pos.x = off_c;
    snprintf(text_buff, 32, "%ld", y_check);
    gfx_draw_text_scaled(info_surf, text_buff, &pos, color.sdl_color, 1, 1, -1, -1);
    // Draw identifier & name
    pos.x = off_b; pos.y += (font_size.h * 2);
    gfx_draw_text_highlight_scaled(info_surf, stage->id_str, &pos, color, 1, 1, -1, -1, SEARCHSEC_ID, NULL);
    pos.x = off_c;
    gfx_draw_text_highlight_scaled(info_surf, stage->name, &pos, color, 1, 1, -1, -1, SEARCHSEC_NAME, NULL);
    
    // Draw remaining parameters
    for(int i = 0; i < stage->n_params; i++) {
        parameter_t * param = &stage->params[i];
        pos.x = off_b; pos.y += font_size.h;
        gfx_draw_text_highlight_scaled(info_surf, param->name, &pos, color, 1, 1, -1, -1, SEARCHSEC_PARAM_NAME, param->name);
        pos.x = off_c;
        gfx_draw_text_highlight_scaled(info_surf, param->value, &pos, color, 1, 1, -1, -1, SEARCHSEC_PARAM_VALUE, param->name);
    }
}

void toggle_help() {
    if (help) {
        help = false;
        input_mode = INMODE_CAM;
    } else {
        help = true;
        input_mode = INMODE_HELP;
        help_page = 0;
        setup_help();
    }
}
void setup_help() {
    const int help_w = 256 + 128;
    const int help_h = 256 + 128;
    // Create help surface
    if (help_surf == NULL) {
        help_surf = SDL_CreateRGBSurface(0, help_w, help_h, 32, 0, 0, 0, 0);
    }
    // Draw border
    SDL_FillRect(help_surf, NULL, COLORS->ui.int_color);
    SDL_Rect pos = {4, 4, help_w-8, help_h-8};
    SDL_FillRect(help_surf, &pos, COLORS->bg.int_color);
    // Draw text
    const char** lines = help_text[help_page];
    pos = (SDL_Rect){8, 8, 0, 0};
    for(int i = 0; ; i++) {
        const char* str = lines[i];
        if (str[0] == '\0') {
            break;
        }
        gfx_draw_text_scaled(help_surf, str, &pos, COLORS->ui.sdl_color, 1, 1, -1, -1);
        pos.x = 8;    pos.y += font_size.h;
    }
    // Draw page number
    char text_buff[16];
    pos = (SDL_Rect){help_w - 128 + 4, help_h - 32 + 4, 0, 0};
    gfx_draw_text_scaled(help_surf, "Page: ", &pos, COLORS->ui.sdl_color, 1, 1, -1, -1);
    snprintf(text_buff, 16, "%d", help_page+1);
    gfx_draw_text_scaled(help_surf, text_buff, &pos, COLORS->ui.sdl_color, 1, 1, -1, -1);
    gfx_draw_text_scaled(help_surf, "/", &pos, COLORS->ui.sdl_color, 1, 1, -1, -1);
    snprintf(text_buff, 16, "%d", num_help_pages);
    gfx_draw_text_scaled(help_surf, text_buff, &pos, COLORS->ui.sdl_color, 1, 1, -1, -1);
}
void gfx_help_page_inc() {
    if (help) {
        help_page ++;
        if (help_page >= num_help_pages) {
            help_page = num_help_pages - 1;
        }
        setup_help();
    }
}
void gfx_help_page_dec() {
    if (help) {
        help_page --;
        if (help_page < 0) {
            help_page = 0;
        }
        setup_help();
    }
}


SDL_Rect gfx_get_win_text_size() {
    SDL_Rect size;
    size.w = (screen_surface->w / scale / font_size.w);
    size.h = screen_surface->h / scale / font_size.h;
    return size;
}


void gfx_look_at(int64_t y_look, int64_t x_look) {
    // Floating point opperations crash and burn if zoomed too far out
    if (scale < 0.01) {
        return;
    }
    if (y_look >= 0) {
        // Make sure provided y location is visible on-screen
        if (y_pos > y_look) {
            y_pos = y_look;
        } else {
            double font_h = scale * font_size.h;
            int num_visible = (double)(gfx_win_height - info_height) / font_h;
            if (y_look > (y_pos + num_visible - 1)) {
                y_pos = y_look - num_visible + 1;
                y_pos = y_look;
            }
        }
    }
    if (x_look >= 0) {
        // Make sure provided x location is visible on-screen
        x_look = (double)x_look * OPTIONS->scale[y_look % 2];
        if (y_look % 2 != focus) {
            x_look += trace_off;
        }
        if (x_pos > x_look) {
            x_pos = x_look;
        } else {
            double font_w = scale * font_size.w;
            int num_visible = (double)(gfx_win_width - instr_surf_width) / font_w;
            if (x_look > (x_pos + num_visible - 1)) {
                x_pos = x_look - (num_visible / 2) + 4;
            }
        }
    }
    gfx_snap_if_forced();
}

void gfx_jump_y(uint64_t y_look) {
    y_pos = y_look;
    gfx_snap();
}


void gfx_setup_int_color(SDL_Surface* surf, gfx_color_t* color) {
    color->int_color = SDL_MapRGB(surf->format, color->sdl_color.r, color->sdl_color.g, color->sdl_color.b);
}



void gfx_begin_dragging(int mx, int my) {
    is_dragging = true;
    drag_orig_mx = mx;
    drag_orig_my = my;
    drag_orig_x_pos = x_pos;
    drag_orig_y_pos = y_pos;
}

void gfx_drag(int mx, int my) {
    if (is_dragging) {
        // Move camera based on dragging
        int64_t to_x = (int64_t)drag_orig_x_pos - (int64_t)((double)(mx - drag_orig_mx) / (font_size.w * scale));
        int64_t to_y = (int64_t)drag_orig_y_pos - (int64_t)((double)(my - drag_orig_my) / (font_size.h * scale));
        if (to_x >= 0)  x_pos = to_x;
        if (to_y >= 0)  y_pos = to_y;
    }
}

void gfx_end_dragging() {
    is_dragging = false;
}



bool isEven(int num) {
    if (num == 1) return false;
    if (num == 2) return true;
    return(num-2);
}


