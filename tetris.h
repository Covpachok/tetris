#ifndef TETRIS_H_SENTRY
#define TETRIS_H_SENTRY

enum {
	scr_h 	    = 20,
	scr_w 	    = 20,
    fld_h       = scr_h,
    fld_w       = scr_w/2,
	border_h 	= scr_h+2,
	border_w 	= scr_h+2,
	nt_win_h 	= 6,
	nt_win_w 	= 8,
    scr_min_h   = scr_h+6,
    scr_min_w   = scr_w+12
};

void tetris_game();

#endif
