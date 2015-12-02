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

static inline unsigned int nbSprite(int sizePix, int sizeCase)
{
  return sizePix * sizeCase;
}

#define YMAP_FOREACH_ELEMS(map, elem, caseIncrOp)	\
  YMAP_FOREACH_CASES(map, mapCaseTmp)			\
    YMAP_FOREACH_ELEMS_IN_CASE(mapCaseTmp, elem)

#define YMAP_FOREACH_ELEMS_IN_CASE(mapCase, elem)		\
  Entity *elem;							\
  for (uint32_t j = 0; j < yeLen(mapCase) &&			\
	 ((elem = yeGet(mapCase, j)) || 1); ++j)


#define YMAP_FOREACH_CASES(map, mapCase)		\
  Entity *mapCase;					\
  for(uint32_t i = 0; i < yeLen(map) &&			\
	(mapCase = yeGet(map, i)); ++i)


static int sdl2Render(YWidgetState *state, int t)
{
  SDLWid *wid = ywidGetRenderData(state, t);
  unsigned int curx = 0, cury = 0;
  Entity *map = yeGet(state->entity, "map");
  unsigned int lenMap = yeLen(map);
  unsigned int wMap = yeGetInt(yeGet(state->entity, "width"));
  YBgConf cfg;
  unsigned int winPixWidth = wid->rect.w;
  unsigned int winPixHight = wid->rect.h;
  unsigned int hMap = lenMap / wMap;
  unsigned int sizeSpriteW;
  unsigned int sizeSpriteH;

  if (!ywMapHasChange(state))
    return 0;
  if (ywidBgConfFill(yeGet(state->entity, "background"), &cfg) >= 0) {
    sdlFillBg(wid, &cfg);
  }

  /* Check if the number of sprites this window can
   * contain is superior to the actual width of the window */
  if (nbSprite(SIZE_SPRITE_W, wMap) <  winPixWidth) {
    sizeSpriteW = SIZE_SPRITE_W;
    sizeSpriteH = SIZE_SPRITE_H;

  } else {
    sizeSpriteW = SIZE_SPRITE_W * winPixWidth /
      nbSprite(SIZE_SPRITE_W, wMap);
    sizeSpriteH = SIZE_SPRITE_H * winPixHight /
      nbSprite(SIZE_SPRITE_H, hMap);
  }

  YMAP_FOREACH_CASES(map, mapCase) {

    if (curx >= wMap) {
      curx = 0;
      ++cury;
    }

    YMAP_FOREACH_ELEMS_IN_CASE(mapCase, mapElem) {
      int id;
      Entity *curRes;

      if (!mapElem)
	continue;
      id = yeGetInt(mapElem);
      curRes = yeGet(ywMapGetResources(state), id);
      sdlDisplaySprites(wid, curx, cury, curRes,
			sizeSpriteW, sizeSpriteH, 0);
    }
    ++curx;
  }

  SDL_RenderPresent(sgRenderer());
  return 0;
}

#undef YMAP_FOREACH_ELEMS
#undef YMAP_FOREACH_CASES
#undef YMAP_FOREACH_ELEMS_IN_CASES

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
