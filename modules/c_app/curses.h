#ifndef CURSES_H_
#define CURSES_H_

#include <yirl/all.h>
#include <stdbool.h>

Entity *cur_wid(void);

struct WINDOW;
typedef struct WINDOW WINDOW;

struct SCREEN;
typedef struct SCREEN SCREEN;

static int COLS;
static int LINES;
static WINDOW *stdscr;

/* colors */
#define COLOR_BLACK     0
#define COLOR_RED       1
#define COLOR_GREEN     2
#define COLOR_YELLOW    3
#define COLOR_BLUE      4
#define COLOR_MAGENTA   5
#define COLOR_CYAN      6
#define COLOR_WHITE     7

typedef uint32_t chtype;
typedef uint32_t mmask_t;
typedef chtype  attr_t;

static int COLOR_PAIR(int n)
{
	printf("color pair %d\n", n);
	return 0;
}

static int addch(const chtype ch)
{
	printf("addch %d\n", ch);
	return 0;
}

static int waddch(WINDOW *win, const chtype ch)
{
	printf("waddch %p %d\n", win, ch);
	return 0;
}

static int mvaddch(int y, int x, const chtype ch)
{
	Entity *texts = yeGet(cur_wid(), "text");
	Entity *line = yeGet(texts, y);

	/* printf("text: %p - line %p\n", texts, line); */
	if (!line || x < 0)
		return 0;
	yeStringReplaceCharAt(line, ch, x);
	return 0;
}

static int mvwaddch(WINDOW *win, int y, int x, const chtype ch)
{
	printf("wmvaddch %p %d %d %d\n", win, x, y, ch);
	return 0;
}

static int echochar(const chtype ch)
{
	printf("echochar %d\n", ch);
	return 0;
}

static int wechochar(WINDOW *win, const chtype ch)
{
	printf("wechochar %p %d\n", win, ch);
	return 0;
}

static WINDOW *initscr(void)
{
	printf("initscr %d - %d\n", ywWidth(cur_wid()) / ywidFontW(),
	       ywHeight(cur_wid())/ ywidFontH());
	COLS = ywWidth(cur_wid()) / ywidFontW();
	LINES = ywHeight(cur_wid())/ ywidFontH();
	return NULL;
}

static int endwin(void)
{
	printf("endwin\n");
	return 0;
}

static bool isendwin(void)
{
	printf("isendwin\n");
	return 0;
}

static SCREEN *newterm(const char *type, FILE *outfd, FILE *infd)
{
	printf("newterm %s %p %p\n", type, outfd, infd);
	return NULL;
}

static SCREEN *set_term(SCREEN *new)
{
	printf("*set_term %p\n", new);
	return NULL;
}

static void delscreen(SCREEN *sp)
{
	printf("delscreen %p\n", sp);
}

static int start_color(void)
{
	printf("start_color\n");
	return 0;
}

static int noecho(void)
{
	printf("no echo\n");
	return 0;
}

static int curs_set(int visibility)
{
	printf("curs_set: %d\n", visibility);
	return 0;
}

static bool has_colors(void)
{
	return 1;
}

static int init_pair(short pair, short f, short b)
{
	printf("init_pair: %d %d %d\n", pair, f, b);
	return 0;
}

static int nodelay(WINDOW *win, bool bf)
{
	printf("nodelay %p %d\n", win, bf);
	return 0;
}

static int leaveok(WINDOW *win, bool bf)
{
	printf("leaveok %p %d\n", win, bf);
	return 0;
}

static int scrollok(WINDOW *win, bool bf)
{
	printf("scrollok %p %d\n", win, bf);
	return 0;
}

static int getch(void)
{
	printf("getch\n");
	return 0;
}

static int refresh(void)
{
	ywidRend(ywidGetMainWid());
	printf("refresh\n");
	return 0;
}

static int mvcur(int oldrow, int oldcol, int newrow, int newcol)
{
	printf("mvcur %d %d %d %d\n", oldrow, oldcol, newrow, newcol);
	return 0;
}

static int attroff(int attrs)
{
	printf("attroff %d\n", attrs);
	return 0;
}

static int attron(int attrs)
{
	printf("attron %d\n", attrs);
	return 0;
}

#endif
