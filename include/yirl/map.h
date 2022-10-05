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

/*
 * Important entity options at map init are:
 * cam-type: if set to center it zoom the map using 'cam', otherwise, the whole map is printed
 * cam: camera, which is a rect, containing positiong and size of the cam (so [x,y,w,h])
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
  YMAP_DRAW_COPY_ELEM = 1 << 1,
  YMAP_DRAW_REPLACE_FIRST = 1 << 2
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
 * get the int id, from a case, as a case can contain multiple elems,
 * idx is use to get the elem position
 */
int ywMapIdAt(Entity *caseElems, int idx);

/**
 * @return the elements array of @map at @pos
 */
Entity *ywMapGetCase(Entity *map, Entity *pos);

static inline Entity *ywMapCase(Entity *map, Entity *pos)
{
	return ywMapGetCase(map, pos);
}

/**
 * @brief same as @ywMapGetCase but use x,y instead of pos
 */
Entity *ywMapCaseXY(Entity *map, int x, int y);

/**
 * return the case under the cam pointer.
 * useful only if you have a "cam_pointer"
 */
static inline Entity *ywMapCamPointedCase(Entity *map)
{
	Entity *cam = yeGet(map, "cam");
	int32_t begX = ywRectX(cam);
	int32_t begY = ywRectY(cam);

	return ywMapCaseXY(map, begX, begY);
}

static inline _Bool ywMapCamPointedContainId(Entity *map, int id) {
	Entity *tmp = ywMapCamPointedCase(map);

	YE_FOREACH(tmp, caseTmp) {
		if (ywMapGetIdByElem(caseTmp) == id)
			return 1;
	}
	return 0;
}

#define ywMapContainEnt(m,x,y,e) yeDoesInclude(ywMapCaseXY(m, x, y), e)
#define ywMapContainStr(m,x,y,s) yeArrayContainEntity(ywMapCaseXY(m, x, y), s)

static inline Entity *ywMapGetEntityById(Entity *state, Entity *pos, int id)
{
	Entity *tmp = ywMapGetCase(state, pos);

	YE_FOREACH(tmp, caseTmp) {
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

Entity *ywMapPushElemAt(Entity *state, Entity *toPush,
			Entity *pos, const char *name, int at);

Entity *ywMapPushNbr(Entity *state, int toPush,
		     Entity *pos, const char *name);

static inline Entity *ywMapTryPushElem(Entity *state, Entity *toPush,
				       Entity *pos, const char *name)
{
	Entity *map_case = ywMapGetCase(state, pos);

	if ((name && yeGet(map_case, name)) || yeDoesInclude(map_case, toPush))
		return NULL;
	return ywMapPushElem(state, toPush, pos, name);
}

Entity *ywMapPushNbrXY(Entity *state, int toPush,
		       int x, int y, const char *name);

/**
 * @smoot	true to activate smoort movement
 */
void ywMapSetSmootMovement(Entity *map, int smoot);

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

static inline int ywMapPopXY(Entity *map, int x, int y)
{
	Entity *cur = ywMapCaseXY(map, x, y);

	if (unlikely(!cur))
		return -1;

	yePopBack(cur);
	return 0;
}

static inline int ywMapPop(Entity *map, Entity *pos)
{
	Entity *cur = ywMapCase(map, pos);

	if (unlikely(!cur))
		return -1;

	yePopBack(cur);
	return 0;
}

static inline int ywMapRemoveByEntity(Entity *state, Entity *pos,
				      Entity *elem);

static inline int ywMapRemoveByInt(Entity *state, Entity *pos, int id)
{
	Entity *cur = ywMapGetCase(state, pos);

	if (unlikely(!cur))
		return -1;
	YE_FOREACH(cur, el) {
		if (ywMapGetIdByElem(el) == id)
			return ywMapRemoveByEntity(state, pos, el);
	}
	return -1;
}

static inline int ywMapRemoveByEntity(Entity *state, Entity *pos,
				      Entity *elem)
{
	Entity *cur = ywMapGetCase(state, pos);

	if (unlikely(!cur))
		return -1;
	if (!yeRemoveChild(cur, elem) && yeType(elem) == YINT)
		return ywMapRemoveByInt(state, pos, yeGetInt(elem));
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

#define ywMapRemove(sate, pos, elem)				\
	_Generic(elem,						\
		 Entity *: ywMapRemoveByEntity,			\
		 Y_GEN_CLANG_ARRAY(char, ywMapRemoveByStr),	\
		 const char *: ywMapRemoveByStr,		\
		 int : ywMapRemoveByInt,			\
		 char *: ywMapRemoveByStr) (sate, pos, elem)

static inline void ywMapCLearArrayPos(Entity *map, Entity *pos_array,
				      const char *str)
{
	YE_FOREACH(pos_array, p) {
		ywMapRemove(map, p, str);
	}
	yeClearArray(pos_array);
}

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
	Entity *cam_threshold = yeGet(state, "cam-threshold");
	int tx = ywPosX(cam_threshold);
	int ty = ywPosY(cam_threshold);

	ywRectSetPos(cam, pos);
	if (unlikely(ywRectX(cam) + ywRectW(cam) + tx > ywMapW(state))) {
		ywPosSetX(cam, ywMapW(state) - ywRectW(cam) - tx);
	} else if (unlikely(ywRectX(cam) < 0)) {
		ywPosSetX(cam, 0);
	}
	if (unlikely(ywRectY(cam) + ywRectH(cam) + ty > ywMapH(state))) {
		ywPosSetY(cam, ywMapH(state) - ywRectH(cam) - ty);
	} else if (unlikely(ywRectY(cam) < 0)) {
		ywPosSetY(cam, 0);
	}
}

static inline void ywMapCamAddX(Entity *state, int x)
{
	Entity *c = ywMapCam(state);
	yeAutoFree Entity *nc = ywPosCreate(c, 0, NULL, NULL);

	ywPosAddXY(nc, x, 0);
	ywMapSetCamPos(state, nc);
}

static inline void ywMapCamAddY(Entity *state, int y)
{
	Entity *c = ywMapCam(state);
	yeAutoFree Entity *nc = ywPosCreate(c, 0, NULL, NULL);

	ywPosAddXY(nc, 0, y);
	ywMapSetCamPos(state, nc);
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

static inline void ywMapResourcePushElem(
	Entity *resource,
	const char *character,
	const char *sprite_path,
	const char *elem_name)
{
	Entity *el = yeCreateArray(resource, elem_name);

	if (character)
		yeCreateString(character, el, "map-char");
	if (sprite_path)
		yeCreateString(sprite_path, el, "map-sprite");
	yeCreateString(elem_name, el, "name");
}


#endif
