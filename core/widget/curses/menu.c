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
#include "widget.h"
#include "curses-state.h"
#include "entity.h"
#include "menu.h"

static int cursesRender(YWidgetState *state, int t)
{
  CWidget *wid = ywidGetRenderData(state, t);
  int x,y,h,w;
  Entity *entries = yeGet(state->entity, "entries");
  unsigned int   len = yeLen(entries);

  wmove(wid->win,0,0);
  getyx(wid->win, y, x);
  getmaxyx(wid->win, h, w);
  w -= x;
  h -= y;
  (void) h; // for debug print
  (void) w; // for debug print
  DPRINT_INFO("pos: h: %d w: %d x: %d y: %d\n", h, w, x, y);

  for (unsigned int i = 0; i < len; ++i)
    {
      Entity *entry = yeGet(entries, i);
      const char *toPrint = yeGetString(yeGet(entry, "text"));
      unsigned int cur = ywMenuGetCurrent(state);
      if (toPrint) {
	if (cur == i)
	  wattron(wid->win, COLOR_PAIR(1));
	mvwaddstr(wid->win, i + 1, 1, toPrint);
	if (cur == i)
	  wattroff(wid->win, COLOR_PAIR(1));
      }
    }
  wrefresh(wid->win);
  return 0;
}

static int cursesInit(YWidgetState *wid, int t)
{
  wid->renderStates[t].opac = g_new(CWidget, 1);
  CWidgetInit(wid, t);
  return 0;
}

int ycursRegistreMenu(void)
{
  return ywidRegistreTypeRender("menu", ycursType(),
				cursesRender, cursesInit, CWidgetDestroy);
}
