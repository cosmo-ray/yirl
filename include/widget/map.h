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

#include "widget.h"

typedef enum {
  YMAP_PARTIAL,
  YMAP_FULL
} YMapRenderType;

typedef struct {
  YWidgetState sate;
  Entity *resources;
  YMapRenderType renderType;
} YMapState;

/**
 * @return A new entity of type YARRAY, need to be free.
 */
Entity *ywMapCreateDefaultEntity(void);

int ywMapInit(void);
int ywMapEnd(void);
int ycursRegistreMap(void);
int ysdl2RegistreMap(void);

int ywMapHasChange(YWidgetState *state);

static inline int ywMapW(YWidgetState *state)
{
  return yeGetInt(yeGet(state->entity, "width"));
}

static inline int ywMapH(YWidgetState *state)
{
  return yeLen(yeGet(state->entity, "map")) / ywMapW(state);
}

int ywMapGetIdByElem(Entity *mapElem);

Entity *ywMapGetCase(YWidgetState *state, Entity *pos);

/**
 * @posX The position in X
 * @posY The position in Y
 * @father the father of the returned entity in which we store the return,
 * can be NULL.
 * @name string index at which we store the returned entity,
 * if NULL but not @father, the return is push back
 * @return an Entity that store a "Map Position", Must be free if @father is NULL
 */
Entity *ywMapCreatePos(int posX, int posY, Entity *father, const char *name);

static inline Entity *ywMapSetPos(Entity *pos, int posX, int posY)
{
  yeSetInt(yeGet(pos, 0), posX);
  yeSetInt(yeGet(pos, 1), posY);
  return pos;
}

static inline int ywMapPosIsSameEnt(Entity *pos1, Entity *pos2, int y)
{
  (void)y;
  return ((yeGetInt(yeGet(pos1, 0)) == yeGetInt(yeGet(pos2, 0))) &&
	  (yeGetInt(yeGet(pos1, 1)) == yeGetInt(yeGet(pos2, 1))));
}

static inline int ywMapPosIsSameInts(Entity *pos1, int x, int y)
{
  return ((yeGetInt(yeGet(pos1, 0)) == x) &&
	  (yeGetInt(yeGet(pos1, 1)) == y));
}

#define ywMapPosIsSame(pos, x, y)				\
  _Generic(x, Entity *: ywMapPosIsSameEnt,			\
	   void *: ywMapPosIsSameEnt,				\
	   const Entity *: ywMapPosIsSameEnt,				\
	   double : ywMapPosIsSameInts,				\
	   int : ywMapPosIsSameInts) (pos, x, y)


Entity *ywMapPosFromInt(YWidgetState *wid, int newPos,
			Entity *father, const char *name);

int ywMapPushElem(YWidgetState *state, Entity *toPush,
		  Entity *pos, const char *name);

static inline Entity *ywMapGetResources(YWidgetState *state)
{
  return ((YMapState *)state)->resources;
}

static inline void ywMapRemoveByEntity(YWidgetState *state, Entity *pos,
				       Entity *elem)
{
  Entity *cur = ywMapGetCase(state, pos);

  if (unlikely(!cur)) {
    return;
  }
  yeRemoveChild(cur, elem);
}

static inline void ywMapRemoveByStr(YWidgetState *state, Entity *pos,
				    const char *str)
{
  Entity *cur = ywMapGetCase(state, pos);

  if (unlikely(!cur)) {
    return;
  }
  yeRemoveChild(cur, yeGetByStrFast(cur, str));
}

#define ywMapRemove(sate, pos, elem)					\
  _Generic(elem,							\
	  Entity *: ywMapRemoveByEntity,				\
	  Y_GEN_CLANG_ARRAY(char, ywMapRemoveByStr),			\
	  const char *: ywMapRemoveByStr,				\
	  char *: ywMapRemoveByStr) (sate, pos, elem)


static inline int ywMapMoveByStr(YWidgetState *state, Entity *from,
				 Entity *to, const char *elem)
{
  Entity *cur = ywMapGetCase(state, from);
  Entity *tmp;

  if ((tmp = yeGetByStrFast(cur, elem)) == NULL)
    return -1;

  YE_INCR_REF(tmp);
  yeRemoveChild(cur, tmp);
  ywMapPushElem(state, tmp, to, elem);
  YE_DESTROY(tmp);
  return 0;
}

static inline int ywMapMoveByEntity(YWidgetState *state, Entity *from,
				    Entity *to, Entity *elem)
{
  YE_INCR_REF(elem);
  ywMapRemove(state, from, elem);
  ywMapPushElem(state, elem, to, NULL);
  YE_DESTROY(elem);
  return 0;
}

#endif
