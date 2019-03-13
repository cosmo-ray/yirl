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

#include "sdl-driver.h"
#include "widget.h"

typedef struct
{
  int           fullscreen;
  SDL_Window	*pWindow;
  TTF_Font	*font;
  unsigned int  fontSize;
  unsigned int  txtHeight;
  unsigned int  txtWidth;
  SDL_Renderer	*renderer;
} SDL_Global;

typedef enum  {
  YSDL_ALIGN_CENTER,
  YSDL_ALIGN_LEFT
} YDsdlAlignementText;

typedef struct
{
  YWidgetState *wid;
  SDL_Rect      rect;
} SDLWid;

void sdlResize(YWidgetState *wid, int renderType);

void sdlWidInit(YWidgetState *wid, int t);

/**
 * like init but does more stuff
 */
static int sdlWidInit2(YWidgetState *wid, int t)
{
	wid->renderStates[t].opac = malloc(sizeof(SDLWid));
	sdlWidInit(wid, t);
	return 0;
}

int sdlFillColorBg(SDLWid *swid, short r, short g, short b, short a);

int sdlFillImgBg(SDLWid *swid, const char *cimg);

int sdlFillBg(SDLWid *swid, YBgConf *cfg);

void sdlDrawRect(SDLWid *swid, SDL_Rect rect, SDL_Color color);

void sdlWidDestroy(YWidgetState *wid, int t);

int sgGetFontSize(void);

uint32_t sgGetTxtW(void);
uint32_t sgGetTxtH(void);

SDL_Rect  getRect(void);
SDL_Surface *wSurface(void);
SDL_Renderer *sgRenderer(void);
TTF_Font *sgDefaultFont(void);
int sgSetDefaultFont(const char *path);

int sdlPrintText(SDLWid *wid,
		 const char *str,
		 SDL_Color color,
		 SDL_Rect pos,
		 int alignementType);

int sdlCanvasCacheTexture(Entity *state, Entity *elem);

int sdlCanvasRendObj(YWidgetState *state, SDLWid *wid, Entity *img,
		     Entity *cam, Entity *widPix);

int sdlDisplaySprites(YWidgetState *state, SDLWid *wid,
		      int x, int y, Entity *mapElem,
		      int w, int h, int thresholdX,
		      int thresholdY, Entity *mod);

SDL_Rect sdlRectFromRectEntity(Entity *rect);

void sdlConsumeError(void);

SDL_Surface *sdlCopySurface(SDL_Surface *surface, Entity *rEnt);

#endif
