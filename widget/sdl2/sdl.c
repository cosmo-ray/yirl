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

#include <glib.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "sdl-internal.h"
#include "utils.h"
#include "widget.h"

static int type = -1;

static SDL_Global sg;

SDL_Rect      getRect(void)
{
  return (wSurface()->clip_rect);
}

SDL_Renderer *sgRenderer(void)
{
  return sg.renderer;
}

TTF_Font *sgDefaultFont(void)
{
  return sg.font;
}

int sgSetDefaultFont(const char *path)
{
  TTF_Font *font = TTF_OpenFont(path, 16);
  sg.fontSize = 16;
  sg.font = font;
  if (!font)
    return -1;
  return -0;
}


int sgGetFontSize(void)
{
  return sg.fontSize;
}

SDL_Surface *wSurface(void)
{
  return (SDL_GetWindowSurface(sg.pWindow));
}

void	sdlDrawRect(SDLWid *swid, SDL_Rect rect, SDL_Color color)
{
  SDL_Surface *textSurface =  SDL_CreateRGBSurface(0, rect.w,
						   rect.h,
						   32, 0, 0, 0, 0);
  SDL_FillRect(textSurface, NULL, SDL_MapRGBA(textSurface->format,
					      color.r, color.g, color.b, color.a));
  SDL_Texture* text = SDL_CreateTextureFromSurface(sg.renderer, textSurface);
  (void)swid;
  SDL_RenderCopy(sg.renderer, text, NULL, &rect);
  SDL_RenderPresent(sg.renderer);
  SDL_DestroyTexture(text);
  SDL_FreeSurface(textSurface);
}

void    sdlFillColorBg(SDLWid *swid, short r, short g, short b, short a)
{
 SDL_Surface *textSurface =  SDL_CreateRGBSurface(0, swid->rect.w,
						   swid->rect.h,
						   32, 0, 0, 0, 0);
  SDL_FillRect(textSurface, NULL, SDL_MapRGBA(textSurface->format, r, g, b, a));
  SDL_Texture* text = SDL_CreateTextureFromSurface(sg.renderer, textSurface);
  SDL_RenderCopy(sg.renderer, text, NULL, &swid->rect);
  SDL_RenderPresent(sg.renderer);
  SDL_DestroyTexture(text);
  SDL_FreeSurface(textSurface);
}

int    sdlFillImgBg(SDLWid *swid, const char *cimg)
{
  if (cimg) {
    SDL_Surface *img = IMG_Load(cimg);
    if (!img)
      return -1;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(sg.renderer, img);
    SDL_RenderCopy(sg.renderer, texture, NULL, &swid->rect);
    SDL_DestroyTexture(texture);
    return 0;
  }
  return -1;
}


void    ysdl2Destroy(void)
{
  if (type == -1)
    return;
  SDL_DestroyWindow(sg.pWindow);
  IMG_Quit();
  TTF_Quit();
  SDL_Quit();
  ywidRemoveRender(type);
  type = -1;
}

int ysdl2Type(void)
{
  return type;
}

static int  convertToYKEY(SDL_Keycode key)
{
  switch (key)
    {
    case SDLK_UP:
      return (Y_UP_KEY);
    case SDLK_DOWN:
      return (Y_DOWN_KEY);
    case SDLK_LEFT:
      return (Y_LEFT_KEY);
    case SDLK_RIGHT:
      return (Y_RIGHT_KEY);
    case SDLK_RETURN:
      return ('\n');
    case SDLK_TAB:
      return ('\t');
    default:
      return (-1);
    }
}

static inline YEvent *SDLConvertEvent(SDL_Event* event)
{
  YEvent *eve = g_new(YEvent, 1);
  
  switch(event->type)
    {
    case SDL_KEYUP:
    case SDL_KEYDOWN:
      eve->type = YKEY_UP;
      break;
      eve->type = YKEY_DOWN;
      break;
    default:
      return NULL;
    }
  eve->key = convertToYKEY(event->key.keysym.sym);
  return eve;
}

static YEvent *SDLWaitEvent(void)
{
  SDL_Event event;

  SDL_WaitEvent(&event);
  return SDLConvertEvent(&event);
}

static YEvent *SDLPollEvent(void)
{
  SDL_Event event;

  SDL_PollEvent(&event);
  return SDLConvertEvent(&event);
}

int    ysdl2Init(void)
{
  if (type != -1)
    return type;

  /* Initialisation simple */
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0 ) {
    DPRINT_ERR("Échec de l'initialisation de la SDL (%s)\n",SDL_GetError());
    return -1;
  }

  if(TTF_Init()==-1) {
    DPRINT_ERR("TTF_Init: %s\n", TTF_GetError());
    goto ttf_fail;
  }


  // initializing Flags for PNG Images
  int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF;

  // Simple check of the Flags
  if(!(IMG_Init(imgFlags)&imgFlags)) {
    DPRINT_ERR("SDL_image could not initialize! SDL_image Error: %s\n",
	       IMG_GetError());
    goto img_fail;
  }

  /* Création de la fenêtre */
  sg.pWindow = SDL_CreateWindow("YIRL isn't a rogue like", SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				WIN_W_SIZE,
				WIN_H_SIZE,
				SDL_WINDOW_SHOWN);
  if(!sg.pWindow) {
      DPRINT_ERR("Error to creeate window:: %s\n",SDL_GetError());
      goto win_fail;
  }
  
  // Render for the main windows
  sg.renderer = SDL_CreateRenderer(sg.pWindow, -1, SDL_RENDERER_TARGETTEXTURE);
  if (!sg.renderer) {
    DPRINT_ERR("Get render from window: %s\n", TTF_GetError());
    goto fail;
  }

  if (sgSetDefaultFont("./sample.ttf") < 0)
    DPRINT_WARN("can not load \"./sample.ttf\"\n");

  
  // fill the window with a black rectangle
  // SDL_Rect   rect = sg.getRect();

  SDL_RenderClear(sg.renderer);
  SDL_RenderPresent(sg.renderer);
  type = ywidRegistreRender(sdlResize, SDLPollEvent, SDLWaitEvent);
  return type;

 fail:
  SDL_DestroyWindow(sg.pWindow);
 win_fail:
  TTF_Quit();
 img_fail:
  IMG_Quit();
 ttf_fail:
  SDL_Quit();
  return -1;
}

int sdlPrintText(SDLWid *wid,
		 const char *str,
		 unsigned int caract_per_line,
		 SDL_Color color,
		 int x, int y)
{
  unsigned int   len = strlen(str);
  int text_width , text_height;
  SDL_Renderer *renderer = sg.renderer;
  
  y += wid->rect.y;
  x += wid->rect.x;
  for (unsigned int i = 0; i < len; i += caract_per_line)
    {
      static char  buff[125];  
      SDL_Surface *textSurface;
      SDL_Texture* text;

      if ((len - i) > caract_per_line) {
	strncpy(buff, str + i, caract_per_line);
	buff[caract_per_line] = 0;
      } else {
	strncpy(buff, str + i, caract_per_line - (len - i));
	buff[caract_per_line - (len - i)] = 0;
      }

      textSurface = TTF_RenderUTF8_Solid(sgDefaultFont(),
						      buff, color);
      text = SDL_CreateTextureFromSurface(renderer, textSurface);
      text_width = textSurface->w;
      text_height = textSurface->h;
      SDL_FreeSurface(textSurface);

      SDL_Rect renderQuad = { x, y, text_width, text_height };
      SDL_RenderCopy(renderer, text, NULL, &renderQuad);
      SDL_DestroyTexture(text);
      y += text_height;
    }
  return 0;
}

void sdlResize(YWidgetState *wid, int renderType)
{
  SDLWid *swid = wid->renderStates[renderType].opac;

  swid->rect.h = wid->pos.h * WIN_H_SIZE / 1000;
  swid->rect.w = wid->pos.w * WIN_W_SIZE / 1000;
  swid->rect.x = wid->pos.x * WIN_W_SIZE / 1000;
  swid->rect.y = wid->pos.y * WIN_H_SIZE / 1000;
}

void sdlWidInit(YWidgetState *wid, int t)
{
  sdlResize(wid, t);
}

void sdlWidDestroy(YWidgetState *wid, int t)
{
  SDLWid *swid = wid->renderStates[t].opac;

  g_free(swid);
}
