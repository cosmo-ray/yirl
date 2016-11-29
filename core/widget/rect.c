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
