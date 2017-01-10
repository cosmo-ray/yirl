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
			unsigned int *sizeSpriteH)
{
  Entity *widPix = yeGet(map, "wid-pix");
  int type = ywMapType(map);

  uint32_t winPixWidth = ywRectW(widPix);
  uint32_t winPixHight = ywRectH(widPix);
  uint32_t winWidth, winHeight;

  if (type == YMAP_PARTIAL) {
    winWidth = yeGetInt(yeGet(map, "cam-w"));
    winHeight = yeGetInt(yeGet(map, "cam-h"));
  } else {
    winWidth = ywMapW(map);
    winHeight = ywMapH(map);
  }
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
  }
}


void yeMapPixielsToPos(Entity *wid, uint32_t pixX, uint32_t pixY,
		       uint32_t *x, uint32_t *y)
{
  uint32_t spriteW, spriteH;

  ywMapGetSpriteSize(wid, &spriteW, &spriteH);
  *x = pixX / spriteW;
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

  if (yuiStrEqual0(yeGetString(yeGet(entity, "cam-type")), "center")) {
    yeReCreateInt(YMAP_PARTIAL, entity, "map-type");
    if (!yeGet(entity, "cam-pos"))
      yeCreateInt(0, entity, "cam-pos");
  } else {
    yeReCreateInt(YMAP_FULL, entity, "map-type");
  }

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
    else if (!yeGet(pos, "x"))
      DPRINT_ERR("unable to get 'x' in pos");
    else if (!yeGet(pos, "y"))
      DPRINT_ERR("unable to get 'w' in pos");
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
  return t;
}

int ywMapEnd(void)
{
  if (ywidUnregiste(t) < 0)
    return -1;
  t = -1;
  return 0;
}
