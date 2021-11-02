#ifndef TETRIS_H_SENTRY
#define TETRIS_H_SENTRY

enum {
	scr_h 	    = 20,
	scr_w 	    = 20,
    fh          = scr_h,
    fw          = scr_w/2,
	border_h 	= scr_h+2,
	border_w 	= scr_h+2,
    scr_min_h   = scr_h+6,
    scr_min_w   = scr_w+12
};

void scr_prep();
void scr_rest();
void tetris_game();
void quit(int);

#endif
