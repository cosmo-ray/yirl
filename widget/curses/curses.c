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

#include <glib.h>
#include "curses-state.h"

/* The type of rander */
static int type = -1;

int ycursType(void)
{
  return type;
}

int ycursInit(void)
{
  initscr();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(1, COLOR_BLACK, COLOR_WHITE);
  if (type == -1)
    type = ywidRegistreRender(resize);
  return type;
}

void ycursDestroy(void)
{
  endwin();
  ywidRemoveRender(type);
}

void CWidgetInit(YWidgetState *wid, int renderType)
{
  CWidget *state = wid->renderStates[renderType].opac;

  state->h = wid->pos.h * LINES / 1000;
  state->w = wid->pos.w * COLS / 1000;
  state->x = wid->pos.x * COLS / 1000;
  state->y = wid->pos.y * LINES / 1000;
  state->win = newwin(state->h, state->w, state->y, state->x);
  wborder(state->win, '|', '|', '-','-','+','+','+','+');
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

  state->h = wid->pos.h * LINES / 1000;
  state->w = wid->pos.w * COLS / 1000;
  state->x = wid->pos.x * COLS / 1000;
  state->y = wid->pos.y * LINES / 1000;

  wresize(state->win, state->h, state->w);
  mvwin(state->win, state->y, state->x);
}
