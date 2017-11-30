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

static inline Entity *ywRectCreateEnt(Entity *other, Entity *father,
				      const char *name)
{
  return ywRectCreateInts(ywRectX(other), ywRectY(other), ywRectW(other),
			  ywRectH(other), father, name);
}

static inline Entity *ywRectReCreateEnt(Entity *other, Entity *father,
					const char *name)
{
  return ywRectReCreateInts(ywRectX(other), ywRectY(other), ywRectW(other),
			    ywRectH(other), father, name);
}

static inline Entity *ywRectCreatePosSize(Entity *pos, Entity *size,
					  Entity *father, const char *name)
{
  int x = 0, y = 0;

  if (pos) {
    x = ywPosX(pos);
    y = ywPosY(pos);
  }

  return ywRectReCreateInts(x, y, ywSizeW(size),
			    ywSizeH(size), father, name);
}


void ywRectSet(Entity *rect, int x, int y, int w, int h);

static inline void ywRectSetFromRect(Entity *rect, Entity *o)
{
  ywRectSet(rect, ywRectX(o), ywRectY(o), ywRectW(o), ywRectH(o));
}

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

static inline Entity *ywRectSetPos(Entity *rect, Entity *pos)
{
  ywRectSetX(rect, ywPosX(pos));
  ywRectSetY(rect, ywPosY(pos));
  return rect;
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
 * @return true if the point at @posx, @posy is inside @rect if @proper is true
 * doen't return true when on the edge
 */
static inline int ywRectContain(Entity *rect, int posx, int posy, int proper)
{
  return posx > ywRectX(rect) - !(proper) &&
    posx < ywRectX(rect) + ywRectW(rect) - !!(proper) &&
    posy > ywRectY(rect) - !(proper) &&
    posy < ywRectY(rect) + ywRectH(rect) - !!(proper);
}

static inline int ywRectContainPos(Entity *rect, Entity *pos, int proper)
{
  return ywRectContain(rect, ywPosX(pos), ywPosY(pos), proper);
}

static inline int ywRectColision(Entity *rect0, Entity *rect1)
{
  int bx0 = ywRectX(rect0), ex0 = ywRectX(rect0) + ywRectW(rect0);
  int by0 = ywRectY(rect0), ey0 = ywRectY(rect0) + ywRectH(rect0);
  int bx1 = ywRectX(rect1), ex1 = ywRectX(rect1) + ywRectW(rect1);
  int by1 = ywRectY(rect1), ey1 = ywRectY(rect1) + ywRectH(rect1);

  if (bx0 > ex1 || bx1 > ex0)
    return 0;

  if (by0 > ey1 || by1 > ey0)
    return 0;

  return 1;
}

static inline char * ywRectToString(Entity *r)
{
  static char tmp[4][256];
  static int i;

  ++i;
  i &= 3;
  snprintf(tmp[i], 256, "x: %d - y: %d w: %d h: %d",
	   yeGetInt(yeGetByIdx(r, 0)), yeGetInt(yeGetByIdx(r, 1)),
	   ywRectW(r), ywRectH(r));
  return tmp[i];
}

static inline void ywRectPrint(Entity *r)
{
  printf("x: %d - y: %d w: %d h: %d\n",
	 ywRectX(r), ywRectY(r), ywRectW(r), ywRectH(r));
}

#endif
