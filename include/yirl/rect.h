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

#ifndef _YIRL_RECT_H_
#define _YIRL_RECT_H_

#include "yirl/pos.h"

/**
 * @father the father of the returned entity in which we store the return,
 * can be NULL.
 * @name string index at which we store the returned entity,
 * if NULL but not @father, the return is push back
 * @return an Entity that store a "Map Position", Must be free if @father is NULL
 */
Entity *ywRectCreateInts(int x, int y, int w, int h, Entity *father,
			 const char *name);

Entity *ywRectReCreateInts(int x, int y, int w, int h,
			   Entity *father, const char *name);

static inline int ywRectX(Entity *e)
{
  return yeGetInt(yeGetByIdx(e, 0));
}

static inline int ywRectY(Entity *e)
{
  return yeGetInt(yeGetByIdx(e, 1));
}

static inline int ywRectW(Entity *e)
{
  return yeGetInt(yeGetByIdx(e, 2));
}

static inline int ywRectH(Entity *e)
{
  return yeGetInt(yeGetByIdx(e, 3));
}

void ywRectSet(Entity *rect, int x, int y, int w, int h);

static inline Entity *ywRectSetX(Entity *pos, int posX)
{
  yeSetInt(yeGetByIdx(pos, 0), posX);
  return pos;
}

static inline Entity *ywRectSetY(Entity *pos, int posY)
{
  yeSetInt(yeGetByIdx(pos, 1), posY);
  return pos;
}

static inline Entity *ywRectSetW(Entity *pos, int posW)
{
  yeSetInt(yeGetByIdx(pos, 2), posW);
  return pos;
}

static inline Entity *ywRectSetH(Entity *pos, int posH)
{
  yeSetInt(yeGetByIdx(pos, 3), posH);
  return pos;
}

/**
 * @return true if the point at @posx, @posy is inside @rect
 */
static inline int ywRectIntersect(Entity *rect, int posx, int posy)
{
  return posx > ywRectX(rect) && posx < (ywRectX(rect) + ywRectW(rect)) &&
    posy > ywRectY(rect) && posy < (ywRectY(rect) + ywRectH(rect));
}

static inline int ywRectIntersectPos(Entity *rect, Entity *pos)
{
  return ywRectIntersect(rect, ywPosX(pos), ywPosY(pos));
}

#endif
