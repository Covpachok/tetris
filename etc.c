#include <ncurses.h>
#include <stdlib.h>

void scr_prep()
{
	initscr();
	cbreak();
	start_color();
	keypad(stdscr, 1);
	noecho();
	curs_set(0);
    refresh();
}

void scr_rest()
{
	curs_set(1);
	echo();
	nocbreak();
	endwin();
}

void quit(int errc)
{
    scr_rest();
    exit(errc);
}
