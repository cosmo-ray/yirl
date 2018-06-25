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

#include "pos.h"
#include "math.h"

Entity *ywPosCreateInts(int posX, int posY, Entity *father, const char *str)
{
  Entity *ret = yeCreateArray(father, str);

  yeCreateInt(posX, ret, "x");
  yeCreateInt(posY, ret, "y");
  return ret;
}

Entity *ywPosCreateAt(int posX, int posY, Entity *father, const char *str, int idx)
{
  Entity *ret = yeCreateArrayAt(father, str, idx);

  yeCreateInt(posX, ret, "x");
  yeCreateInt(posY, ret, "y");
  return ret;
}

double ywPosAngle(Entity *p0, Entity *p1)
{
  return atan2(ywPosY(p0) - ywPosY(p1),
	       ywPosX(p0) - ywPosX(p1)) * 180 / M_PI;
}

#endif  /* _YIRL_RECT_H_ */
