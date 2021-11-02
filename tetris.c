#include <ncurses.h>
#include <stdlib.h>

#include "tetris.h"

enum {
	shapes_count 	= 7,
	rotation_count 	= 4,
	blocks_count 	= 4,
};

enum {
	shape_i = 1,
    shape_l, shape_j,
    shape_s, shape_z,
    shape_t, shape_o
};

enum {
    left  = -1,
    right = 1,
};

enum {
	key_escape  = 27
};

typedef enum {
    crd_y = 0,
    crd_x = 1
} crdcnst;

typedef struct {
	int x, y;
} coord_type;

typedef struct {
	coord_type pos;
	int rotation;
	int shape;
} tetrimino_type;

typedef int scr_type[fh][fw];

#if 0
typedef struct {
    int coord[fh][fw];
    int shape[fh][fw];
} scr_type;
#endif

const char 	tetr_piece[] = "[]";
const int 	tetr_shapes[shapes_count][rotation_count][blocks_count][2] = {
	/* I */
	{{{1, 0}, {1, 1}, {1, 2}, {1, 3}},
	 {{0, 2}, {1, 2}, {2, 2}, {3, 2}},
	 {{2, 0}, {2, 1}, {2, 2}, {2, 3}},
	 {{0, 1}, {1, 1}, {2, 1}, {3, 1}}},

	/* L */
	{{{1, 0}, {1, 1}, {1, 2}, {0, 2}},
	 {{0, 1}, {1, 1}, {2, 1}, {2, 2}},
	 {{1, 0}, {1, 1}, {1, 2}, {2, 0}},
	 {{0, 0}, {0, 1}, {1, 1}, {2, 1}}},

	/* J */
	{{{0, 0}, {1, 0}, {1, 1}, {1, 2}},
	 {{0, 1}, {0, 2}, {1, 1}, {2, 1}},
	 {{1, 0}, {1, 1}, {1, 2}, {2, 2}},
	 {{0, 1}, {1, 1}, {2, 1}, {2, 0}}},

	/* S */
	{{{0, 1}, {0, 2}, {1, 0}, {1, 1}},
	 {{0, 1}, {1, 1}, {1, 2}, {2, 2}},
	 {{1, 1}, {1, 2}, {2, 0}, {2, 1}},
	 {{0, 0}, {1, 0}, {1, 1}, {2, 1}}},

	/* Z */
	{{{0, 0}, {0, 1}, {1, 1}, {1, 2}},
	 {{0, 2}, {1, 1}, {1, 2}, {2, 1}},
	 {{1, 0}, {1, 1}, {2, 1}, {2, 2}},
	 {{0, 1}, {1, 0}, {1, 1}, {2, 0}}},

	/* T */
	{{{0, 1}, {1, 0}, {1, 1}, {1, 2}},
	 {{0, 1}, {1, 1}, {1, 2}, {2, 1}},
	 {{1, 0}, {1, 1}, {1, 2}, {2, 1}},
	 {{0, 1}, {1, 0}, {1, 1}, {2, 1}}},

	/* O */
	{{{0, 1}, {0, 2}, {1, 1}, {1, 2}},
	 {{0, 1}, {0, 2}, {1, 1}, {1, 2}},
	 {{0, 1}, {0, 2}, {1, 1}, {1, 2}},
	 {{0, 1}, {0, 2}, {1, 1}, {1, 2}}}
};

const int delay_times[] = { 35, 30, 25, 20, 15, 10, 5, 2};

void tetr_colors_init()
{
	init_pair(shape_i, COLOR_WHITE, COLOR_CYAN);
	init_pair(shape_l, COLOR_WHITE, COLOR_YELLOW);
	init_pair(shape_j, COLOR_WHITE, COLOR_BLUE);
	init_pair(shape_s, COLOR_WHITE, COLOR_GREEN);
	init_pair(shape_z, COLOR_WHITE, COLOR_RED);
	init_pair(shape_t, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(shape_o, COLOR_BLACK, COLOR_WHITE);
}

void tetr_init(tetrimino_type *t, int shape)
{
	t->shape = shape; 
	t->rotation = 0; 
	t->pos.x = (scr_w/2)/2-1;
	t->pos.y = 1;
}

int get_tetr_block_crd(tetrimino_type t, int block, crdcnst crd)
{
    return tetr_shapes[t.shape][t.rotation][block][crd] + 
        (crd == crd_y ? t.pos.y : t.pos.x);
}

int get_tetr_scr_crd(tetrimino_type t, int block, crdcnst crd)
{
    return get_tetr_block_crd(t, block, crd)*(crd+1)+crd;
}

void put_block_at_field(tetrimino_type t, scr_type *f, int block)
{
    (*f)[get_tetr_block_crd(t, block, crd_y)-1]
        [get_tetr_block_crd(t, block, crd_x)] = t.shape+1;
}

void write_block(WINDOW *win, int y, int x, int shape)
{
    wattrset(win, COLOR_PAIR(shape));
    mvwaddstr(win, y, x, tetr_piece);
    wattroff(win, COLOR_PAIR(shape));
}

void erase_block(WINDOW *win, int y, int x)
{
    mvwaddstr(win, y, x, "  ");
}

void write_tetr(WINDOW *win, tetrimino_type t)
{
	int i;
    for(i = 0; i < blocks_count; i++) {
        write_block(win, get_tetr_scr_crd(t, i, crd_y),
                         get_tetr_scr_crd(t, i, crd_x), t.shape+1);
    }
}

void erase_tetr(WINDOW *win, tetrimino_type t)
{
	int i;
	for(i = 0; i < blocks_count; i++) {
		erase_block(win, get_tetr_scr_crd(t, i, crd_y),
                         get_tetr_scr_crd(t, i, crd_x));
	}
}

void save_at_field(tetrimino_type t, scr_type *f)
{
#ifdef DEBUG
    FILE *file;
    int j;
#endif
    int i;
    for(i = 0; i < blocks_count; i++) {
        put_block_at_field(t, f, i);
    }
#ifdef DEBUG
    file = fopen("debug.txt", "w");
    for(i = 0; i < fh; i++) {
        for(j = 0; j < fw; j++)
            fprintf(file, "%d", (*f)[i][j]);
        fprintf(file, "\n");
    }
    fclose(file);
#endif
}

int is_tetr_fit(const tetrimino_type t, scr_type f)
{
    int i, ty, tx;
    for(i = 0; i < blocks_count; i++) {
        ty = get_tetr_block_crd(t, i, crd_y);
        tx = get_tetr_block_crd(t, i, crd_x);
#ifdef DEBUG
        mvprintw(4+i, 0, "itf ty=%02d tx=%02d", ty, tx);
#endif
        if((f[ty][tx] != 0) ||
            (ty < 0)  || (tx < 0) ||
            (ty > fh) || (tx >= fw))
            return 0;
    }
    return 1;
}

int try_to_fit(tetrimino_type *t, scr_type f)
{
    t->pos.x--;
    if(is_tetr_fit(*t, f)) {
        return 1;
    }

    t->pos.x += 2;
    if(is_tetr_fit(*t, f)) {
        return 1;
    }

    t->pos.x--;
    return 0;
}

int is_tetr_on_something(const tetrimino_type t, scr_type f)
{
    int i, ty, tx;
    for(i = 0; i < blocks_count; i++) {
        ty = get_tetr_block_crd(t, i, crd_y);
        tx = get_tetr_block_crd(t, i, crd_x);
#ifdef DEBUG
        mvprintw(8+i, 0, "itos ty=%02d tx=%02d", ty, tx);
#endif
        if((ty > fh) || (f[ty-1][tx] != 0))
            return 1;
    }
    return 0;
}

int tetr_fall(tetrimino_type *t, scr_type f)
{
	t->pos.y++;
    if(is_tetr_on_something(*t, f)) {
	    t->pos.y--;
        return 1;
    }
    return 0;
}

void tetr_rotate(tetrimino_type *t, scr_type f)
{
	t->rotation++;

	if(t->rotation == 4)
		t->rotation = 0;

    if(!is_tetr_fit(*t, f)) {
        if(try_to_fit(t, f) != 1) {
            if(t->rotation == 0)
                t->rotation = 3;
            else
                t->rotation--;
        }
    }
}

void tetr_move(tetrimino_type *t, scr_type f, int dir)
{
    t->pos.x += dir;
    if(!is_tetr_fit(*t, f)) {
        t->pos.x -= dir;
    }
}

int is_line_full(int line[fw])
{
    int i, n = 0;
    for(i = 0; i < fw; i++) {
        n += line[i] != 0;
    }
    return n == fw;
}

void scr_rewrite(WINDOW *win, scr_type f)
{
    int i, j;
    for(i = 0; i < fh; i++) {
        for(j = 0; j < fw; j++) {
            if(f[i][j] != 0)
                write_block(win, i+1, j*2+1, f[i][j]);
            else
                erase_block(win, i+1, j*2+1);
        }
    }
}

void scr_remove_line(scr_type *f, int n)
{
    int i;
    for(i = 0; i < fw; i++)
        (*f)[n][i] = 0;
}

void scr_shift(scr_type *f, int n)
{
    int i, j;

    for(i = n; i > 1; i--)
        for(j = 0; j < fw; j++)
            (*f)[i][j] = (*f)[i-1][j];
}

int check_scr_lines(scr_type *f)
{
    int i, n = 0;

    for(i = 0; i < fh; i++) {
        if(is_line_full((*f)[i])) {
            n++;
            scr_remove_line(f, i);
            scr_shift(f, i);
        }
    }
    return n;
}

void init_game_field(scr_type *f)
{
    int i, j;
    for(i = 0; i < fh; i++)
        for(j = 0; j < fw; j++) 
            (*f)[i][j] = 0;
}

void init_tetris_win(WINDOW **win, int sm_h, int sm_w)
{
    *win = newwin(border_h, border_w,
            (sm_h - border_h)/2,
            (sm_w - border_w)/2);
    box(*win, 0, 0);
    wrefresh(*win);
}  

void fall_delay(int *n)
{
    napms(20);
    (*n)++;
}

int is_game_lost(scr_type f)
{
    int i;
    for(i = 0; i < fw; i++) {
         if(f[0][i] != 0 || f[1][i] != 0)
             return 1;
    }
    return 0;
}    

void tetris_game()
{
	WINDOW *win;
	tetrimino_type curr_tetr, next_tetr;
    scr_type field;
	int key, sm_h, sm_w, dcount,
        dflag, flines, level, score_mod;
    long score;

    getmaxyx(stdscr, sm_h, sm_w);
    init_tetris_win(&win, sm_h, sm_w);

    tetr_colors_init();
    timeout(0);

    init_game_field(&field);
    tetr_init(&curr_tetr, rand()%shapes_count);
    tetr_init(&next_tetr, rand()%shapes_count);
    write_tetr(win, curr_tetr);
    wrefresh(win);

    dcount = 0;
    dflag = 0;
    score = 0;
    score_mod = 1;
    level = 0;
    while((key = getch()) != key_escape) {
        erase_tetr(win, curr_tetr);
        switch(key) {
            case 'r':
                tetr_rotate(&curr_tetr, field);
                break;
            case KEY_LEFT:
                tetr_move(&curr_tetr, field, left);
                break;
            case KEY_RIGHT:
                tetr_move(&curr_tetr, field, right);
                break;
            case KEY_DOWN:
                dflag = dflag == 0 ? 1 : 0;
                break;
        }
        fall_delay(&dcount);
        if(dcount == delay_times[level] || dflag) {
            if(tetr_fall(&curr_tetr, field)) {
                save_at_field(curr_tetr, &field);
                write_tetr(win, curr_tetr);
                curr_tetr = next_tetr;
                tetr_init(&next_tetr, rand()%shapes_count);
                flines = check_scr_lines(&field);
                score += flines == 4 ?
                    flines*2000*score_mod :
                    flines*500*score_mod;
                if(flines > 0) {
                    scr_rewrite(win, field);
                };
                dflag = 0;
            }
            score += dflag == 1 ? 7*score_mod : 0;
            dcount = 0;
        }
#ifdef DEBUG
        mvprintw(0, 0, "y=%d x=%d", curr_tetr.pos.y, curr_tetr.pos.x);
        mvprintw(1, 0, "rot=%d", curr_tetr.rotation);
        mvprintw(2, 0, "shape=%d", curr_tetr.shape);
        mvprintw(3, 0, "delay count=%02d", dcount);
#endif
        mvprintw(0, (sm_w-16)/2, "score=%010d", score);
        mvprintw(1, (sm_w-6)/2, "level=%d", level);
        write_tetr(win, curr_tetr);
        refresh();
        wrefresh(win);
        if(is_game_lost(field)) {
            curr_tetr = next_tetr;
            write_tetr(win, curr_tetr);
            refresh();
            wrefresh(win);
            napms(500);
            quit(0);
        }
        if(score >= 1000*(10*level*level+1)) {
            score_mod += level*level;
            level++;
        }
    }
    write_tetr(win, curr_tetr);
    wrefresh(win);
}

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

