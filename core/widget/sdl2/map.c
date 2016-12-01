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

/* crop the map and print the middle of it */
static int sdl2PartialRender(YWidgetState *state, SDLWid *wid, Entity *entity)
{
  unsigned int curx = 0, cury = 0;
  Entity *map = yeGet(entity, "map");
  unsigned int wMap = ywMapW(entity);
  unsigned int wCam = yeGetInt(yeGet(entity, "cam-w"));
  unsigned int hCam = yeGetInt(yeGet(entity, "cam-h"));
  YBgConf cfg;
  unsigned int sizeSpriteW;
  unsigned int sizeSpriteH;
  int posCam = yeGetInt(yeGet(entity, "cam-pos"));
  int32_t begX = posCam;
  Entity *mapCase;
  int thresholdX;

  if (ywidBgConfFill(yeGet(entity, "background"), &cfg) >= 0) {
    sdlFillBg(wid, &cfg);
  }

  ywMapGetSpriteSize(state, &sizeSpriteW, &sizeSpriteH);
  thresholdX = (wid->rect.w % sizeSpriteW) / 2;

  for(unsigned int i = 0; i < wCam * hCam &&
	(mapCase = yeGet(map, begX + curx + (cury * wMap))); ++i) {

    YE_ARRAY_FOREACH(mapCase, mapElem) {
      sdlDisplaySprites(state, wid, curx, cury, mapElem,
			sizeSpriteW, sizeSpriteH, thresholdX);
    }
    ++curx;
    if (curx >= wCam) {
      curx = 0;
      ++cury;
    }
  }

  return 0;
}

/* rend all the map, regardeless if the map is bigger than the screen */
static int sdl2FullRender(YWidgetState *state, SDLWid *wid, Entity *entity)
{
  Entity *map = yeGet(entity, "map");
  unsigned int lenMap = ywMapLen(entity);
  unsigned int wMap = ywMapW(entity);
  YBgConf cfg;
  unsigned int hMap = lenMap / wMap;
  unsigned int sizeSpriteW;
  unsigned int sizeSpriteH;
  int thresholdX;

  if (unlikely(!hMap || !wMap || !yeLen(map))) {
    DPRINT_ERR("can't rend empty map\n");
    return -1;
  }

  if (ywidBgConfFill(yeGet(entity, "background"), &cfg) >= 0) {
    sdlFillBg(wid, &cfg);
  }

  ywMapGetSpriteSize(state, &sizeSpriteW, &sizeSpriteH);
  thresholdX = (wid->rect.w % sizeSpriteW) / 2;

  YE_ARRAY_FOREACH_EXT(map, mapCase, it) {
    unsigned int curx = yBlockArrayIteratorIdx(it) % wMap;
    unsigned int cury = yBlockArrayIteratorIdx(it) / wMap;

    YE_ARRAY_FOREACH(mapCase, mapElem) {
      if (unlikely(sdlDisplaySprites(state, wid, curx, cury, mapElem,
				     sizeSpriteW, sizeSpriteH, thresholdX) < 0)) {
	sdlConsumeError();
      }
    }
  }

  return 0;
}


static int sdl2Render(YWidgetState *state, int t)
{
  SDLWid *wid = ywidGetRenderData(state, t);
  Entity *ent = state->entity;

  if (((YMapState *)state)->renderType == YMAP_PARTIAL)
    return sdl2PartialRender(state, wid, ent);
  return sdl2FullRender(state, wid, ent);
}

static int sdl2Init(YWidgetState *wid, int t)
{
  wid->renderStates[t].opac = g_new(SDLWid, 1);
  sdlWidInit(wid, t);
  return 0;
}

int ysdl2RegistreMap(void)
{
  int ret = ywidRegistreTypeRender("map", ysdl2Type(),
				   sdl2Render, sdl2Init, sdlWidDestroy);
  return ret;
}
