#include <ncurses.h>
#include <stdlib.h>

#include "tetris.h"
#include "etc.h"

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
} tetromino_type;

typedef int scr_type[fld_h][fld_w];

#if 0
typedef struct {
    int coord[fld_h][fld_w];
    int shape[fld_h][fld_w];
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

const int delay_times[] = { 70, 60, 50, 40, 30, 20, 10, 5};

static void tetr_colors_init()
{
	init_pair(shape_i, COLOR_WHITE, COLOR_CYAN);
	init_pair(shape_l, COLOR_WHITE, COLOR_YELLOW);
	init_pair(shape_j, COLOR_WHITE, COLOR_BLUE);
	init_pair(shape_s, COLOR_WHITE, COLOR_GREEN);
	init_pair(shape_z, COLOR_WHITE, COLOR_RED);
	init_pair(shape_t, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(shape_o, COLOR_BLACK, COLOR_WHITE);
}

static void tetr_init(tetromino_type *t, int shape)
{
	t->shape = shape; 
	t->rotation = 0; 
	t->pos.x = (scr_w/2)/2-1;
	t->pos.y = 1;
}

static int get_tetr_block_crd(tetromino_type t, int block, crdcnst crd)
{
    return tetr_shapes[t.shape][t.rotation][block][crd] + 
        (crd == crd_y ? t.pos.y : t.pos.x);
}

static int get_tetr_scr_crd(tetromino_type t, int block, crdcnst crd)
{
    return get_tetr_block_crd(t, block, crd)*(crd+1)+crd;
}

static void put_block_at_field(tetromino_type t, scr_type *f, int block)
{
    (*f)[get_tetr_block_crd(t, block, crd_y)-1]
        [get_tetr_block_crd(t, block, crd_x)] = t.shape+1;
}

static void write_block(WINDOW *win, int y, int x, int shape)
{
    wattrset(win, COLOR_PAIR(shape));
    mvwaddstr(win, y, x, tetr_piece);
    wattroff(win, COLOR_PAIR(shape));
}

static void erase_block(WINDOW *win, int y, int x)
{
    mvwaddstr(win, y, x, "  ");
}

static void write_tetr(WINDOW *win, tetromino_type t)
{
	int i;
    for(i = 0; i < blocks_count; i++) {
        write_block(win, get_tetr_scr_crd(t, i, crd_y),
                         get_tetr_scr_crd(t, i, crd_x), t.shape+1);
    }
}

static void erase_tetr(WINDOW *win, tetromino_type t)
{
	int i;
	for(i = 0; i < blocks_count; i++) {
		erase_block(win, get_tetr_scr_crd(t, i, crd_y),
                         get_tetr_scr_crd(t, i, crd_x));
	}
}

static void save_at_field(tetromino_type t, scr_type *f)
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
    for(i = 0; i < fld_h; i++) {
        for(j = 0; j < fld_w; j++)
            fprintf(file, "%d", (*f)[i][j]);
        fprintf(file, "\n");
    }
    fclose(file);
#endif
}

static int is_tetr_fit(const tetromino_type t, scr_type f)
{
    int i, ty, tx;
    for(i = 0; i < blocks_count; i++) {
        ty = get_tetr_block_crd(t, i, crd_y)-1;
        tx = get_tetr_block_crd(t, i, crd_x);
#ifdef DEBUG
        mvprintw(4+i, 0, "itf ty=%02d tx=%02d", ty, tx);
#endif
        if((f[ty][tx] != 0) ||
            (ty < 0)  || (tx < 0) ||
            (ty > fld_h) || (tx >= fld_w))
            return 0;
    }
    return 1;
}

static int try_to_fit(tetromino_type *t, scr_type f)
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

static int is_tetr_on_something(const tetromino_type t, scr_type f)
{
    int i, ty, tx;
    for(i = 0; i < blocks_count; i++) {
        ty = get_tetr_block_crd(t, i, crd_y)-1;
        tx = get_tetr_block_crd(t, i, crd_x);
#ifdef DEBUG
        mvprintw(8+i, 0, "itos ty=%02d tx=%02d", ty, tx);
#endif
        if((ty >= fld_h) || (f[ty][tx] != 0))
            return 1;
    }
    return 0;
}

static int tetr_fall(tetromino_type *t, scr_type f)
{
	t->pos.y++;
    if(is_tetr_on_something(*t, f)) {
	    t->pos.y--;
        return 1;
    }
    return 0;
}

static void tetr_rotate(tetromino_type *t, scr_type f)
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

static void tetr_move(tetromino_type *t, scr_type f, int dir)
{
    t->pos.x += dir;
    if(!is_tetr_fit(*t, f)) {
        t->pos.x -= dir;
    }
}

static int is_line_full(int line[fld_w])
{
    int i, n = 0;
    for(i = 0; i < fld_w; i++) {
        n += line[i] != 0;
    }
    return n == fld_w;
}

static void scr_rewrite(WINDOW *win, scr_type f)
{
    int i, j;
    for(i = 0; i < fld_h; i++) {
        for(j = 0; j < fld_w; j++) {
            if(f[i][j] != 0)
                write_block(win, i+1, j*2+1, f[i][j]);
            else
                erase_block(win, i+1, j*2+1);
        }
    }
}

static void scr_remove_line(scr_type *f, int n)
{
    int i;
    for(i = 0; i < fld_w; i++)
        (*f)[n][i] = 0;
}

static void scr_shift(scr_type *f, int n)
{
    int i, j;

    for(i = n; i > 1; i--)
        for(j = 0; j < fld_w; j++)
            (*f)[i][j] = (*f)[i-1][j];
}

static int check_scr_lines(scr_type *f)
{
    int i, n = 0;

    for(i = 0; i < fld_h; i++) {
        if(is_line_full((*f)[i])) {
            n++;
            scr_remove_line(f, i);
            scr_shift(f, i);
        }
    }
    return n;
}

static void init_game_field(scr_type *f)
{
    int i, j;
    for(i = 0; i < fld_h; i++)
        for(j = 0; j < fld_w; j++) 
            (*f)[i][j] = 0;
}

static void
init_tetris_wins(WINDOW **mwin, WINDOW **ntwin, WINDOW **swin,
		int sm_h, int sm_w)
{
    *mwin = newwin(border_h, border_w,
            (sm_h - border_h)/2,
            (sm_w - border_w)/2);
    box(*mwin, 0, 0);
    wrefresh(*mwin);
    *ntwin = newwin(nt_win_h, nt_win_w,
            (sm_h - border_h)/2,
            (sm_w + border_w)/2);
	box(*ntwin, 0, 0);
	wrefresh(*ntwin);
    *swin = newwin(sc_win_h, sc_win_w,
            (sm_h - nt_win_h - 4)/2,
            (sm_w + border_w)/2);
	box(*swin, 0, 0);
	wrefresh(*swin);
}  

static void fall_delay(int *n)
{
    napms(10);
    (*n)++;
}

static int is_game_lost(scr_type f)
{
    int i;
    for(i = 0; i < fld_w; i++) {
         if(f[0][i] != 0 || f[1][i] != 0)
             return 1;
    }
    return 0;
}    

static void new_tetr(tetromino_type *ct, tetromino_type *nt)
{
	*ct = *nt;
	tetr_init(nt, rand()%shapes_count);
}

static int score_for_flines(scr_type *f, long *score, int smod)
{
	int flines;
	flines = check_scr_lines(f);
	*score += flines == 4 ?
		flines*2000*smod :
		flines*500*smod;
	if(flines > 0)
		return 1;
	else
		return 0;
}

static void score_for_falling(long *score, int df, int smod, int *dcount)
{
	*score += (df == 1 ? 7*smod : 0);
	*dcount = 0;
}

static void level_inc(long score, int *smod, int *lvl)
{
	int n = (*lvl)*(*lvl);
	if(score >= 1000*(10*n+1)) {
		*smod += n;
		(*lvl)++;
	} 
}

static void write_next_tetr(WINDOW *ntwin, tetromino_type t)
{
	tetromino_type temp;
	temp = t;
	temp.pos.x -= 2;
	temp.pos.y += 1;
	write_tetr(ntwin, temp);
}

static void erase_next_tetr(WINDOW *ntwin, tetromino_type t)
{
	tetromino_type temp;
	temp = t;
	temp.pos.x -= 2;
	temp.pos.y += 1;
	erase_tetr(ntwin, temp);
}

static void write_score(WINDOW *swin, long score, int level, int sm_h, int sm_w)
{
    wattrset(swin, A_BOLD);
    mvwprintw(swin, 1, (sc_win_w-6)/2, "SCORE:");
    mvwprintw(swin, 2, 2, "%012d", score);

    mvwprintw(swin, 4, (sc_win_w-6)/2, "LEVEL:");
    mvwprintw(swin, 5, (sc_win_w-2)/2, "%02d", level);

    wattroff(swin, A_BOLD);
}

void tetris_game()
{
	WINDOW *win, *ntwin, *swin;
	tetromino_type curr_tetr, next_tetr;
    scr_type field;
	int key, sm_h, sm_w, dcount,
        dflag, level, score_mod;
    long score;

	/* initializing variables */
    getmaxyx(stdscr, sm_h, sm_w);
    init_tetris_wins(&win, &ntwin, &swin, sm_h, sm_w);

    tetr_colors_init();
    timeout(0);

    init_game_field(&field);
    tetr_init(&curr_tetr, rand()%shapes_count);
    tetr_init(&next_tetr, rand()%shapes_count);
    write_tetr(win, curr_tetr);
    wrefresh(win);
	write_next_tetr(ntwin, next_tetr);
    wrefresh(ntwin);

    dcount = 0;
    dflag = 0;
    score = 0;
    score_mod = 1;
    level = 0;


	/* main game loop */
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
				/* delay flag 
				 * if set then tetromino
				 * falls without delay.
				 */
                dflag = dflag == 0 ? 1 : 0;
                break;
			default:
				break;
        }
                   
		/* tetromino falling */
        fall_delay(&dcount);
        if(dcount == delay_times[level] || dflag) {
            if(tetr_fall(&curr_tetr, field)) {
                save_at_field(curr_tetr, &field);
                write_tetr(win, curr_tetr);

        		erase_next_tetr(ntwin, next_tetr);
                
				new_tetr(&curr_tetr, &next_tetr);

                write_next_tetr(ntwin, next_tetr);
    			wrefresh(ntwin);

				/* score system */
				if(score_for_flines(&field, &score, score_mod) == 1)
					scr_rewrite(win, field);
                dflag = 0;
            }
			score_for_falling(&score, dflag, score_mod, &dcount);
        }

#ifdef DEBUG
        mvprintw(0, 0, "y=%d x=%d", curr_tetr.pos.y, curr_tetr.pos.x);
        mvprintw(1, 0, "rot=%d", curr_tetr.rotation);
        mvprintw(2, 0, "shape=%d", curr_tetr.shape);
        mvprintw(3, 0, "delay count=%02d", dcount);
#endif

        write_score(swin, score, level, sm_h, sm_w);
        wrefresh(swin);

        write_tetr(win, curr_tetr);
        refresh();
        wrefresh(win);

        if(is_game_lost(field)) {
			new_tetr(&curr_tetr, &next_tetr);

			write_tetr(win, curr_tetr);
            refresh();
            wrefresh(win);

            napms(500);
            quit(0);
        }

		level_inc(score, &score_mod, &level);
    }
}


