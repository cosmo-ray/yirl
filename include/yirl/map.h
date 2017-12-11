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

#ifndef	_YIRL_MAP_H_
#define	_YIRL_MAP_H_

#include "yirl/widget.h"
#include "yirl/pos.h"
#include "yirl/rect.h"

typedef enum {
  YMAP_FULL = 0,
  YMAP_PARTIAL
} YMapRenderType;

#define	YMAP_SIZE_SPRITE_W  64
#define	YMAP_SIZE_SPRITE_H  64

typedef enum {
  YMAP_OUT_WARP,
  YMAP_OUT_NOTHING,
  YMAP_OUT_BLOCK
} YMapOutBehavior;

typedef struct {
  YWidgetState sate;
  Entity *resources;
} YMapState;

typedef enum {
  YMAP_DRAW_NO_DOUBLE = 1 << 0,
  YMAP_DRAW_COPY_ELEM = 1 << 1
} YMapDrawFlag;

/**
 * @return A new entity of type YARRAY, need to be free if father is NULL.
 */
Entity *ywMapCreateDefaultEntity(Entity *father, const char *name,
				 Entity *resources,
				 int baseId, uint32_t w, uint32_t h);

/**
 * create a map of @w * @y elemens
 * @return @entity
 */
Entity *ywMapInitEntity(Entity *entity, Entity *resources,
			int baseId, uint32_t w, uint32_t h);

int ywMapInit(void);
int ywMapEnd(void);
int ycursRegistreMap(void);
int ysdl2RegistreMap(void);

int ywMapType(Entity *map);

int ywMapHasChange(YWidgetState *state);

static inline int ywMapLen(Entity *state)
{
  Entity *tmpLen = yeGetByStrFast(state, "len");

  return  tmpLen ? (uint32_t)yeGetInt(tmpLen) :
    yeLen(yeGetByStrFast(state, "map"));
}

static inline int ywMapW(Entity *state)
{
  return yeGetInt(yeGetByStrFast(state, "width"));
}

static inline int ywMapH(Entity *state)
{
  return ywMapLen(state) / ywMapW(state);
}

static inline void ywMapSetW(Entity *state, int w)
{
  yeCreateInt(w, state, "width");
  if (yeGetInt(yeGet(state, "map-type")) == YMAP_FULL)
    ywRectReCreateInts(0, 0, w, ywMapH(state), state, "cam");
}

/**
 * @return true if @pos is inside the map
 */
static inline int ywMapIsInside(Entity *map, Entity *pos)
{
  return !(ywPosY(pos) < 0 || ywPosX(pos) < 0 ||
	   ywPosY(pos) >= ywMapH(map) || ywPosX(pos) >= ywMapW(map));
}

int ywMapGetIdByElem(Entity *mapElem);

/**
 * @return the elements array of @map at @pos
 */
Entity *ywMapGetCase(Entity *map, Entity *pos);

static inline Entity *ywMapGetEntityById(Entity *state, Entity *pos, int id)
{
  Entity *tmp = ywMapGetCase(state, pos);

  YE_ARRAY_FOREACH(tmp, caseTmp) {
    if (ywMapGetIdByElem(caseTmp) == id)
      return caseTmp;
  }
  return NULL;
}

Entity *ywMapPosFromInt(Entity *wid, int newPos,
			Entity *father, const char *name);
int ywMapIntFromPos(Entity *wid, Entity *pos);

Entity *ywMapPushElem(Entity *state, Entity *toPush,
		      Entity *pos, const char *name);

Entity *ywMapPushNbr(Entity *state, int toPush,
		     Entity *pos, const char *name);


/**
 * @smoot	true to activate smoort movement
 */
static inline void ywMapSetSmootMovement(Entity *map, int smoot)
{
  yeReCreateInt(smoot, map, "$smoot");
}

Entity *ywMapMvTablePush(Entity *map, Entity *from,
			 Entity *to, Entity *elem, Entity *callback);

void ywMapMvTableRemove(Entity *map, Entity *to_rm);

static inline void ywMapSetOutBehavior(Entity *map, YMapOutBehavior ob)
{
  Entity *ol = yeGetByStr(map, "$out_logic");

  if (!ol)
    yeCreateInt(ob, map, "$out_logic");
  else
    yeSetInt(ol, ob);
}

Entity *ywMapGetResourcesFromEntity(Entity *map);

static inline Entity *ywMapGetResources(YWidgetState *state)
{
  if (state)
    return ((YMapState *)state)->resources;
  return NULL;
}

static inline int ywMapRemoveByEntity(Entity *state, Entity *pos,
				      Entity *elem)
{
  Entity *cur = ywMapGetCase(state, pos);

  if (unlikely(!cur)) {
    return -1;
  }
  yeRemoveChild(cur, elem);
  return 0;
}

static inline int ywMapRemoveByStr(Entity *state, Entity *pos,
				    const char *str)
{
  Entity *cur = ywMapGetCase(state, pos);

  if (unlikely(!cur)) {
    return - 1;
  }
  yeRemoveChild(cur, yeGetByStrFast(cur, str));
  return 0;
}

#define ywMapRemove(sate, pos, elem)					\
  _Generic(elem,							\
	  Entity *: ywMapRemoveByEntity,				\
	  Y_GEN_CLANG_ARRAY(char, ywMapRemoveByStr),			\
	  const char *: ywMapRemoveByStr,				\
	  char *: ywMapRemoveByStr) (sate, pos, elem)


int ywMapSmootMove(Entity *state, Entity *from,
		   Entity *to, Entity *elem);

int ywMapMoveByStr(Entity *state, Entity *from,
		   Entity *to, const char *elem);

int ywMapMoveByEntity(Entity *state, Entity *from,
		      Entity *to, Entity *elem);

static inline Entity *ywMapCam(Entity *state)
{
  return yeGet(state, "cam");
}

static inline void ywMapSetCamPos(Entity *state, Entity *pos)
{
  Entity *cam = ywMapCam(state);

  ywRectSetPos(cam, pos);
  if (unlikely(ywRectX(cam) + ywRectW(cam) > ywMapW(state))) {
    ywPosSetX(cam, ywMapW(state) - ywRectW(cam));
  } else if (unlikely(ywRectX(cam) < 0)) {
    ywPosSetX(cam, 0);
  }
  if (unlikely(ywRectY(cam) + ywRectH(cam) > ywMapH(state))) {
    ywPosSetY(cam, ywMapH(state) - ywRectH(cam));
  } else if (unlikely(ywRectY(cam) < 0)) {
    ywPosSetY(cam, 0);
  }
}

/**
 * @map	The map to draw on
 * @posStart x, y pos where to start drawing
 * @size a pos use to specify width/height
 */
int ywMapDrawRect(Entity *map, Entity *rect, int id);

int ywMapDrawSegment(Entity *map, Entity *start, Entity *end, Entity *elem,
		     uint32_t flag);

/**
 * @brief add @x and @y to @pos then move @elem at @pos
 * @pos initial position, modified durring operation
 */
int ywMapAdvenceWithPos(Entity *map, Entity *pos,
			int x, int y, Entity *elem);

static inline int ywMapAdvenceWithEPos(Entity *map, Entity *pos,
				       Entity *other, Entity *elem)
{
  return ywMapAdvenceWithPos(map, pos, ywPosX(other), ywPosY(other), elem);
}


void ywMapGetSpriteSize(Entity *map, uint32_t *sizeSpriteW,
			uint32_t *sizeSpriteH, uint32_t *thresholdX);

void yeMapPixielsToPos(Entity *wid, uint32_t pixX, uint32_t pixY,
		       uint32_t *w, uint32_t *h);

Entity *ywMapPosFromPixs(Entity *wid, uint32_t x, uint32_t y,
			 Entity *father, const char *name);

int ywMapGetResourceId(Entity *map, Entity *elem);

static inline Entity *ywMapGetResource(Entity *map, Entity *elem)
{
  Entity *resources = ywMapGetResources(ywidGetState(map));

  if (!resources)
    resources = yeGet(map, "resources");
  return yeGet(resources, ywMapGetResourceId(map, elem));
}

#endif
