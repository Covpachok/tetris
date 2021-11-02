#ifndef ETC_H_SENTRY
#define ETC_H_SENTRY

enum {
	key_escape  = 27,
	key_enter 	= 10
};

void scr_prep();
void scr_rest();
void quit(int);

#endif
