
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

#include "rect.h"

Entity *ywRectReCreateInts(int x, int y, int w, int h,
			   Entity *father, const char *name)
{
  Entity *ret = yeReCreateArray(father, name, NULL);
  yeCreateInt(x, ret, "x");
  yeCreateInt(y, ret, "y");
  yeCreateInt(w, ret, "w");
  yeCreateInt(h, ret, "h");
  return ret;
}

Entity *ywRectCreateInts(int x, int y, int w, int h, Entity *father,
			 const char *name)
{
  Entity *ret = yeCreateArray(father, name);
  yeCreateInt(x, ret, "x");
  yeCreateInt(y, ret, "y");
  yeCreateInt(w, ret, "w");
  yeCreateInt(h, ret, "h");
  return ret;
}

void ywRectSet(Entity *rect, int x, int y, int w, int h)
{
  ywRectSetX(rect, x);
  ywRectSetY(rect, y);
  ywRectSetW(rect, w);
  ywRectSetH(rect, h);
}

Entity *ywRectColisionRect(Entity *rect0, Entity *rect1,
			   Entity *father, const char *name)
{
  int bx0 = ywRectX(rect0), ex0 = ywRectX(rect0) + ywRectW(rect0);
  int by0 = ywRectY(rect0), ey0 = ywRectY(rect0) + ywRectH(rect0);
  int bx1 = ywRectX(rect1), ex1 = ywRectX(rect1) + ywRectW(rect1);
  int by1 = ywRectY(rect1), ey1 = ywRectY(rect1) + ywRectH(rect1);
  int ret0x, ret0y, retw, reth;
  int ret1x, ret1y;
  Entity *ret;

  if (bx0 > ex1 || bx1 > ex0)
    return NULL;

  if (by0 > ey1 || by1 > ey0)
    return NULL;

  if (bx0 < bx1 && ex0 > ex1) {
    /* [ {---} ] */
    ret0x = bx1 - bx0;
    ret1x = 0;
    retw = ex1 - bx1;;
  } else if (bx0 < bx1 && ex0 < ex1) {
    /* [ {---] } */
    ret0x = bx1 - bx0;
    ret1x = 0;
    retw = ex0 - bx1;;
  } else if (bx0 > bx1 && ex0 > ex1) {
    /* { [---} ] */
    ret0x = 0;
    ret1x = bx0 - bx1;
    retw = ex1 - bx0;;
  } else {
    /* { [---] } */
    ret0x = 0;
    ret1x = bx0 - bx1;
    retw = ex0 - bx0;;
  }

  if (by0 < by1 && ey0 > ey1) {
    /* [ {---} ] */
    ret0y = by1 - by0;
    ret1y = 0;
    reth = ey1 - by1;;
  } else if (by0 < by1 && ey0 < ey1) {
    /* [ {---] } */
    ret0y = by1 - by0;
    ret1y = 0;
    reth = ey0 - by1;;
  } else if (by0 > by1 && ey0 > ey1) {
    /* { [---} ] */
    ret0y = 0;
    ret1y = by0 - by1;
    reth = ey1 - by0;;
  } else {
    /* { [---] } */
    ret0y = 0;
    ret1y = by0 - by1;
    reth = ey0 - by0;;
  }

  ret = yeCreateArray(father, name);
  ywRectCreateInts(ret0x, ret0y, retw, reth, ret, NULL);
  ywRectCreateInts(ret1x, ret1y, retw, reth, ret, NULL);
  return ret;
}
