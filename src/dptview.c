#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include "dptv.h"
#include "options.h"
#include "trace_handler.h"
#include "gfx.h"
#include "event.h"
#include "search.h"
#include <stdbool.h>

bool quit;

int main(int argc, char *argv[]) {
    quit = false;
    if (process_cmd_line(argc, argv) != CMD_OPTIONS_OK){
        dump_cmd_opts();
        return EXIT_FAILURE;
    }

    init_traces();
    init_search();
    init_gfx();
    
    while(!quit) {
        run_events();
        gfx_update();
        frame_limit();
    }

    return EXIT_SUCCESS;
}
