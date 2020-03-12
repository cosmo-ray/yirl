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

Entity *ywTextureNewImg(const char *path, Entity *size,
			Entity *father, const char *name);

int ywTextureFastMerge(Entity *src, Entity *srcRect,
		       Entity *dest, Entity *dstRect);

/**
 * merge @src on @dest
 * @return -1 on error, 0 on sucess
 */
int ywTextureMerge(Entity *src, Entity *srcRect,
		   Entity *dest, Entity *dstRect);

int	ywTextureNormalize(Entity *text);

#endif
