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

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include <glib.h>
#include "sdl-internal.h"
#include "map.h"
#include "entity.h"

#define	SIZE_SPRITE_W  50
#define	SIZE_SPRITE_H  50

static int sdl2Render(YWidgetState *state, int t)
{
  SDLWid *wid = ywidGetRenderData(state, t);
  /* int x,y,h,w; */
  unsigned int curx = 0, cury = 0;
  Entity *map = yeGet(state->entity, "map");
  unsigned int lenMap = yeLen(map);
  unsigned int wMap = yeGetInt(yeGet(state->entity, "width"));
  YBgConf cfg;
  /* unsigned int hMap = lenMap / wMap; */

  if (!ywMapHasChange(state))
    return 0;
  if (ywidBgConfFill(yeGet(state->entity, "background"), &cfg) < 0) {
    DPRINT_ERR("fail to pasre background");
    cfg.type = BG_COLOR;
    cfg.path = (char *)&cury; // eat your not so random color b****
    sdlFillBg(wid, &cfg);
  } else if (sdlFillBg(wid, &cfg) < 0) {

    DPRINT_ERR("fail to draw background");    
  }

  for (unsigned int i = 0; i < lenMap; ++i)
    {
      Entity *mapCase = yeGet(map, i);
      int id = yeGetInt(yeGet(mapCase, yeLen(mapCase) - 1)) + '0';

      if (curx >= wMap)
	{
	  curx = 0;
	  ++cury;
	}

      sdlDisplaySprites(wid, curx, cury, id,
			SIZE_SPRITE_W, SIZE_SPRITE_H, 0);
      ++curx;
    }

  SDL_RenderPresent(sgRenderer());
  return 0;
}

static int sdl2Init(YWidgetState *wid, int t)
{
  wid->renderStates[t].opac = g_new(SDLWid, 1);
  sdlWidInit(wid, t);
  return 0;
}

int ysdl2RegistreMap(void)
{
  return ywidRegistreTypeRender("map", ysdl2Type(),
				sdl2Render, sdl2Init, sdlWidDestroy);
}
