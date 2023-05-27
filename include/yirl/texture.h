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

#ifndef	YIRL_TEXTURE_H_
#define	YIRL_TEXTURE_H_

#include "yirl/entity.h"

Entity *ywTextureNewImg(const char *path, Entity *srcRect,
			Entity *father, const char *name);

int ywTextureFastMerge(Entity *src, Entity *srcRect,
		       Entity *dest, Entity *dstRect);

/**
 * merge @src on @dest
 * @return -1 on error, 0 on sucess
 */
int ywTextureMerge(Entity *src, Entity *srcRect,
		   Entity *dest, Entity *dstRect);

/**
 * Like ywTextureMerge, but don't check NULL
 * and don't check if src and dst jave the same format, which if not,
 * can silently fail
 * you use ywTextureNormalize if you want to have yirl 'standard' texture
 */
int ywTextureMergeUnsafe(Entity *src, Entity *srcRect,
		   Entity *dest, Entity *dstRect);

int ywTextureNormalize(Entity *text);

Entity *ywTextureNew(Entity *size, Entity *father, const char *name);

int ywTextureMergeText(Entity *texture, int x, int y, int w, int h,
		       const char * txt);

int ywTextureMergeRectangle(Entity *texture, int x, int y,
			    int w, int h, const char * col);

int ywTextureMergeTexture(Entity *src, Entity *dst,
			  Entity *srcRect, Entity *dstRect);

#endif
