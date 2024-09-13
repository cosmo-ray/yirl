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
#include "game.h"
#include "canvas.h"
#include "sdl2/canvas-sdl.h"
#include <SDL2/SDL.h>

int ywTextureFastMerge(Entity *src, Entity *srcRect,
		       Entity *dest, Entity *dstRect)
{
	return sdlMergeSurface(src, srcRect, dest, dstRect);
}

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

int ywTextureMergeUnsafe(Entity *src, Entity *srcRect,
		   Entity *dest, Entity *dstRect)
{
	return sdlMergeSurface(src, srcRect, dest, dstRect);
}

int	ywTextureNormalize(Entity *text)
{
	void *tmp = sdlCopySurface(yeGetDataAt(text, YCANVAS_SURFACE_IDX), NULL);
	Entity *data;

	if (!tmp)
		return -1;
	yeRemoveChild(text, YCANVAS_SURFACE_IDX);
	data = yeCreateDataAt(tmp, text, "$img-surface", YCANVAS_SURFACE_IDX);
	yeSetDestroy(data, sdlFreeSurface);
	return 0;
}

Entity *ywTextureNewImg(const char *path, Entity *size,
			Entity *father, const char *name)
{
	Entity * ret = yeCreateArray(father, name);

	yeCreateInt(YCanvasTexture, ret, "canvas-type");
	if (sdlCanvasCacheImg2(ret, NULL, path, size,
			       YSDL_CACHE_IMG_NO_TEXTURE) < 0)
		return NULL;
	return ret;
}

int ywTextureH(Entity *texture)
{
	SDL_Surface *dSurface = yeGetDataAt(texture, YCANVAS_SURFACE_IDX);

	return dSurface->h;
}

int ywTextureW(Entity *texture)
{
	SDL_Surface *dSurface = yeGetDataAt(texture, YCANVAS_SURFACE_IDX);

	return dSurface->w;
}

Entity *ywTextureNew(Entity *size, Entity *father, const char *name)
{
	Entity *obj = yeCreateArray(father, name);

	if (ywSizeH(size) == 0 || ywSizeW(size) == 0) {
		DPRINT_ERR("Broken size, %d %d is Invalide",
			   ywSizeW(size), ywSizeH(size));
		ygDgbAbort();
	}
	yeCreateInt(YCanvasTexture, obj, "canvas-type");
	sdlCanvasCacheVoidTexture(obj, size);
	return obj;
}

int ywTextureMergeText(Entity *texture, int x, int y, int w, int h,
		       const char * txt)
{
	return sdlMergeText(texture, x, y, w, h, txt);
}

int ywTextureMergeRectangle(Entity *texture, int x, int y,
			    int w, int h, const char * col)
{
	return sdlMergeRect(texture, x, y, w, h, col);
}

int ywTextureMergeTexture(Entity *texture, Entity *yTexture,
			  Entity *srcRect, Entity *dstRect)
{
	return sdlMergeSurface(texture, srcRect,
			       yTexture, dstRect);
}
