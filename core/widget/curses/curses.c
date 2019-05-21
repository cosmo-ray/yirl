/*
**Copyright (C) 2015 Matthias Gatto
**
**This program is free software: you can redistribute it and/or modify
**it under the terms of the GNU Lesser General Public License as published by
**the Free Software Foundation, either version 3 of the License, or
**(at your option) any later version.
**
**This program is distributed in the hope that it will be useful,
**but WITHOUT ANY WARRANTY; without even the implied warranty of
**MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**GNU General Public License for more details.
**
**You should have received a copy of the GNU Lesser General Public License
**along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <glib.h>
#include "curses-state.h"

/* The type of rander */
static int type = -1;

int ycursType(void)
{
  return type;
}

static int CDraw(void)
{
  return 0;
}

int ycursInit(void)
{
  if (type != -1)
    return type;
  initscr();
  noecho();
  cbreak();
  curs_set(FALSE);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(1, COLOR_BLACK, COLOR_WHITE);
  type = ywidRegistreRender(resize, CPollEvent, CWaitEvent, CDraw, NULL, NULL);
  return type;
}

void ycursDestroy(void)
{
  if (type == -1)
    return;
  endwin();
  ywidRemoveRender(type);
  type = -1;
}

static inline Entity *CGetEvent(void)
{
	YE_NEW(Array, eve);
	static struct { int c; unsigned int turn_id; }  buf[128];
	static int last;
	int c = getch();

	if (c == ERR) {
		for (int i = last - 1; i >= 0; --i) {
			if (buf[i].turn_id != ywTurnId()) {
				--last;
				c = buf[i].c;
				if (i != last) {
					buf[i].c = buf[last].c;
					buf[i].turn_id = buf[last].turn_id;
				}
				yeCreateIntAt(YKEY_UP, eve, NULL, YEVE_TYPE);
				goto end;
			}
		}
		return NULL;
	}  else {
		yeCreateIntAt(YKEY_DOWN, eve, NULL, YEVE_TYPE);
		for (int i = 0; i < last; ++i) {
			if (buf[i].c == c) {
				buf[i].turn_id = ywTurnId();
				goto end;
			}
		}
		buf[last].c = c;
		buf[last].turn_id = ywTurnId();
		++last;
	}
end:
	yeIncrRef(eve);
	yeCreateIntAt(c, eve, NULL, YEVE_KEY);
	yeCreateIntAt(NOTHANDLE, eve, NULL, YEVE_STATUS);
	return eve;
}


Entity *CWaitEvent(void)
{
    timeout(-1);

    return CGetEvent();
}

Entity *CPollEvent(void)
{
    timeout(0);

    return CGetEvent();
}

void CWidgetInit(YWidgetState *wid, int renderType)
{
  CWidget *state = wid->renderStates[renderType].opac;

  state->win = newwin(10, 10, 0, 0);
  resize(wid, renderType);
}

void CWidgetDestroy(YWidgetState *wid, int renderType)
{
  CWidget *state = wid->renderStates[renderType].opac;

  delwin(state->win);
  g_free(state);
}

void resize(YWidgetState *wid, int renderType)
{
  CWidget *state = wid->renderStates[renderType].opac;
  Entity *pos = yeGet(wid->entity, "wid-pos");

  state->h = yeGetInt(yeGet(pos, "h")) * LINES / 1000;
  state->w = yeGetInt(yeGet(pos, "w")) * COLS / 1000;
  state->x = yeGetInt(yeGet(pos, "x")) * COLS / 1000;
  state->y = yeGetInt(yeGet(pos, "y")) * LINES / 1000;

  wresize(state->win, state->h, state->w);
  mvwin(state->win, state->y, state->x);
  wborder(state->win, '|', '|', '-','-','+','+','+','+');
  refresh();
}
