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
#include "sdl-internal.h"
#include "yirl/map.h"
#include "yirl/rect.h"
#include "yirl/entity.h"


#define PRINT_BG_AT(pos_at)						\
  if (likely(ywRectContainPos(cam, pos_at, 0))) {			\
    Entity *pos_at##mc = ywMapGetCase(ent, pos_at);			\
    YE_ARRAY_FOREACH(pos_at##mc, mapElem##pos_at) {			\
      if (yeFind(mv_tbl, findEnt, mapElem##pos_at)) {			\
	continue;							\
      }									\
      sdlDisplaySprites(state, wid, ywPosX(pos_at) - begX,		\
			ywPosY(pos_at) - begY,				\
			mapElem##pos_at, sizeSpriteW,			\
			sizeSpriteH,					\
			thresholdX, 0, NULL);				\
    }									\
  }									\

static void sdl2MidRender(YWidgetState *state, SDLWid *wid, Entity *ent,
			  int percent);

static Entity *findEnt(const char *useless, Entity *ent, void *ent2)
{
  (void)useless;
  if (yeGet(ent, 2) == ent2)
    return ent2;
  return NULL;
}

/* crop the map and print the middle of it */
static void sdl2PartialRender(YWidgetState *state, SDLWid *wid, Entity *entity)
{
  Entity *map = yeGet(entity, "map");
  Entity *cam = yeGet(entity, "cam");
  Entity *cam_threshold = yeGet(entity, "cam-threshold");
  Entity *cam_ptr = yeGet(entity, "cam-pointer");
  Entity *cam_getter = yeGet(entity, "cam-getter");
  int wMap = ywMapW(entity);
  int hMap = ywMapH(entity);
  int wCam = ywRectW(cam);
  int hCam = ywRectH(cam);
  unsigned int sizeSpriteW;
  unsigned int sizeSpriteH;
  int cptr_x = 0, cptr_y = 0;
  int32_t begX = ywRectX(cam) + ywPosX(cam_threshold);
  int32_t begY = ywRectY(cam) + ywPosY(cam_threshold);
  uint32_t thresholdX;
  Entity *elem_get = NULL;

  if (cam_getter)
	  elem_get = yeReCreateArray(entity, "elem-get", NULL);
  ywMapGetSpriteSize(entity, &sizeSpriteW, &sizeSpriteH, &thresholdX);

  if (begX < 0) {
	  cptr_x = begX;
	  begX = 0;
  } else if (begX > (wMap - wCam)) {
	  cptr_x = begX - (wMap - wCam);
	  begX = wMap - wCam;
  }

  if (begY < 0) {
	  cptr_y = begY;
	  begY = 0;
  } else if (begY > hMap - hCam) {
	  cptr_y = begY - (hMap - hCam);
	  begY = hMap - hCam;
  }

  for(int i = 0, curx = 0, cury = 0; i < wCam * hCam; ++i) {
	  int idx = begX + curx + ((cury + begY) * wMap);
	  Entity *mapCase = yeGet(map, idx);

	  YE_ARRAY_FOREACH(mapCase, mapElem) {
		  if (cam_getter &&
		      yeGetInt(cam_getter) == ywMapGetIdByElem(mapElem)) {
			  yePushBack(elem_get, mapElem, NULL);
			  yeReCreateInt(idx, mapElem, "_map_idx");
		  }
		  sdlDisplaySprites(state, wid, curx, cury, mapElem,
				    sizeSpriteW, sizeSpriteH,
				    thresholdX, 0, NULL);
	  }
	  ++curx;
	  if (curx >= wCam) {
		  curx = 0;
		  ++cury;
	  }
  }
  if (cam_ptr)
	  sdlDisplaySprites(state, wid,
			    -ywPosX(cam_threshold) + cptr_x,
			    -ywPosY(cam_threshold) + cptr_y,
			    cam_ptr, sizeSpriteW, sizeSpriteH,
			    thresholdX, 0, NULL);
}

/* rend all the map, regardeless if the map is bigger than the screen */
static int sdl2FullRender(YWidgetState *state, SDLWid *wid, Entity *entity)
{
	Entity *map = yeGet(entity, "map");
	unsigned int wMap = ywMapW(entity);
	unsigned int sizeSpriteW;
	unsigned int sizeSpriteH;
	uint32_t thresholdX;

	ywMapGetSpriteSize(entity, &sizeSpriteW, &sizeSpriteH, &thresholdX);

	YE_ARRAY_FOREACH_EXT(map, mapCase, it) {
		unsigned int curx = yBlockArrayIteratorIdx(it) % wMap;
		unsigned int cury = yBlockArrayIteratorIdx(it) / wMap;

		YE_ARRAY_FOREACH(mapCase, mapElem) {
			sdlDisplaySprites(state, wid, curx,
					  cury, mapElem,
					  sizeSpriteW,
					  sizeSpriteH, thresholdX,
					  0, NULL);
		}
	}

	return 0;
}

static void sdl2MidRender(YWidgetState *state, SDLWid *wid, Entity *ent,
			  int percent)
{
  Entity *gc = yeCreateArray(NULL, NULL);
  Entity *mv_tbl;
  unsigned int sizeSpriteW;
  unsigned int sizeSpriteH;
  uint32_t thresholdX;
  int wMap = ywMapW(ent);
  int hMap = ywMapH(ent);
  Entity *cam = yeGet(ent, "cam");
  // THIS IS MOST LIKELY BUGGY FOR MID RENDER
  Entity *cam_threshold = yeGet(ent, "cam-threshold");
  int wCam = ywRectW(cam);
  int hCam = ywRectH(cam);
  int32_t begX = ywRectX(cam) + ywPosX(cam_threshold);
  int32_t begY = ywRectY(cam) + ywPosY(cam_threshold);
  int32_t endX = begX + wCam;
  int32_t endY = begY + hCam;
  YBgConf cfg;

  if (unlikely(!hMap || !wMap)) {
    DPRINT_ERR("can't rend empty map\n");
    return;
  }

  if (ywidBgConfFill(yeGet(ent, "background"), &cfg) >= 0) {
    sdlFillBg(wid, &cfg);
  }

  if (ywMapType(ent) != YMAP_PARTIAL)
    sdl2FullRender(state, wid, ent);
  else
    sdl2PartialRender(state, wid, ent);

  if (!ywIsSmootOn)
    return;

  if (begX < 0) {
    begX = 0;
    endX = begX + wCam;
  } else if (endX > wMap) {
    begX = wMap - wCam;
    endX = wMap;
  }

  if (begY < 0) {
    begY = 0;
    endY = begY + hCam;
  } else if (endY > hMap) {
    begY = hMap - hCam;
    endY = hMap;
  }

  ywMapGetSpriteSize(ent, &sizeSpriteW, &sizeSpriteH, &thresholdX);
  mv_tbl = yeGet(ent, "$mv_tbl");
  YE_ARRAY_FOREACH(mv_tbl, tbl) {
    Entity *from = yeGet(tbl, 0);
    Entity *to = yeGet(tbl, 1);
    Entity *seg = ywPosDoPercent(ywSegmentFromPos(from, to,
                                                  gc, NULL),
                                 percent);
    ywPosAdd(from, seg);
    {PRINT_BG_AT(from)};
    ywPosAddXY(from, 0, 1); // 0/1
    {PRINT_BG_AT(from)};
    ywPosAddXY(from, 1, 0); // 1/1
    {PRINT_BG_AT(from)};
    ywPosAddXY(from, -2, 0); // -1/1
    {PRINT_BG_AT(from)};
    ywPosAddXY(from, 0, -1); // -1/0
    {PRINT_BG_AT(from)};
    ywPosAddXY(from, 2, 0); // 1/0
    {PRINT_BG_AT(from)};
    ywPosAddXY(from, 0, -1); // 1/-1
    {PRINT_BG_AT(from)};
    ywPosAddXY(from, -1, 0); // 0/-1
    {PRINT_BG_AT(from)};
    ywPosAddXY(from, -1, 0); // -1/-1
    {PRINT_BG_AT(from)};
    ywPosAddXY(from, 1, 1); // 0/0
    yeClearArray(gc);
  }


  YE_ARRAY_FOREACH(mv_tbl, tbl2) {
    Entity *from = yeGet(tbl2, 0);
    Entity *to = yeGet(tbl2, 1);
    Entity *seg = ywPosDoPercent(ywPosMultXY(ywSegmentFromPos(from, to,
							      gc, NULL),
					     sizeSpriteW, sizeSpriteH),
				 percent);
    Entity *movingElem = yeGet(tbl2, 2);
    Entity *modifier = NULL;

    if (!ywRectContainPos(cam, from, 0)) {
      if (!ywRectContainPos(cam, to, 0))
	continue;
      modifier = yeCreateArray(gc, NULL);
      // type of modifier, 0 because for now is the only modifier
      yeCreateInt(0, modifier, NULL);
      if (begX == ywPosX(to) && ywPosX(to) > ywPosX(from)) {
      	yeCreateInts(modifier, yuiPercentOf(sizeSpriteW, 100 - percent),
      		     yuiPercentOf(sizeSpriteW, 100 - percent), 100 - percent,
		     100 - percent);
      }
      else if (endX == ywPosX(from) && ywPosX(to) < ywPosX(from)) {
      	yeCreateInts(modifier, 0, yuiPercentOf(sizeSpriteW, 100 - percent), 0,
		     100 - percent);
      }
    } else if (!ywRectContainPos(cam, to, 0)) {
      modifier = yeCreateArray(gc, NULL);
      // type of modifier, 0 because for now is the only modifier
      yeCreateInt(0, modifier, NULL);
      if (begX == ywPosX(from) && ywPosX(to) < ywPosX(from)) {
      	yeCreateInts(modifier, yuiPercentOf(sizeSpriteW, percent),
      		     yuiPercentOf(sizeSpriteW, percent), percent,
		     percent);
      } else if (endX == ywPosX(to) && ywPosX(to) > ywPosX(from)) {
      	yeCreateInts(modifier, 0, yuiPercentOf(sizeSpriteW, percent), 0,
		     percent);
      }
    }

    sdlDisplaySprites(state, wid, ywPosX(from) - begX,
		      ywPosY(from) - begY,
		      movingElem, sizeSpriteW, sizeSpriteH,
		      thresholdX + ywPosX(seg),
		      ywPosY(seg), modifier);

    yeClearArray(gc);
  }

  if (ywidBgConfFill(yeGet(state->entity, "foreground"), &cfg) >= 0)
    sdlFillBg(wid, &cfg);

  yeDestroy(gc);
}

static int sdl2Render(YWidgetState *state, int t)
{
  SDLWid *wid = ywidGetRenderData(state, t);
  Entity *ent = state->entity;

  sdl2MidRender(state, wid, ent, ywTurnPecent);
  return 0;
}

static int sdl2Init(YWidgetState *wid, int t)
{
  wid->renderStates[t].opac = y_new(SDLWid, 1);
  sdlWidInit(wid, t);
  return 0;
}


int ysdl2RegistreMap(void)
{
  int ret = ywidRegistreTypeRender("map", ysdl2Type(),
				   sdl2Render, sdl2Init, sdlWidDestroy);
  return ret;
}

#undef PRINT_BG_AT
