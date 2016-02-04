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

typedef struct {
  YWidgetState sate;
  Entity *resources;
  Entity *pos;
  int actionIdx;
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
Entity *ywMapGetPos(YWidgetState *state);
Entity *ywMapGetCurrentCase(YWidgetState *state);

Entity *ywMapCreatePos(int posX, int posY, Entity *father, const char *str);

int ywMapPushElem(YWidgetState *state, Entity *toPush,
		  Entity *pos, const char *name);

static inline Entity *ywMapGetResources(YWidgetState *state)
{
  return ((YMapState *)state)->resources;
}

static inline void ywMapRemove(YWidgetState *state, Entity *pos, Entity *elem)
{
  Entity *cur = ywMapGetCase(state, pos);

  yeRemoveChild(cur, elem);
}

#endif
