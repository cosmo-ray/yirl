/*
**Copyright (C) 2017 Matthias Gatto
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

#ifndef _SDL2_CANVAS_SDL_H_
#define _SDL2_CANVAS_SDL_H_

typedef enum {
  YSDL_CACHE_IMG_NO_TEXTURE = 1
} ysdlImgCacheFlag;

struct SDL_Surface;
typedef struct SDL_Surface SDL_Surface;

void sdlFreeSurface(void *surface);
void sdlCanvasCacheVoidTexture(Entity *obj, Entity *size);
int sdlCanvasCacheTexture(Entity *state, Entity *elem);
uint32_t sdlCanvasPixInfo(Entity *obj, int x, int y);
SDL_Surface *sdlCopySurface(SDL_Surface *surface, Entity *rEnt);
int sdlCanvasCacheImg(Entity *elem, Entity *resource, const char *imgPath,
		      Entity *rEnt);
int sdlCanvasCacheImg2(Entity *elem, Entity *resource, const char *imgPath,
		       Entity *rEnt, int32_t flag);
int sdlCanvasCacheImg3(Entity *elem, Entity *resource, const char *imgPath,
		       Entity *rEnt, int32_t flag, Entity *img_dst_rect);
int sdlMergeSurface(Entity *textSrc, Entity *srcRect,
		    Entity *textDest, Entity *destRect);
void sdlCanvasCacheBicolorImg(Entity *elem, const uint8_t *img, Entity *info);
void sdlCanvasCacheHeadacheImg(Entity *elem, Entity *map, Entity *info);

void sdlCanvasDrawableSetPix(Entity *elem, int x, int y, int color);
void sdlCanvasDrawableFinalyze(Entity *elem);
void sdlCanvasDrawableNew(Entity *elem, Entity *size);

int sdlMergeRect(Entity *dst, int x, int y, int w, int h, const char *c);
int sdlMergeText(Entity *dst, int x, int y, int w, int h, const char *str);

int sgGetFontSize(void);
uint32_t sgGetTxtH(void);
uint32_t sgGetTxtW(void);

#endif
