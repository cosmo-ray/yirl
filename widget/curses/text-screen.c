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


#include <curses.h>
#include <string.h>
#include <glib.h>
#include "curses-state.h"
#include "widget.h"
#include "entity.h"

static int cursesRender(YWidgetState *state, int t)
{
  CWidget *wid = ywidGetRenderData(state, t);
  int x,y,h,w;
  const char *toPrint = yeGetString(yeGet(state->entity, "text"));
  int   len = strlen(toPrint);

  /* wrefresh(wid->win); */
  wmove(wid->win,0,0);
  getyx(wid->win, y, x);
  getmaxyx(wid->win, h, w);
  w -= x;
  h -= y;
  (void) h; // for debug print
  DPRINT_INFO("pos: h: %d w: %d x: %d y: %d\n", h, w, x, y);

  //iterator on the string pos
  int it = 0;
  int itX;
  for (int itY = 1; it < len; ++itY)
    {
      for (itX = 1; itX < (w - 1) && it < len; ++itX)
	{
	  if (toPrint[it] == '\n')
	    {
	      ++it;
	      break;
	    }
	  mvwaddch(wid->win, itY, itX, toPrint[it]);
	  ++it;
	}
    }
  wprintw(wid->win, "bye i love you %s\n", toPrint);
  wrefresh(wid->win);
  return (0);
}

static int cursesInit(YWidgetState *wid, int t)
{
  wid->renderStates[t].opac = g_new(CWidget, 1);
  CWidgetInit(wid, t);
  return 0;
}

int ycursRegistreTextScreen(void)
{
  return ywidRegistreTypeRender("text-screen", ycursType(), cursesRender, NULL, cursesInit, CWidgetDestroy);
}
