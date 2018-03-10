/*
**Copyright (C) 2018 Matthias Gatto
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

#include "texture.h"
#include "sdl2/canvas-sdl.h"

Entity *ywTextureNewImg(const char *path, Entity *size,
			Entity *father, Entity *name)
{
  Entity * ret = yeCreateArray(father, name);
  yePushBack(ret, size, "img-src-rect");

  if (sdlCanvasCacheImg(ret, NULL, path) < 0)
    return NULL;
  yeRemoveChild(ret, "img-src-rect");
  return ret;
}
