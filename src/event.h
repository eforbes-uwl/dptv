#ifndef GFX_EVENT_H
#define GFX_EVENT_H

#include <SDL2/SDL.h>
#include "dptv.h"
#include "options.h"
#include "gfx.h"


void run_events();

void frame_limit();

extern int mx;
extern int my;


#endif
