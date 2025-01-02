#ifndef _GFX_SYSTEM_H_
#define _GFX_SYSTEM_H_

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "dptv.h"


// Color definition struct, some SDL routines expect a uint64_t for the color while some want an SDL_Color struct
struct gfx_color {
    uint64_t int_color;
    SDL_Color sdl_color;
} typedef gfx_color_t;

// Line section struct, linked list of which stages to connect with lines
struct line_section {
    char connect;
    uint64_t color;
    struct line_section* next;
} typedef line_sec_t;

// Position struct, includes cycle number, instuction number, and trace number
struct cycle_pos {
    uint64_t x;
    uint64_t y;
    int trace;
} typedef cycle_pos_t;


void init_gfx();
void gfx_reset();
void gfx_win_refresh();
void gfx_win_resize(int w, int h);
void gfx_update();
void gfx_draw_text_scaled(SDL_Surface* surf, const char* text, SDL_Rect* text_pos, SDL_Color color, double x_scale, double y_scale, int l_clip, int r_clip);
void gfx_draw_char_scaled(SDL_Surface* surf, char c, SDL_Rect* text_pos, SDL_Color color, double x_scale, double y_scale, int l_clip, int r_clip);
void gfx_gen_char_surfs();
void gfx_draw_text_colors_scaled(SDL_Surface* surf, const char* text, SDL_Rect* text_pos, gfx_color_t* colors, double x_scale, double y_scale, int l_clip, int r_clip);
void gfx_draw_text_highlight_scaled(SDL_Surface* surf, const char* text, SDL_Rect* text_pos, gfx_color_t def, double x_scale, double y_scale, int l_clip, int r_clip, int sec, char* param_name);
SDL_Rect gfx_get_font_size();
void gfx_draw_trace_pos(uint64_t y, gfx_color_t color, double scale, int num_disp, int trace, double trace_scale, int num_trace, int y_start, int off);
void gfx_draw_box(gfx_color_t color, SDL_Rect pos, SDL_Renderer* rend);
void gfx_draw_stage_box(gfx_color_t color, cycle_pos_t pos, SDL_Renderer* rend);
gfx_color_t gfx_get_overall_stage_color(stage_t* stage, gfx_color_t def);
void setup_info();
void setup_help();
void setup_cmd();
void toggle_help();
void gfx_help_page_inc();
void gfx_help_page_dec();
instruction_t* get_instr_at_pos(uint64_t pos, int trace);
int gfx_get_trace_num(uint64_t y_pos);
uint64_t gfx_get_instr_pos(uint64_t y_pos);

cycle_pos_t gfx_get_mouse_stage_position(int mx, int my);
SDL_Rect gfx_get_mouse_world_position(int mx, int my);
stage_t* gfx_get_stage(uint64_t x, uint64_t y, int trace);

void gfx_move(int x, int y, bool fast_move);
void gfx_snap();
void gfx_snap_if_forced();
void gfx_toggle_force_snap();
void gfx_move_to_first();
void gfx_move_to_last();
void gfx_snap_to_cycle(int mx, int my);
void gfx_inc_scale(int mx, int my);
void gfx_dec_scale(int mx, int my);
void gfx_shift_trace(int m, bool fast_move);
void gfx_look_at(int64_t y_pos, int64_t x_pos);
void gfx_jump_y(uint64_t y_pos);
void gfx_begin_dragging(int mx, int my);
void gfx_drag(int mx, int my);
void gfx_end_dragging();

void gfx_setup_int_color(SDL_Surface* surf, gfx_color_t* color);

extern int64_t x_pos;
extern int64_t y_pos;
extern int focus;
extern int input_mode;
extern int instr_surf_width;
#define INMODE_CAM     0
#define INMODE_HELP    1
#define INMODE_SEARCH  2



#endif
