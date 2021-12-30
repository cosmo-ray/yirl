/*
**Copyright (C) 2015 Matthias Gatto
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

#ifndef _SDL_INTERNAL_H_
#define _SDL_INTERNAL_H_

#include <SDL_gpu.h>
#include "sdl-driver.h"
#include "widget.h"

typedef struct
{
  int           fullscreen;
  GPU_Target	*pWindow;
  TTF_Font	*font;
  unsigned int  fontSize;
  unsigned int  txtHeight;
  unsigned int  txtWidth;
} SDL_Global;

typedef enum  {
  YSDL_ALIGN_CENTER,
  YSDL_ALIGN_LEFT
} YDsdlAlignementText;

typedef struct
{
  YWidgetState *wid;
  GPU_Rect      rect;
} SDLWid;

/**
 * @return the SDLWid inside the magin
 */
SDLWid *sddComputeMargin(YWidgetState *w, SDLWid *swid);

void sdlResize(YWidgetState *wid, int renderType);

void sdlWidInit(YWidgetState *wid, int t);

int sdlFillColorBg(SDLWid *swid, short r, short g, short b, short a);

int sdlFillBg(SDLWid *swid, YBgConf *cfg);

void sdlDrawRect(SDLWid *swid, GPU_Rect rect, SDL_Color color);

void sdlWidDestroy(YWidgetState *wid, int t);

int sgGetFontSize(void);

uint32_t sgGetTxtW(void);
uint32_t sgGetTxtH(void);

GPU_Rect  getRect(void);
TTF_Font *sgDefaultFont(void);
int sgSetDefaultFont(const char *path);

int sdlPrintText(SDLWid *wid,
		 const char *str,
		 SDL_Color color,
		 GPU_Rect pos,
		 int alignementType);

int sdlPrintTextExt(SDLWid *wid, const char *str, SDL_Color color,
		    GPU_Rect pos, int alignementType, int lineSpace);

int sdlCanvasCacheImg3(Entity *elem, Entity *resource, const char *imgPath,
		       Entity *rEnt, int32_t flag, Entity *img_dst_rect);

int sdlCanvasCacheTexture(Entity *state, Entity *elem);

void sdlCanvasCacheVoidTexture(Entity *obj, Entity *size);

int sdlCanvasRendObj(YWidgetState *state, SDLWid *wid, Entity *img,
		     Entity *cam, Entity *widPix);

int sdlDisplaySprites(YWidgetState *state, SDLWid *wid,
		      int x, int y, Entity *mapElem,
		      int w, int h, int thresholdX,
		      int thresholdY, Entity *mod);

static inline GPU_Rect sdlRectFromRectEntity(Entity *rect)
{
	GPU_Rect ret = {ywRectX(rect), ywRectY(rect),
			ywRectW(rect), ywRectH(rect)};

	return ret;
}

void sdlConsumeError(void);

SDL_Surface *sdlCopySurface(SDL_Surface *surface, Entity *rEnt);

#endif
