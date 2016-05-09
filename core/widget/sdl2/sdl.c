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

void	sdlDrawRect(SDL_Rect rect, SDL_Color color)
{
  unsigned char r, g, b, a;

  SDL_GetRenderDrawColor(sg.renderer, &r, &g, &b, &a);
  SDL_SetRenderDrawColor(sg.renderer, color.r, color.g, color.b, color.a);
  SDL_RenderFillRect(sg.renderer, &rect);
  SDL_SetRenderDrawColor(sg.renderer, r, g, b, a);  
}

int   sdlFillColorBg(SDLWid *swid, short r, short g, short b, short a)
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
  return 0;
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

int    sdlFillBg(SDLWid *swid, YBgConf *cfg)
{
  if (cfg->type == BG_COLOR)
    return sdlFillColorBg(swid, cfg->r, cfg->g, cfg->b, cfg->a);
  else if (cfg->type == BG_IMG)
    return sdlFillImgBg(swid, cfg->path);
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
  if (key >= 'a' && key <= 'z')
    return key;
  switch (key)
    {
    case SDLK_UP:
      return Y_UP_KEY;
    case SDLK_DOWN:
      return Y_DOWN_KEY;
    case SDLK_LEFT:
      return Y_LEFT_KEY;
    case SDLK_RIGHT:
      return Y_RIGHT_KEY;
    case SDLK_RETURN:
      return '\n';
    case SDLK_TAB:
      return '\t';
    case SDLK_ESCAPE:
      return Y_ESC_KEY;
    default:
      return -1;
    }
}

static inline YEvent *SDLConvertEvent(SDL_Event* event)
{
  YEvent *eve = g_new0(YEvent, 1);
  
  if (!event)
    return NULL;
  switch(event->type)
    {
    case SDL_KEYUP:
      eve->type = YKEY_UP;
      break;
    case SDL_KEYDOWN:
      eve->type = YKEY_DOWN;
      break;
    default:
      eve->type = YKEY_NONE;
      break;
    }
  eve->key = convertToYKEY(event->key.keysym.sym);
  return eve;
}

static YEvent *SDLWaitEvent(void)
{
  static SDL_Event event;

  if (!SDL_WaitEvent(&event))
    return NULL;
  return SDLConvertEvent(&event);
}

static YEvent *SDLPollEvent(void)
{
  static SDL_Event event;

  if (!SDL_PollEvent(&event))
    return NULL;
  return SDLConvertEvent(&event);
}

int    ysdl2Init(void)
{
  if (type != -1)
    return type;

  /* Initialisation simple */
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0 ) {
    DPRINT_ERR("SDL initialisation failed: (%s)\n",SDL_GetError());
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

  /* Make a window */
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

  if (SDL_SetRenderDrawBlendMode(sg.renderer, SDL_BLENDMODE_BLEND) < 0) {
    goto fail;
  }

  if (sgSetDefaultFont("./sample.ttf") < 0)
    DPRINT_WARN("Cannot load \"./sample.ttf\"\n");

  
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

static int sdlPrintLine(SDLWid *wid,
			const char *str,
			unsigned int caract_per_line,
			SDL_Color color,
			int x, int y, int line)
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
	strncpy(buff, str + i, len - i);
	buff[len - i] = 0;
      }

      textSurface = TTF_RenderUTF8_Solid(sgDefaultFont(),
					 buff, color);
      text = SDL_CreateTextureFromSurface(renderer, textSurface);
      text_width = textSurface->w;
      text_height = textSurface->h;
      if (i == 0)
	y += textSurface->h * line;	

      SDL_FreeSurface(textSurface);
      SDL_Rect renderQuad = { x, y, text_width, text_height };
      SDL_RenderCopy(renderer, text, NULL, &renderQuad);
      SDL_DestroyTexture(text);
      y += text_height;
    }
  return 0;
}

int sdlPrintText(SDLWid *wid,
		 const char *str,
		 unsigned int caract_per_line,
		 SDL_Color color,
		 int x, int y)
{
  if (!str)
    return 0;
  char **tmp = g_strsplit(str, "\n", 0);
  int ret = 0;

  for (int i = 0; tmp[i]; ++i) {
    ret = sdlPrintLine(wid, tmp[i], caract_per_line, color, x, y, i);
    if (ret < 0)
      goto exit;
  }
 exit:
  g_strfreev(tmp);
  return ret;
}

void sdlResize(YWidgetState *wid, int renderType)
{
  SDLWid *swid = wid->renderStates[renderType].opac;
  Entity *pos = yeGet(wid->entity, "wid-pos");

  swid->rect.h = yeGetInt(yeGet(pos, "h")) * WIN_H_SIZE / 1000;
  swid->rect.w = yeGetInt(yeGet(pos, "w")) * WIN_W_SIZE / 1000;
  swid->rect.x = yeGetInt(yeGet(pos, "x")) * WIN_W_SIZE / 1000;
  swid->rect.y = yeGetInt(yeGet(pos, "y")) * WIN_H_SIZE / 1000;
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

/* Wrapper for DataEntity destroy */
static void sdlFreeTexture(void *txt)
{
  SDL_DestroyTexture(txt);
}

#define Y_SDL_TILD 1
#define Y_SDL_SPRITE 2

static SDL_Texture *sdlLoasAndCachImg(Entity *elem)
{
  const char *path;
  SDL_Texture *texture = yeGetData(yeGet(elem, "$sdl-img"));
  Entity *data;

  if (texture)
    return texture;
  SDL_Surface *image;

  if ((path = yeGetString(yeGet(elem, "map-tild"))) != NULL)
    yeCreateInt(Y_SDL_TILD, elem, "$sdl-type");
  else if ((path = yeGetString(yeGet(elem, "map-srite"))) != NULL)
    yeCreateInt(Y_SDL_SPRITE, elem, "$sdl-type");
  else
    return NULL;

  image = IMG_Load(path);
  if (!image) {
    return NULL;
  }

  texture = SDL_CreateTextureFromSurface(sg.renderer, image);
  data = yeCreateData(texture, elem, "$sdl-img");
  yeSetDestroy(data, sdlFreeTexture);
  SDL_FreeSurface(image);
  return texture;
}

int sdlDisplaySprites(SDLWid *wid, int x, int y, Entity *elem,
		      int w, int h, int thresholdX)
{
  SDL_Color color = {0,0,0,255};
  SDL_Rect DestR;
  SDL_Texture *texture = sdlLoasAndCachImg(elem);


  if (texture) {
    int type = yeGetInt(yeGet(elem, "$sdl-type"));
    
    if (type == Y_SDL_TILD) {
      DestR.x = x * w + wid->rect.x + thresholdX;
      DestR.y = y * h + wid->rect.y;
      DestR.w = w;
      DestR.h = h;
      SDL_RenderCopy(sg.renderer, texture, NULL, &DestR);
    } else {
      SDL_QueryTexture(texture, NULL, NULL, &DestR.w, &DestR.h);
      DestR.x = x * w + wid->rect.x + thresholdX;
      DestR.y = y * h - (DestR.h - h) + wid->rect.y;
      SDL_RenderCopy(sg.renderer, texture, NULL, &DestR);
    }
  } else {
    return sdlPrintText(wid, yeGetString(yeGet(elem, "map-char")),
			2, color, x * w, y * h);
  }
  return 0;
}
