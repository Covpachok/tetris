#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

#include "tetris.h"

int main()
{
    int sm_h, sm_w;

    srand(time(NULL));
	scr_prep();

    getmaxyx(stdscr, sm_h, sm_w);
    if(sm_h < scr_min_h || sm_w < scr_min_w) {
        scr_rest();
        fprintf(stderr, "ERROR: too small screen size\n");
        return 1;
    }

    tetris_game();

    quit(0);
}
