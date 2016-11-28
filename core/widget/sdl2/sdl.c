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
#include "map.h"

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
  int w, h;
  sg.fontSize = 16;
  TTF_SizeText(font, "A", &w, &h);

  sg.txtHeight = h;
  sg.txtWidth = w;

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

static int	sdlDraw(void)
{
  SDL_RenderPresent(sg.renderer);
  return 0;
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
  SDL_Color color = {r, g, b, a};

  sdlDrawRect(swid->rect, color);
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
  
  if (!event || !eve)
    return NULL;
  switch(event->type)
    {
    case SDL_KEYUP:
      eve->type = YKEY_UP;
      break;
    case SDL_KEYDOWN:
      eve->type = YKEY_DOWN;
      break;
    case SDL_MOUSEBUTTONDOWN:
      eve->type = YKEY_MOUSEDOWN;
      eve->key = event->button.button;
      eve->xMouse = event->button.x;
      eve->yMouse = event->button.y;
      return eve;
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

  // Simple check of the Flags
  if(!IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF)) {
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
  sg.renderer = SDL_CreateRenderer(sg.pWindow, -1,
				   SDL_RENDERER_TARGETTEXTURE);
  if (!sg.renderer) {
    DPRINT_ERR("Get render from window: %s\n", SDL_GetError());
    goto fail;
  }

  if (SDL_SetRenderDrawBlendMode(sg.renderer, SDL_BLENDMODE_BLEND) < 0) {
    goto fail;
  }

  if (sgSetDefaultFont("./sample.ttf") < 0) {
    DPRINT_ERR("Cannot load \"./sample.ttf\"\n");
    goto fail;
  }
  
  // fill the window with a black rectangle
  // SDL_Rect   rect = sg.getRect();

  SDL_RenderClear(sg.renderer);
  type = ywidRegistreRender(sdlResize, SDLPollEvent, SDLWaitEvent, sdlDraw);
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

static inline int sdlPrintLine(SDLWid *wid,
			       char *str,
			       SDL_Color color,
			       SDL_Rect pos,
			       int line,
			       int alignementType)
{
  int len = strlen(str);
  int text_width;
  SDL_Renderer *renderer = sg.renderer;
  int caract_per_line = len;
  int ret = 0;
  int32_t fontSize = sgGetFontSize();

  if ((fontSize * len) > pos.w) {
    caract_per_line = pos.w / sg.txtWidth;
  }

  pos.y += wid->rect.y + sgGetFontSize() * line;
  pos.x += wid->rect.x;
  for (int i = 0; i < len; i += caract_per_line)
    {
      SDL_Surface *textSurface;
      SDL_Texture* text;
      char tmp = 0;

      if ((len - i) > caract_per_line) {
	tmp = str[i + caract_per_line];
	str[i + caract_per_line] = 0;
	ret += 1;
      }

      textSurface = TTF_RenderUTF8_Solid(sgDefaultFont(), str + i, color);
      text = SDL_CreateTextureFromSurface(renderer, textSurface);
      text_width = textSurface->w;

      SDL_FreeSurface(textSurface);
      SDL_Rect renderQuad = { pos.x, pos.y, text_width, fontSize};
      if (alignementType == YSDL_ALIGN_CENTER)
	renderQuad.x = pos.x + ((pos.w / 2) - (text_width / 2));
      SDL_RenderCopy(renderer, text, NULL, &renderQuad);
      SDL_DestroyTexture(text);
      pos.y += fontSize;
      if (tmp)
	str[i + caract_per_line] = tmp;
    }
  return ret;
}

int sdlPrintText(SDLWid *wid,
		 const char *str,
		 SDL_Color color,
		 SDL_Rect pos,
		 int alignementType)
{
  if (!str)
    return 0;
  char **tmp = g_strsplit(str, "\n", 0);
  int ret = 0;
  int aditioner = 0;
  int end;

  for (end = 0; tmp[end]; ++end);

  for (int i = 0; i < end; ++i) {
    ret = sdlPrintLine(wid, tmp[i], color, pos, i + aditioner, alignementType);
    if (ret < 0)
      goto exit;
    aditioner += ret;
  }
 exit:
  g_strfreev(tmp);
  return 0;
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
#define Y_SDL_COLOR 3

static SDL_Texture *sdlLoasAndCachTexture(Entity *elem)
{
  const char *path = NULL;
  SDL_Texture *texture = yeGetData(yeGet(elem, "$sdl-img"));
  Entity *data;

  if (texture)
    return texture;
  SDL_Surface *image;

  if (!yeGet(elem, "$sdl-type")) {
    if ((path = yeGetString(yeGet(elem, "map-tild"))) != NULL) {
      yeCreateInt(Y_SDL_TILD, elem, "$sdl-type");
    } else if ((path = yeGetString(yeGet(elem, "map-srite"))) != NULL) {
      yeCreateInt(Y_SDL_SPRITE, elem, "$sdl-type");
    } else if ((path = yeGetString(yeGet(elem, "map-color"))) != NULL) {
      SDL_Color *col = g_new(SDL_Color, 1);

      ywidColorFromString((char *)path, &col->r, &col->g, &col->b, &col->a);
      yeCreateInt(Y_SDL_COLOR, elem, "$sdl-type");
      data = yeCreateData(col, elem, "$sdl-img");
      yeSetDestroy(data, g_free);
      return (SDL_Texture *)col;
    } else {
      return NULL;
    }

  }

  if (unlikely(!path || !(image =  IMG_Load(path)))) {
    return NULL;
  }

  texture = SDL_CreateTextureFromSurface(sg.renderer, image);
  data = yeCreateData(texture, elem, "$sdl-img");
  yeSetDestroy(data, sdlFreeTexture);
  SDL_FreeSurface(image);
  return texture;
}

static GError *sdlError;

void sdlConsumeError(void)
{
  if (!sdlError)
    return;
  fprintf(stderr, "%s\n", sdlError->message);
  g_error_free (sdlError);
  sdlError = NULL;
}

int sdlDisplaySprites(YWidgetState *state, SDLWid *wid,
		      int x, int y, Entity *mapElem,
		      int w, int h, int thresholdX)
{
  SDL_Color color = {0,0,0,255};
  SDL_Rect DestR = {x * w + wid->rect.x + thresholdX,
		    y * h + wid->rect.y,
		    w, h};
  SDL_Texture *texture;
  int id;
  Entity *elem;

  if (unlikely(!mapElem))
    return 0;;
  id = ywMapGetIdByElem(mapElem);
  elem = yeGet(ywMapGetResources(state), id);

  texture = sdlLoasAndCachTexture(elem);

  if (texture) {
    int type = yeGetInt(yeGet(elem, "$sdl-type"));

    if (type == Y_SDL_TILD || type == Y_SDL_COLOR) {
      if (type == Y_SDL_COLOR)
	sdlDrawRect(DestR, *((SDL_Color *)texture));
      else
	SDL_RenderCopy(sg.renderer, texture, NULL, &DestR);
    } else {
      SDL_QueryTexture(texture, NULL, NULL, &DestR.w, &DestR.h);
      DestR.y = y * h - (DestR.h - h) + wid->rect.y;
      SDL_RenderCopy(sg.renderer, texture, NULL, &DestR);
    }
  } else {
    Entity *entStr = yeGet(elem, "map-char");
    char *str = (char *)yeGetString(entStr);
    int ret;
      char tmp;

    if (unlikely(!str)) {
      Entity *char_sprite = yeGet(elem, "map-char");
      char *entityStr;

      if (char_sprite)
	return 0;
      entityStr = yeToCStr(elem, -1, 0);

      if (yeType(char_sprite) != YSTRING) {
	sdlError = g_error_new(1, 1,
			       "map-char of elem :\n\%s\n"
			       "At pos %d %d should be string but is %s instead\n",
			       entityStr, x, y,
			       yeTypeToString(yeType(char_sprite)));
      }
      g_free(entityStr);
      return -1;
    }
    DestR.x = x * w + thresholdX;
    DestR.y = y * h;
    tmp = str[1];
    str[1] = 0;
    ret = sdlPrintText(wid, str, color, DestR, YSDL_ALIGN_CENTER);
    str[1] = tmp;
    return ret;
  }
  return 0;
}
