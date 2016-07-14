/*
**Copyright (C) 2016 Matthias Gatto
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

#ifndef _YIRL_POS_H_
#define _YIRL_POS_H_

#include "entity.h"

/**
 * @posX The position in X
 * @posY The position in Y
 * @father the father of the returned entity in which we store the return,
 * can be NULL.
 * @name string index at which we store the returned entity,
 * if NULL but not @father, the return is push back
 * @return an Entity that store a "Map Position", Must be free if @father is NULL
 */
Entity *ywPosCreate(int posX, int posY, Entity *father, const char *name);

static inline Entity *ywPosSet(Entity *pos, int posX, int posY)
{
  yeSetInt(yeGet(pos, 0), posX);
  yeSetInt(yeGet(pos, 1), posY);
  return pos;
}

static inline int ywPosIsSameEnt(Entity *pos1, Entity *pos2, int y)
{
  (void)y;
  return ((yeGetInt(yeGet(pos1, 0)) == yeGetInt(yeGet(pos2, 0))) &&
	  (yeGetInt(yeGet(pos1, 1)) == yeGetInt(yeGet(pos2, 1))));
}

static inline int ywPosIsSameInts(Entity *pos1, int x, int y)
{
  return ((yeGetInt(yeGet(pos1, 0)) == x) &&
	  (yeGetInt(yeGet(pos1, 1)) == y));
}

#define ywPosIsSame(pos, x, y)			\
  _Generic(x, Entity *: ywPosIsSameEnt,		\
	   void *: ywPosIsSameEnt,			\
	   const Entity *: ywPosIsSameEnt,		\
	   double : ywPosIsSameInts,			\
	   int : ywPosIsSameInts) (pos, x, y)


static inline int ywPosIsSameXEnt(Entity *pos1, Entity *pos2)
{
  if (yeType(pos2) == YINT)
    return (yeGetInt(yeGet(pos1, 0)) == yeGetInt(pos2));
  return (yeGetInt(yeGet(pos1, 0)) == yeGetInt(yeGet(pos2, 0)));
}

static inline int ywPosIsSameXInt(Entity *pos1, int x)
{
  return (yeGetInt(yeGet(pos1, 0)) == x);
}

#define ywPosIsSameX(pos, x)			\
  _Generic(x, Entity *: ywPosIsSameXEnt,	\
	   void *: ywPosIsSameXEnt,		\
	   const Entity *: ywPosIsSameXEnt,	\
	   double : ywPosIsSameXInt,		\
	   int : ywPosIsSameXInt) (pos, x)

static inline int ywPosIsSameYEnt(Entity *pos1, Entity *pos2)
{
  if (yeType(pos2) == YINT)
    return (yeGetInt(yeGet(pos1, 1)) == yeGetInt(pos2));
  return (yeGetInt(yeGet(pos1, 1)) == yeGetInt(yeGet(pos2, 0)));
}

static inline int ywPosIsSameYInt(Entity *pos1, int x)
{
  return (yeGetInt(yeGet(pos1, 1)) == x);
}

#define ywPosIsSameY(pos, x)			\
  _Generic(x, Entity *: ywPosIsSameYEnt,	\
	   void *: ywPosIsSameYEnt,		\
	   const Entity *: ywPosIsSameYEnt,	\
	   double : ywPosIsSameYInt,		\
	   int : ywPosIsSameYInt) (pos, x)

static inline Entity *ywPosAdd(Entity *pos1, Entity *pos2)
{
  yeOpsAddEnt(yeGet(pos1, 0), yeGet(pos2, 0));
  yeOpsAddEnt(yeGet(pos1, 1), yeGet(pos2, 1));
  return pos1;
}

#endif
