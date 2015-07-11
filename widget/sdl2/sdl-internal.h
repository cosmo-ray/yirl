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

#include "widget.h"

#define WIN_W_SIZE 640
#define WIN_H_SIZE 480

typedef struct
{
  int           fullscreen;
  SDL_Window*   pWindow;
  TTF_Font*     font;
  SDL_Renderer* renderer;
} SDL_Global;

typedef struct
{
  YWidgetState *wid;
  SDL_Rect      rect;
} SDLWid;

void resize(YWidgetState *wid, int renderType);
void resize(YWidgetState *wid, int renderType)
{
  SDLWid *swid = wid->renderStates[renderType].opac;

  swid->rect.h = wid->pos.h * WIN_H_SIZE / 1000;
  swid->rect.w = wid->pos.w * WIN_W_SIZE / 1000;
  swid->rect.x = wid->pos.x * WIN_W_SIZE / 1000;
  swid->rect.y = wid->pos.y * WIN_H_SIZE / 1000;
}

void sdlWidInit(YWidgetState *wid, int t);
void sdlWidInit(YWidgetState *wid, int t)
{
  resize(wid, t);
}


SDL_Rect  getRect(void);
SDL_Surface *wSurface(void);
void ysdl2Destroy(void);
int ysdl2Type(void);
int ysdl2Init(void);

#endif
