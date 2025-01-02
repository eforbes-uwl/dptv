#include <SDL2/SDL.h>
#include "dptv.h"
#include "options.h"
#include "gfx.h"
#include "search.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>


static SDL_Event e;
bool fast_move = false;
int mx = 0;
int my = 0;

clock_t start, stop;
bool first_frame = true;


void run_events() {
    // Itterate through all occuring events
    while(SDL_PollEvent(&e) != 0) {
        SDL_Scancode code = e.key.keysym.scancode;
        SDL_Keycode key = e.key.keysym.sym;
        switch(e.type) {
            // User requests quit
            case(SDL_QUIT):
                quit = true;
                break;
            // User does something with the mouse
            case(SDL_MOUSEBUTTONDOWN):
                if (e.button.button == SDL_BUTTON_LEFT) {
                    gfx_begin_dragging(mx, my);
                }
                if (e.button.button == SDL_BUTTON_RIGHT) {
                    gfx_snap_to_cycle(mx, my);
                }
                if (e.button.button == SDL_BUTTON_MIDDLE) {
                }
                break;
            case(SDL_MOUSEMOTION):
                mx = e.motion.x;
                my = e.motion.y;
                gfx_drag(mx, my);
                break;
            case(SDL_MOUSEBUTTONUP):
                if (e.button.button == SDL_BUTTON_LEFT) {
                    gfx_end_dragging();
                }
            case(SDL_WINDOWEVENT):
                // User resizes window
                if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    gfx_win_resize(e.window.data1, e.window.data2);
                }
                break;
            // User presses keyboard button
            case(SDL_KEYDOWN):
                // Quit
                if (input_mode != INMODE_SEARCH) {
                    if (code == SDL_SCANCODE_Q) {
                        quit = true;
                    }
                }
                if (input_mode == INMODE_CAM) {
                    // Switch to other modes
                    if (code == SDL_SCANCODE_H) {
                        toggle_help();
                    } else if (code == SDL_SCANCODE_SLASH) {
                        search_input_begin(false);
                        //SDL_StartTextInput();
                    } else if (code == SDL_SCANCODE_SEMICOLON) {
                        search_input_begin(true);
                    } else if (code == SDL_SCANCODE_G) {
                        if (fast_move) {
                            gfx_move_to_last();
                        } else {
                            gfx_move_to_first();
                        }
                    } else
                    // Other actions with keycodes
                    if (code == SDL_SCANCODE_N) {
                        int64_t x = -1;
                        int64_t y = search_find(fast_move ? false : true, &x);
                        gfx_look_at(y, x);
                    } else if (code == SDL_SCANCODE_P) {
                        gfx_snap();
                        if (fast_move) {
                            gfx_toggle_force_snap();
                        }
                    } else if (code == SDL_SCANCODE_R) {
                        gfx_reset();
                    } else if (code == SDL_SCANCODE_L) {
                        instr_surf_width += 1;
                        gfx_win_refresh();
                    } else if (code == SDL_SCANCODE_K) {
                        instr_surf_width -= 1;
                        gfx_win_refresh();
                    } else
                    // Move camera (scancode/keycode)
                    if (code == SDL_SCANCODE_LSHIFT || code == SDL_SCANCODE_RSHIFT) {
                        fast_move = true;
                    } else if (code == SDL_SCANCODE_W || key == SDLK_UP) {
                        gfx_move(0, -1, fast_move);
                    } else if (code == SDL_SCANCODE_S || key == SDLK_DOWN) {
                        gfx_move(0, 1, fast_move);
                    } else if (code == SDL_SCANCODE_D || key == SDLK_RIGHT) {
                        gfx_move(1, 0, fast_move);
                    } else if (code == SDL_SCANCODE_A || key == SDLK_LEFT) {
                        gfx_move(-1, 0, fast_move);
                    }
                    // Shift second trace cycle offset
                    if (code == SDL_SCANCODE_Z) {
                        gfx_shift_trace(-1, fast_move);
                    } else if (code == SDL_SCANCODE_X) {
                        gfx_shift_trace(1, fast_move);
                    }
                    // Cancel search
                    if (key == SDLK_ESCAPE) {
                        search_end();
                    }
                } else if (input_mode == INMODE_HELP) {
                    // Help menu navigation
                    if (code == SDL_SCANCODE_D || key == SDLK_RIGHT) {
                        gfx_help_page_inc();
                    } else if (code == SDL_SCANCODE_A || key == SDLK_LEFT) {
                        gfx_help_page_dec();
                    } else if (code == SDL_SCANCODE_H || key == SDLK_ESCAPE) {
                        toggle_help();
                    }
                } else if (input_mode == INMODE_SEARCH) {
                    // Searching
                    if (key == SDLK_ESCAPE) {
                        search_input_abort();
                        //SDL_StopTextInput();
                    } else if (key == SDLK_KP_ENTER || key == SDLK_RETURN || key == SDLK_RETURN2) {
                        search_input_finish();
                        //SDL_StopTextInput();
                    } else if (key == SDLK_BACKSPACE) {
                        search_input_key(0);
                    }
                }
                break;
            case(SDL_TEXTINPUT):
                // Text input, used in searching
                if (input_mode == INMODE_SEARCH) {
                    search_input_key(e.text.text[0]);
                }
                break;
            case(SDL_KEYUP):
                if (code == SDL_SCANCODE_P) {
                    //gfx_snap_off();
                } else if (code == SDL_SCANCODE_LSHIFT || code == SDL_SCANCODE_RSHIFT) {
                    fast_move = false;
                }
                break;
            case(SDL_MOUSEWHEEL):
                if (input_mode == INMODE_CAM) {
                    if (e.wheel.y < 0) {
                        gfx_inc_scale(mx, my);
                    } else if (e.wheel.y > 0) {
                        gfx_dec_scale(mx, my);
                    }
                }
                break;
            default:
                break;
        }
    }

}

void frame_limit() {
    
    // Get how long to sleep
    if (!first_frame) {
        stop = clock();
        double exec_time = (stop - start);
        double sleep_time = (1/120 - exec_time);
        if (sleep_time > 0) {
            sleep(sleep_time);
        }
    }
    start = clock();
    
}
