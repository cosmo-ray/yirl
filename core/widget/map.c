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

#include <unistd.h>
#include <glib.h>
#include "rect.h"
#include "map.h"
#include "widget-callback.h"
#include "entity-script.h"
#include "native-script.h"

static Entity *moveTblCallback;
static Entity *pushTblCallback;

static int t = -1;

static inline unsigned int nbPixInSprites(int spriteSize, int nbOfSprites)
{
  return spriteSize * nbOfSprites;
}

int ywMapType(Entity *map)
{
  return yeGetInt(yeGet(map, "map-type"));
}

void ywMapGetSpriteSize(Entity *map, unsigned int *sizeSpriteW,
			unsigned int *sizeSpriteH, unsigned int *thresholdX)
{
  Entity *widPix = yeGet(map, "wid-pix");
  int type = ywMapType(map);

  uint32_t winPixWidth = ywRectW(widPix);
  uint32_t winPixHight = ywRectH(widPix);
  uint32_t winWidth, winHeight;

  *thresholdX = 0;
  if (type == YMAP_PARTIAL) {
    winWidth = yeGetInt(yeGet(map, "cam-w"));
    winHeight = yeGetInt(yeGet(map, "cam-h"));
  } else {
    winWidth = ywMapW(map);
    winHeight = ywMapH(map);
  }
  // TODO: use real SPRITE SIZE
  // compute proportion:
  // x - y = diff
  // bigger = x < y ? x : y;
  // proportion = diff / bigger + 1.0

  /* Check if the number of sprites this window can
   * contain is superior to the actual width of the window */
  if (nbPixInSprites(YMAP_SIZE_SPRITE_W, winWidth) <  winPixWidth) {
    *sizeSpriteW = YMAP_SIZE_SPRITE_W;
    *sizeSpriteH = YMAP_SIZE_SPRITE_H;

  } else {
    *sizeSpriteW = YMAP_SIZE_SPRITE_W * winPixWidth /
      nbPixInSprites(YMAP_SIZE_SPRITE_W, winWidth);

    *sizeSpriteH = YMAP_SIZE_SPRITE_H * winPixHight /
      nbPixInSprites(YMAP_SIZE_SPRITE_H, winHeight);
    if (*sizeSpriteW > *sizeSpriteH) {
      *thresholdX = (winPixWidth - winPixHight) / 2 ;
      *sizeSpriteW = *sizeSpriteH;
    } else {
      *sizeSpriteH = *sizeSpriteW;
    }
  }
  if (thresholdX)
    *thresholdX += (winPixWidth % *sizeSpriteW) / 2;
}


void yeMapPixielsToPos(Entity *wid, uint32_t pixX, uint32_t pixY,
		       uint32_t *x, uint32_t *y)
{
  uint32_t spriteW, spriteH, thresholdX;

  ywMapGetSpriteSize(wid, &spriteW, &spriteH, &thresholdX);
  if (pixX < thresholdX)
    *x = 0;
  else
    *x = (pixX - thresholdX) / spriteW;
  *y = pixY / spriteH;
}

Entity *ywMapPosFromPixs(Entity *wid, uint32_t x, uint32_t y,
			 Entity *father, const char *name)
{
  uint32_t posX, posY;

  yeMapPixielsToPos(wid, x, y, &posX, &posY);
  if (posX >=  (uint32_t)ywMapW(wid))
    posX = ywMapW(wid) - 1;
  if (posY >= (uint32_t)ywMapH(wid))
    posY = ywMapW(wid) - 1;
  return ywPosCreateInts(posX, posY, father, name);
}

int ywMapMoveByStr(Entity *state, Entity *from,
		   Entity *to, const char *elem)
{
  Entity *cur = ywMapGetCase(state, from);
  Entity *tmp;

  if ((tmp = yeGet(cur, elem)) == NULL)
    return -1;

  YE_INCR_REF(tmp);
  yeRemoveChild(cur, tmp);
  ywMapPushElem(state, tmp, to, elem);
  YE_DESTROY(tmp);
  return 0;
}

int ywMapMoveByEntity(Entity *state, Entity *from,
		      Entity *to, Entity *elem)
{
  YE_INCR_REF(elem);
  ywMapRemove(state, from, elem);
  ywMapPushElem(state, elem, to, NULL);
  YE_DESTROY(elem);
  return 0;
}

static void *moveTbl(va_list ap)
{
  Entity *ent = va_arg(ap, Entity *);
  Entity *tbl = va_arg(ap, Entity *);

  ywMapMoveByEntity(ent, yeGet(tbl, 0), yeGet(tbl, 1), yeGet(tbl, 2));
  return NULL;
}


static void *pushTbl(va_list ap)
{
  Entity *ent = va_arg(ap, Entity *);
  Entity *tbl = va_arg(ap, Entity *);

  ywMapPushElem(ent, yeGet(tbl, 2), yeGet(tbl, 1), NULL);
  return NULL;
}

int ywMapSmootMove(Entity *state, Entity *from,
		      Entity *to, Entity *elem)
{
  if (!ywMapMvTablePush(state, from, to, elem, moveTblCallback))
    ywMapMoveByEntity(state, from, to, elem);
  return 0;
}

static int mapInitCheckResources(Entity *resources)
{
  Entity *firstELem = yeGet(resources, 0);

  if (unlikely(!resources || (yeType(resources) != YARRAY) ||
	       !yeLen(resources))) {
    DPRINT_ERR("can retrive ressources");
    return -1;
  } else if (unlikely(!firstELem || !yeLen(firstELem) || !yeLen(resources) ||
		      !(yeGet(firstELem, "map-char") ||
			yeGet(firstELem, "map-tild") ||
			yeGet(firstELem, "map-sprite")))) {
    DPRINT_ERR("resource bad format");
    return -1;
  }
  return 0;
}

static int mapInit(YWidgetState *opac, Entity *entity, void *args)
{
  Entity *resources;

  ywidGenericCall(opac, t, init);

  ((YMapState *)opac)->resources = yeGet(entity, "resources");
  resources = ((YMapState *)opac)->resources;
  if (mapInitCheckResources(resources) < 0)
    return -1;

  if (!yeStrCmp(yeGet(entity, "cam-type"), "center")) {
    yeReCreateInt(YMAP_PARTIAL, entity, "map-type");
    yeTryCreateInt(20, entity, "cam-w");
    yeTryCreateInt(20, entity, "cam-h");
    if (!yeGet(entity, "cam-pos"))
      ywPosCreateInts(0, 0, entity, "cam-pos");
  } else {
    yeReCreateInt(YMAP_FULL, entity, "map-type");
  }

  yeCreateArray(entity, "$mv_tbl");
  (void)args;
  return 0;
}

Entity *ywMapInitEntity(Entity *entity,
			Entity *resources,
			int baseId, uint32_t w, uint32_t h)
{
  uint32_t len = w * h;
  Entity *map = yeReCreateArray(entity, "map", NULL);

  if (resources)
    yeReplaceBack(entity, resources, "resources");
  yeReCreateInt(w, entity, "width");
  yeReCreateInt(len, entity, "len");
  if (baseId > -1) {
    for (uint32_t i = 0; i < len; ++i) {
      yeCreateInt(baseId, yeCreateArray(map, NULL), NULL);
    }
  }
  return entity;
}

int ywMapDrawRect(Entity *map, Entity *posStart, Entity *size, int id)
{
  Entity *mapElems = yeGet(map, "map");
  int x = ywPosX(posStart);
  int y = ywPosY(posStart);
  int w = ywPosX(size);
  int h = ywPosY(size);
  int mapW = ywMapW(map);
  int start = x + (y * mapW);
  int mapLen = ywMapLen(map);
  int lenX = w + x;
  int realEnd = start + w + ((h - 1) * mapW);

  if (start > mapLen || x > mapW)
    return -1;

  if (lenX > mapW) {
    w = mapW - w;
    lenX = mapW;
  }

  for (int i = start, curX = x, end = realEnd > mapLen ? mapLen : realEnd;
       i < end;) {
    Entity *tmp = yeGet(mapElems, i);

    if (!tmp)
      tmp = yeCreateArrayAt(mapElems, NULL, i);

    yeCreateInt(id, tmp, NULL);

    if (curX + 1 >= lenX) {
      curX = x;
      i += (mapW - w) + 1;
    } else {
      ++curX;
      ++i;
    }
  }
  return 0;
}


Entity *ywMapCreateDefaultEntity(Entity *father, const char *name,
				 Entity *resources,
				 int baseId, uint32_t w, uint32_t h)
{
  Entity *ret = yeCreateArray(father, name);
  return ywMapInitEntity(ret, resources, baseId, w, h);
}

Entity *ywMapPosFromInt(Entity *wid, int pos,
			Entity *father, const char *name)
{
  int w = yeGetInt(yeGet(wid, "width"));

  return ywPosCreate(pos % w, pos / w, father, name);
}

int ywMapIntFromPos(Entity *wid, Entity *pos)
{
  int w = yeGetInt(yeGet(wid, "width"));
  int x = yeGetInt(yeGet(pos, 0));
  int y = yeGetInt(yeGet(pos, 1));

  return x + y * w;
}


Entity *ywMapPushElem(Entity *state, Entity *toPush,
		      Entity *pos, const char *name)
{
  if (unlikely(!pos)) {
    pos = yeGet(toPush, "pos");
  }

  int ret = yePushBack(ywMapGetCase(state, pos), toPush, name);
  return ret ? toPush : NULL;
}

Entity *ywMapPushNbr(Entity *state, int toPush,
		     Entity *pos, const char *name)
{
  return yeCreateInt(toPush, ywMapGetCase(state, pos), name);
}

Entity *ywMapGetCase(Entity *state, Entity *pos)
{
  Entity *map = yeGet(state, "map");
  int w = yeGetInt(yeGet(state, "width"));
  Entity *ret;

  ret = yeGet(map, ywPosX(pos) + (w * ywPosY(pos)));
  if (unlikely(!ret)) {
    int iPos = ywPosX(pos) + (w * ywPosY(pos));

    if (!state)
      DPRINT_ERR("entity is NULL");
    else if (!map)
      DPRINT_ERR("unable to get 'map'");
    else if (!w)
      DPRINT_ERR("unable to get 'width'");
    else if (!pos)
      DPRINT_ERR("pos is NULL");
    else if (iPos < ywMapLen(state)) /* case fault ocure */
      ret = yeCreateArrayAt(map, NULL, iPos);
  }
  return ret;
}

static int mapDestroy(YWidgetState *opac)
{
  g_free(opac);
  return 0;
}

static int mapRend(YWidgetState *opac)
{
  ywidGenericRend(opac, t, render);
  return 0;
}

static void mapMidRend(YWidgetState *wid, int turnPercent)
{
  YUI_FOREACH_BITMASK(widgetOptTab[t].rendersMask, it, useless_tmask) {
    if (widgetOptTab[t].midRend[it])
      widgetOptTab[t].midRend[it](wid, it, turnPercent);
  }
  if (wid->shouldDraw) {
    ywidDrawScreen();
  }
}

static void mapMidRendEnd(YWidgetState *wid)
{
  Entity *ent = wid->entity;
  Entity *mv_tbl = yeGet(ent, "$mv_tbl");

  YE_ARRAY_FOREACH(mv_tbl, tbl) {
    yesCall(yeGet(tbl, 3), ent, tbl);
  }
  yeClearArray(mv_tbl);
}

int ywMapHasChange(YWidgetState *state)
{
  return state->hasChange;
}

static void *alloc(void)
{
  YMapState *ret = g_new0(YMapState, 1);
  YWidgetState *wstate = (YWidgetState *)ret;

  if (!ret)
    return NULL;

  wstate->render = mapRend;
  wstate->midRend = mapMidRend;
  wstate->midRendEnd = mapMidRendEnd;
  wstate->init = mapInit;
  wstate->destroy = mapDestroy;
  wstate->handleEvent = ywidEventCallActionSin;
  wstate->type = t;
  return  ret;
}

int ywMapGetIdByElem(Entity *mapElem)
{
  if (yeType(mapElem) == YINT)
    return yeGetInt(mapElem);
  if (yeType(mapElem) == YARRAY)
    return yeGetInt(yeGet(mapElem, "id"));

  return -1;
}

int ywMapInit(void)
{
  if (t != -1)
    return t;

  t = ywidRegister(alloc, "map");
  moveTblCallback = ysRegistreCreateNativeEntity(moveTbl, "moveTbl",
						 NULL, NULL);
  pushTblCallback = ysRegistreCreateNativeEntity(pushTbl, "pushTbl",
						 NULL, NULL);
  return t;
}

int ywMapEnd(void)
{
  if (ywidUnregiste(t) < 0)
    return -1;
  yeDestroy(moveTblCallback);
  yeDestroy(pushTblCallback);
  t = -1;
  return 0;
}

int ywMapIsSmoot(Entity *map);
int ywMapIsSmoot(Entity *map)
{
  return yeGetInt(yeGet(map, "$smoot"));
}

void ywMapMvTableRemove(Entity *map, Entity *to_rm)
{
  Entity *mv_tbl;

  mv_tbl = yeGet(map, "$mv_tbl");
  if (!mv_tbl)
    return;
  YE_ARRAY_FOREACH(mv_tbl, tbl) {
    Entity *movingElem = yeGet(tbl, 2);

    if (movingElem == to_rm) {
      yeRemoveChild(mv_tbl, tbl);
      return;
    }
  }
}

Entity *ywMapMvTablePush(Entity *map, Entity *from,
			 Entity *to, Entity *elem, Entity *callback)
{
  Entity *mv_tbl;
  Entity *ret;

  if (!ywMapIsSmoot(map))
    return NULL;
  mv_tbl = yeGet(map, "$mv_tbl");
  ret = yeCreateArray(mv_tbl, NULL);
  ywPosCreate(from, 0, ret, NULL);
  ywPosCreate(to, 0, ret, NULL);
  yePushBack(ret, elem, NULL);
  yePushBack(ret, callback, NULL);
  return ret;
}

int ywMapAdvenceWithPos(Entity *map, Entity *pos, int x, int y, Entity *elem)
{
  Entity *out_logic_entity;
  int out_logic;
  Entity *oldPos;

  if (unlikely(!elem || !map || ! pos)) {
    if (!map)
      DPRINT_WARN("map is NULL");
    else if (!elem)
      DPRINT_WARN("elem is NULL");
    else
      DPRINT_WARN("pos is NULL");
    return -1;
  }
  if (!x && !y) {
    return 0;
  }

  out_logic_entity = yeGet(map, "$out_logic");
  out_logic = out_logic_entity ? yeGetInt(out_logic_entity) : YMAP_OUT_WARP;
  YE_INCR_REF(elem);
  ywMapRemoveByEntity(map, pos, elem);
  oldPos = yeCreateArray(NULL, NULL);
  yeCopy(pos, oldPos);
  ywPosAddXY(pos, x, y);

  if (out_logic == YMAP_OUT_WARP) {
    if (ywPosX(pos) < 0)
      ywPosSetX(pos, ywMapW(map) + ywPosX(pos));
    else if (ywPosX(pos) >= ywMapW(map))
      ywPosSetX(pos, ywPosX(pos) - ywMapW(map));
    else if (ywPosY(pos) < 0)
      ywPosSetY(pos, ywMapH(map) + ywPosY(pos));
    else if (ywPosY(pos) >= ywMapH(map))
      ywPosSetY(pos, ywPosY(pos) - ywMapH(map));
    else
      goto push;
    ywMapPushElem(map, elem, pos, NULL);
    goto clean;
  } else if (out_logic == YMAP_OUT_BLOCK && !ywMapIsInside(map, pos)) {
    ywPosAddXY(pos, -x, -y);
    goto push;
  } else if (!ywMapIsInside(map, pos)) {
    goto clean;
  }
 push:
  if (!ywMapMvTablePush(map, oldPos, pos, elem, pushTblCallback))
    ywMapPushElem(map, elem, pos, NULL);
 clean:
  YE_DESTROY(oldPos);
  YE_DESTROY(elem);
  return 0;
}
