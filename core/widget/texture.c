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
#include "canvas.h"
#include "sdl2/canvas-sdl.h"

/**
 * merge dest into src, srcRect and dstRect are curently unused
 */
int ywTextureMerge(Entity *src, Entity *srcRect,
		   Entity *dest, Entity *dstRect)
{
  if (unlikely(!src || !dest))
    return -1;
  if (ywCanvasObjType(src) != YCanvasRect)
	  ywTextureNormalize(src);
  if (ywCanvasObjType(dest) != YCanvasRect)
	  ywTextureNormalize(dest);
  return sdlMergeSurface(src, srcRect, dest, dstRect);
}

int	ywTextureNormalize(Entity *text)
{
  void *tmp = sdlCopySurface(yeGetDataAt(text, "$img-surface"), NULL);
  Entity *data;

  if (!tmp)
    return -1;
  yeRemoveChild(text, "$img-surface");
  data = yeCreateData(tmp, text, "$img-surface");
  yeSetDestroy(data, sdlFreeSurface);
  return 0;
}

Entity *ywTextureNewImg(const char *path, Entity *size,
			Entity *father, const char *name)
{
  Entity * ret = yeCreateArray(father, name);

  if (sdlCanvasCacheImg2(ret, NULL, path, size,
			 YSDL_CACHE_IMG_NO_TEXTURE) < 0)
    return NULL;
  return ret;
}
