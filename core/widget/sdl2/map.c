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

int ywMapIsSmoot(Entity *map);

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

  ywMapGetSpriteSize(entity, &sizeSpriteW, &sizeSpriteH);
  thresholdX = (wid->rect.w % sizeSpriteW) / 2;

  for(unsigned int i = 0; i < wCam * hCam &&
	(mapCase = yeGet(map, begX + curx + (cury * wMap))); ++i) {

    YE_ARRAY_FOREACH(mapCase, mapElem) {
      sdlDisplaySprites(state, wid, curx, cury, mapElem,
			sizeSpriteW, sizeSpriteH, thresholdX, 0);
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

  ywMapGetSpriteSize(entity, &sizeSpriteW, &sizeSpriteH);
  thresholdX = (wid->rect.w % sizeSpriteW) / 2;

  YE_ARRAY_FOREACH_EXT(map, mapCase, it) {
    unsigned int curx = yBlockArrayIteratorIdx(it) % wMap;
    unsigned int cury = yBlockArrayIteratorIdx(it) / wMap;

    YE_ARRAY_FOREACH(mapCase, mapElem) {
      if (unlikely(sdlDisplaySprites(state, wid, curx, cury, mapElem,
				     sizeSpriteW, sizeSpriteH, thresholdX,
				     0) < 0)) {
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

  if (ywMapType(ent) == YMAP_PARTIAL)
    return sdl2PartialRender(state, wid, ent);
  return sdl2FullRender(state, wid, ent);
}

static int sdl2Init(YWidgetState *wid, int t)
{
  wid->renderStates[t].opac = g_new(SDLWid, 1);
  sdlWidInit(wid, t);
  return 0;
}

static void sdl2MidFullRender(YWidgetState *state, SDLWid *wid, Entity *ent,
			      int percent)
{
  Entity *gc = yeCreateArray(NULL, NULL);
  Entity *mv_tbl;
  unsigned int sizeSpriteW;
  unsigned int sizeSpriteH;
  int thresholdX;
  int thresholdY = 0;

  if (!ywMapIsSmoot(ent))
    return;

  ywMapGetSpriteSize(ent, &sizeSpriteW, &sizeSpriteH);
  thresholdX = (wid->rect.w % sizeSpriteW) / 2;
  mv_tbl = yeGet(ent, "$mv_tbl");
  YE_ARRAY_FOREACH(mv_tbl, tbl) {
    Entity *from = yeGet(tbl, 0);
    Entity *to = yeGet(tbl, 1);
    Entity *movingElem = yeGet(tbl, 2);

#define PRINT_BG_AT(pos_at)						\
    Entity *pos_at##mc = ywMapGetCase(ent, pos_at);			\
    YE_ARRAY_FOREACH(pos_at##mc, mapElem##pos_at) {			\
      if (mapElem##pos_at == movingElem) {				\
	continue;							\
      }									\
      if (unlikely(sdlDisplaySprites(state, wid, ywPosX(pos_at), ywPosY(pos_at), \
				     mapElem##pos_at, sizeSpriteW, sizeSpriteH, \
				     thresholdX, 0) < 0)) {		\
	sdlConsumeError();						\
      }									\
    }

    PRINT_BG_AT(from);
    PRINT_BG_AT(to);

#undef PRINT_BG_AT
  }

  YE_ARRAY_FOREACH(mv_tbl, tbl2) {
    Entity *from = yeGet(tbl2, 0);
    Entity *to = yeGet(tbl2, 1);
    Entity *seg = ywPosDoPercent(ywPosMultXY(ywSegmentFromPos(from, to,
							      gc, NULL),
					     sizeSpriteW, sizeSpriteH),
				 percent);
    Entity *movingElem = yeGet(tbl2, 2);

    if (unlikely(sdlDisplaySprites(state, wid, ywPosX(from), ywPosY(from),
				   movingElem, sizeSpriteW, sizeSpriteH,
				   thresholdX + ywPosX(seg),
				   thresholdY + ywPosY(seg)) < 0)) {
      sdlConsumeError();
    }

    yeClearArray(gc);
  }
  yeDestroy(gc);
}

static void midRender(YWidgetState *state, int t, int percent)
{
  SDLWid *wid = ywidGetRenderData(state, t);
  Entity *ent = state->entity;

  if (ywMapType(ent) == YMAP_PARTIAL)
    return;
  return sdl2MidFullRender(state, wid, ent, percent);
}

int ysdl2RegistreMap(void)
{
  int ret = ywidRegistreTypeRender("map", ysdl2Type(),
				   sdl2Render, sdl2Init, sdlWidDestroy);
  ywidRegistreMidRend(midRender, ret, ysdl2Type());
  return ret;
}
