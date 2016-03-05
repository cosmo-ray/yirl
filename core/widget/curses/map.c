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
#include "entity.h"
#include "map.h"

static char getPrintableChar(Entity *mapCases, Entity *res)
{
  size_t ret = ywMapGetIdByElem(yeGet(mapCases, yeLen(mapCases) - 1));

  res = yeGet(yeGet(res, ret), "map-char");
  return res != NULL ? yeGetString(res)[0] : '-';
}

static int cursesRender(YWidgetState *state, int t)
{
  CWidget *wid = ywidGetRenderData(state, t);
  int x,y,h,w;
  unsigned int curx = 0, cury = 0;
  Entity *map = yeGet(state->entity, "map");
  Entity *res = ywMapGetResources(state);
  unsigned int lenMap = yeLen(map);
  unsigned int wMap = yeGetInt(yeGet(state->entity, "width"));
  unsigned int hMap = lenMap / wMap;

  wmove(wid->win,0,0);
  getyx(wid->win, y, x);
  getmaxyx(wid->win, h, w);
  w -= x;
  h -= y;

  for (unsigned int i = 0; i < lenMap; ++i)
    {
      Entity *mapCase = yeGet(map, i);

      if (curx >= wMap)
	{
	  curx = 0;
	  ++cury;
	}

      mvwaddch(wid->win,
	       (cury + 1) + (h/2 - hMap / 2),
	       (curx + 1) + (w/2 - wMap / 2),
	       getPrintableChar(mapCase, res));
      ++curx;
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

int ycursRegistreMap(void)
{
  int ret = ywidRegistreTypeRender("map", ycursType(),
				cursesRender, cursesInit, CWidgetDestroy);
  return ret;
}
