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
#include "canvas-sdl.h"
#include "utils.h"
#include "widget.h"
#include "rect.h"
#include "map.h"
#include "canvas.h"

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

int ysdl2WindowMode(void)
{
  return SDL_SetWindowFullscreen(sg.pWindow, 0);
}

int ysdl2FullScreen(void)
{
  return SDL_SetWindowFullscreen(sg.pWindow, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS);
}

int sgSetDefaultFont(const char *path)
{
  TTF_Font *font;
  int w, h;

  if (access(path, F_OK ) < 0) {
    return -1;
  }
  font = TTF_OpenFont(path, 16);
  if (!font)
    return -1;
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
  SDL_RenderClear(sg.renderer);
  return 0;
}

SDL_Rect sdlRectFromRectEntity(Entity *rect)
{
  SDL_Rect ret = {ywRectX(rect), ywRectY(rect),
		  ywRectW(rect), ywRectH(rect)};

  return ret;
}

void	sdlDrawRect(SDLWid *swid, SDL_Rect rect, SDL_Color color)
{
  unsigned char r, g, b, a;

  if (swid) {
    rect.y += swid->rect.y;
    rect.x += swid->rect.x;
  }

  SDL_GetRenderDrawColor(sg.renderer, &r, &g, &b, &a);
  SDL_SetRenderDrawColor(sg.renderer, color.r, color.g, color.b, color.a);
  SDL_RenderFillRect(sg.renderer, &rect);
  SDL_SetRenderDrawColor(sg.renderer, r, g, b, a);
}

int   sdlFillColorBg(SDLWid *swid, short r, short g, short b, short a)
{
  SDL_Color color = {r, g, b, a};

  sdlDrawRect(NULL, swid->rect, color);
  return 0;
}

int    sdlFillImgBg(SDLWid *swid, const char *cimg)
{
  if (cimg) {
    SDL_Surface *img = IMG_Load(cimg);
    if (!img)
      return -1;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(sg.renderer, img);
    SDL_FreeSurface(img);
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
  if ((key >= 'a' && key <= 'z') ||
      (key >= '0' && key <= '9') || key == ' ')
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

static inline Entity *SDLConvertEvent(SDL_Event* event)
{
  Entity *eve = yeCreateArray(NULL, NULL);

  if (!event || !eve)
    return NULL;
  yeCreateIntAt(NOTHANDLE, eve, NULL, YEVE_STATUS);
  switch(event->type)
    {
    case SDL_KEYUP:
      yeCreateIntAt(YKEY_UP, eve, NULL, YEVE_TYPE);
      break;
    case SDL_KEYDOWN:
      yeCreateIntAt(YKEY_DOWN, eve, NULL, YEVE_TYPE);
      break;
    case SDL_MOUSEBUTTONDOWN:
      {
	Entity *mouse = ywPosCreateInts(event->button.x, event->button.y,
					NULL, NULL);
	yeCreateIntAt(YKEY_MOUSEDOWN, eve, NULL, YEVE_TYPE);
	yeCreateIntAt(event->button.button, eve, NULL, YEVE_KEY);
	yePushAt(eve, mouse, YEVE_MOUSE);
	yeDestroy(mouse);
      }
      return eve;
    case SDL_MOUSEWHEEL:
      yeCreateIntAt(YKEY_MOUSEWHEEL, eve, NULL, YEVE_TYPE);
      yeCreateIntAt(event->wheel.y, eve, NULL, YEVE_KEY);
      return eve;
    case SDL_MOUSEMOTION:
      {
	Entity *mouse = ywPosCreateInts(event->button.x, event->button.y,
					NULL, NULL);
	yeCreateIntAt(YKEY_MOUSEMOTION, eve, NULL, YEVE_TYPE);
	yeCreateIntAt(-1, eve, NULL, YEVE_KEY);
	yePushAt(eve, mouse, YEVE_MOUSE);
	yeDestroy(mouse);
      }
      return eve;
    default:
      yeCreateIntAt(YKEY_NONE, eve, NULL, YEVE_TYPE);
      break;
    }
  yeCreateIntAt(convertToYKEY(event->key.keysym.sym), eve, NULL, YEVE_KEY);
  return eve;
}

static Entity *SDLWaitEvent(void)
{
  static SDL_Event event;

  if (!SDL_WaitEvent(&event))
    return NULL;
  return SDLConvertEvent(&event);
}

static Entity *SDLPollEvent(void)
{
  static SDL_Event event;

  if (!SDL_PollEvent(&event))
    return NULL;
  return SDLConvertEvent(&event);
}

static void changeWindName(const char *name)
{
  SDL_SetWindowTitle(sg.pWindow, name);
}

static int sdlRenderCreate(void)
{
    sg.renderer = SDL_CreateRenderer(sg.pWindow, -1,
				   SDL_RENDERER_TARGETTEXTURE);
  if (!sg.renderer) {
    DPRINT_ERR("Get render from window: %s\n", SDL_GetError());
    return -1;
  }

  if (SDL_SetRenderDrawBlendMode(sg.renderer, SDL_BLENDMODE_BLEND) < 0) {
    DPRINT_ERR("failt to set blend mode: %s\n", SDL_GetError());
    return -1;
  }
  return 0;
}

static int sdlChangeResolution(void)
{
  SDL_SetWindowSize(sg.pWindow, ywidWindowWidth, ywidWindowHight);
  SDL_DestroyRenderer(sg.renderer);
  if (sdlRenderCreate() < 0) {
    DPRINT_ERR("SDL is DEAD");
    ysdl2Destroy();
    return -1;
  }
  SDL_RenderClear(sg.renderer);
  return 0;
}

int    ysdl2Init(void)
{
  if (type != -1)
    return type;

  /* Initialisation simple */
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0 ) {
    DPRINT_ERR("SDL initialisation failed: (%s)\n", SDL_GetError());
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
  sg.pWindow = SDL_CreateWindow(ywidWindowName(), SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				ywidWindowWidth,
				ywidWindowHight,
				SDL_WINDOW_SHOWN);
  if(!sg.pWindow) {
      DPRINT_ERR("Error to creeate window:: %s\n",SDL_GetError());
      goto win_fail;
  }

  // Render for the main windows
  if (sdlRenderCreate() < 0)
    goto fail;

  if (sgSetDefaultFont("./DejaVuSansMono.ttf") < 0 &&
      sgSetDefaultFont("/Library/Fonts/Tahoma.ttf") < 0 &&
      sgSetDefaultFont("/usr/share/fonts/TTF/DejaVuSansMono.ttf") < 0 &&
      sgSetDefaultFont("C:\\Windows\\Fonts\\constanb.ttf")) {
    DPRINT_ERR("Cannot load fonts\n");
    goto fail;
  }
  // fill the window with a black rectangle
  // SDL_Rect   rect = sg.getRect();

  SDL_RenderClear(sg.renderer);
  type = ywidRegistreRender(sdlResize, SDLPollEvent, SDLWaitEvent,
			    sdlDraw, sdlChangeResolution,
			    changeWindName);
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

  pos.y += wid->rect.y + fontSize * line;
  pos.x += wid->rect.x;
  for (int i = 0; i < len; i += caract_per_line) {
      SDL_Surface *textSurface;
      SDL_Texture* text;
      char tmp = 0;

      if ((len - i) > caract_per_line) {
	tmp = str[i + caract_per_line];
	str[i + caract_per_line] = 0;
	ret += 1;
      }

      if (pos.y >= wid->rect.y && pos.y + fontSize <= wid->rect.y + wid->rect.h) {
	textSurface = TTF_RenderUTF8_Solid(sgDefaultFont(), str + i, color);
	text = SDL_CreateTextureFromSurface(renderer, textSurface);

	if (!text) {
	  SDL_FreeSurface(textSurface);
	  return -1;
	}
	text_width = textSurface->w;
	SDL_FreeSurface(textSurface);

	SDL_Rect renderQuad = { pos.x, pos.y, text_width, fontSize};

	if (alignementType == YSDL_ALIGN_CENTER)
	  renderQuad.x = pos.x + ((pos.w / 2) - (text_width / 2));
	if (SDL_RenderCopy(renderer, text, NULL, &renderQuad) < 0)
	  DPRINT_ERR("sdl fail to rend text\n");
	SDL_DestroyTexture(text);
      }
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
  Entity *pos = yeGet(wid->entity, "wid-pix");

  swid->rect.h = ywRectH(pos);
  swid->rect.w = ywRectW(pos);
  swid->rect.x = ywRectX(pos);
  swid->rect.y = ywRectY(pos);
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

void sdlFreeSurface(void *surface)
{
  SDL_FreeSurface(surface);
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
  }

  if (unlikely(!path || !(image =  IMG_Load(path)))) {
    if ((path = yeGetString(yeGet(elem, "map-char"))) != NULL) {
      SDL_Color color = {0,0,0,255};
      char *str = (char *)path;
      char tmp;

      tmp = str[1];
      str[1] = 0;
      image = TTF_RenderUTF8_Solid(sgDefaultFont(), str, color);
      yeReCreateInt(Y_SDL_TILD, elem, "$sdl-type");
      str[1] = tmp;
    } else {
      return NULL;
    }
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
  g_error_free(sdlError);
  sdlError = NULL;
}

static int sdlCanvasCacheText(Entity *state, Entity *elem, Entity *resource,
			      const char *str)
{
  SDL_Color color = {0,0,0,255};
  SDL_Surface *image;
  SDL_Texture *texture;
  Entity *data;
  int w = 0, h = 0;

  image = TTF_RenderUTF8_Solid(sgDefaultFont(), str, color);
  texture = SDL_CreateTextureFromSurface(sg.renderer, image);
  SDL_QueryTexture(texture, NULL, NULL, &w, &h);
  data = yeCreateData(texture, elem, "$img");
  yeSetDestroy(data, sdlFreeTexture);
  ywSizeCreate(w, h, elem, "$size");
  data = yeCreateData(image, elem, "$img-surface");
  yeSetDestroy(data, sdlFreeSurface);

  yeGetPush(elem, resource, "$img");
  yeGetPush(elem, resource, "$size");
  yeGetPush(elem, resource, "$img-surface");
  return 0;
}

int sdlMergeSurface(Entity *textSrc, Entity *srcRect,
		    Entity *textDest, Entity *destRect)
{
  SDL_Surface *sSurface = yeGetDataAt(textSrc, "$img-surface");
  SDL_Surface *dSurface = yeGetDataAt(textDest, "$img-surface");

  /* FIXME: TODO */
  (void)srcRect;
  (void)destRect;
  if (unlikely(!sSurface || !dSurface)) {
    return -1;
  }
  return SDL_BlitSurface(sSurface, NULL, dSurface, NULL);  
}

SDL_Surface *sdlCopySurface(SDL_Surface *surface, Entity *rEnt)
{
  SDL_Rect r;
  SDL_Rect *rptr = NULL;
  SDL_Surface *tmpSurface = surface;
  int w = surface->w, h = surface->h;

  if (rEnt) {
    r.x = ywRectX(rEnt);
    r.y = ywRectY(rEnt);
    r.w = ywRectW(rEnt);
    r.h = ywRectH(rEnt);
    w = r.w;
    h = r.h;
    rptr = &r;
  }

  surface = SDL_CreateRGBSurface(0, w, h, 32, surface->format->Rmask,
  				 surface->format->Gmask,
  				 surface->format->Bmask,
  				 surface->format->Amask);
  uint32_t ck0;
  int ri = SDL_GetColorKey(tmpSurface, &ck0);
  if (!ri)
    SDL_SetColorKey(surface, 1, ck0);

  if (unlikely(!surface)) {
    DPRINT_ERR("fail to create surface");
    SDL_FreeSurface(tmpSurface);
    return NULL;
  }

  SDL_BlitSurface(tmpSurface, rptr, surface, NULL);
  return surface;
}

int sdlCanvasCacheImg(Entity *elem, Entity *resource, const char *imgPath,
		      Entity *rEnt)
{
  SDL_Surface *surface;
  SDL_Texture *texture;
  Entity *data;
  int w, h;

  if (!rEnt)
    rEnt = yeGet(elem, "img-src-rect") ? : yeGet(resource, "img-src-rect");
  if (!imgPath) {
    surface = yeGetData(yeGet(resource, "$img-surface"));
  } else {
    surface = IMG_Load(imgPath);
  }
  if (unlikely(!surface)) {
    DPRINT_ERR("fail to load %s", imgPath);
    return -1;
  }

  if (rEnt) {
    SDL_Surface *tmpSurface = surface;

    surface = sdlCopySurface(tmpSurface, rEnt);
    if (!surface)
      return -1;
    if (imgPath)
      SDL_FreeSurface(tmpSurface);
    /* trick to sdlFreeSurface anyway */
    imgPath = "";
  }
  texture = SDL_CreateTextureFromSurface(sg.renderer, surface);
  if (unlikely(!texture))
    return -1;
  SDL_QueryTexture(texture, NULL, NULL, &w, &h);
  data = yeCreateData(texture, elem, "$img");
  yeSetDestroy(data, sdlFreeTexture);
  ywSizeCreate(w, h, elem, "$size");
  data = yeCreateData(surface, elem, "$img-surface");
  /* if not img path a texture was use */
  if (imgPath)
    yeSetDestroy(data, sdlFreeSurface);
  yeGetPush(elem, resource, "$img");
  yeGetPush(elem, resource, "$size");
  yeGetPush(elem, resource, "$img-surface");
  return 0;
}

int sdlCanvasCacheTexture(Entity *state, Entity *elem)
{
  int type = yeGetIntAt(elem, 0);
  Entity *resource;
  const char *txt;

  if (type == YCanvasRect) {
    return 0;
  } else if (ywCanvasObjType(elem) == YCanvasString) {
    Entity *str = yeGet(elem, 2);
    int w = 0, h;

    h = yuiStrCountCh(yeGetString(str), '\n', &w);
    w = (w + 1) * (sg.txtWidth);
    h = (h + 1) * (sgGetFontSize() + 2);

    ywPosCreateInts(w, h, elem, "$size");
    return 0;
  } else if (unlikely(type == YCanvasImg)) {
    txt = yeGetStringAt(elem, 2);
    if (txt)
      return sdlCanvasCacheImg(elem, NULL, txt, NULL);
  } else if (unlikely(type == YCanvasTexture)) {
    return sdlCanvasCacheImg(elem, yeGet(elem, 2), NULL, NULL);
  } else if (unlikely(type != YCanvasResource)) {
    return -1;
  }

  resource = yeGet(yeGet(state, "resources"),
			   yeGetIntAt(elem, 2));
  if (yeGetPush(resource, elem, "$img")) {
    yeGetPush(resource, elem, "$size");
    yeGetPush(resource, elem, "$img-surface");
    return 0;
  }

  txt = yeGetStringAt(resource, "img");
  if (txt)
    return sdlCanvasCacheImg(elem, resource, txt, NULL);
  txt = yeGetStringAt(resource, "text");
  if (txt)
    return sdlCanvasCacheText(state, elem, resource, txt);
  return -1;
}

static void sdlCanvasAplyModifier(Entity *img, SDL_Rect *dst,
				  SDL_Rect **src, SDL_Point **center,
				  double *rotation, SDL_RendererFlip *flip)
{
  Entity *mod = ywCanvasObjMod(img);

  if (!mod)
    return;
  *rotation = yeGetFloat(yeGet(mod, YCanvasRotate));
  return;
}

static int sdlCanvasRendImg(YWidgetState *state, SDLWid *wid, Entity *img,
			    Entity *cam)
{
  SDL_Texture *t = yeGetData(yeGet(img, "$img"));
  if (!t) {
    sdlCanvasCacheTexture(state->entity, img);
    t = yeGetData(yeGet(img, "$img"));
  }
  Entity *wid_pix = yeGet(state->entity, "wid-pix");
  Entity *s = ywCanvasObjSize(state->entity, img);
  Entity *p = ywCanvasObjPos(img);
  SDL_Rect *sd = NULL;
  SDL_Point *center = NULL;
  SDL_Rect rd = { ywPosX(p) - ywPosX(cam), ywPosY(p) - ywPosY(cam),
		  ywSizeW(s), ywSizeH(s) };
  double rotation = 0;
  SDL_RendererFlip flip = SDL_FLIP_NONE;

  if (unlikely(!t))
    return -1;
  rd.x += ywRectX(wid_pix);
  rd.y += ywRectY(wid_pix);
  sdlCanvasAplyModifier(img, &rd, &sd, &center, &rotation, &flip);
  SDL_RenderCopyEx(sg.renderer, t, sd, &rd, rotation, center, flip);
  free(sd);
  free(center);
  return 0;
}

uint32_t sdlCanvasPixInfo(Entity *obj, int x, int y)
{
  if (!obj)
    return 0;
  SDL_Surface *surface = yeGetDataAt(obj, "$img-surface");
  int type = yeGetIntAt(obj, 0);

  if (type == YCanvasRect) {
    YBgConf cfg;
    Entity *s = ywCanvasObjSize(NULL, obj);

    if (y < 0 || y >= ywSizeH(s) || x < 0 || x > ywSizeW(s))
      return 0;
    ywidBgConfFill(yeGet(yeGet(obj, 2), 1), &cfg);
    return cfg.rgba;
  }
  if (!surface) {
    return 0;
  }

  if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
    return 0;
  }

  switch (surface->format->BitsPerPixel) {
  case 32:
    if (!SDL_ISPIXELFORMAT_ALPHA(surface->format->format) &&
	((uint32_t *)surface->pixels)[x + surface->w * y]) {
	YCanvasPixiel ret;
	ret.i = ((uint32_t *)surface->pixels)[x + surface->w * y];
	ret.rgba[3] = 255; /* I guess if Alpha is store at the begin, we're fuck */

	return ret.i;
    }
    return ((uint32_t *)surface->pixels)[x + surface->w * y];
  case 8:
    {
      union {
	SDL_Color s_col;
	uint32_t i;
      } ret;
      int index = ((Uint8 *)surface->pixels)[(x + surface->w * y)];

      ret.s_col =  surface->format->palette->colors[index];
      return ret.i;
    }
  default:
    DPRINT_ERR("unsuported pixiel format, %dbits per pixiels",
	       surface->format->BitsPerPixel);
  }
  return 0;
}

int sdlCanvasRendObj(YWidgetState *state, SDLWid *wid, Entity *obj, Entity *cam)
{
  int type = yeGetIntAt(obj, 0);

  if (type == YCanvasResource || type == YCanvasImg || type == YCanvasTexture)
    return sdlCanvasRendImg(state, wid, obj, cam);

  Entity *p = ywCanvasObjPos(obj);
  SDL_Color c = {0, 0, 0, 255};
  Entity *s = ywCanvasObjSize(state->entity, obj);
  SDL_Rect rect = {ywPosX(p) - ywPosX(cam), ywPosY(p) - ywPosY(cam),
		   ywSizeW(s), ywSizeH(s)};

  if (type == YCanvasRect) {
    YBgConf cfg;
    Entity *wid_pix = yeGet(state->entity, "wid-pix");

    ywidBgConfFill(yeGet(yeGet(obj, 2), 1), &cfg);
    c.r = cfg.r;
    c.g = cfg.g;
    c.b = cfg.b;
    c.a = cfg.a;
    rect.x += ywRectX(wid_pix);
    rect.y += ywRectY(wid_pix);
    // stuff to do here
    sdlDrawRect(NULL, rect, c);
    return 0;
  } else if (type == YCanvasString) {
    if (yeGet(obj, 3)) {
      ywidColorFromString((char *)yeGetStringAt(obj, 3),
			  &c.r, &c.g, &c.b, &c.a);
    }
    sdlPrintText(wid, yeGetStringAt(obj, 2), c, rect, YSDL_ALIGN_LEFT);
  }
  return -1;
}

int sdlDisplaySprites(YWidgetState *state, SDLWid *wid,
		      int x, int y, Entity *mapElem,
		      int w, int h, int thresholdX,
		      int thresholdY, Entity *mod)
{
  SDL_Rect DestR = {x * w + wid->rect.x + thresholdX,
		    y * h + wid->rect.y + thresholdY,
		    w, h};
  SDL_Texture *texture;
  int id;
  Entity *elem;

  if (unlikely(!mapElem))
    return 0;
  id = ywMapGetIdByElem(mapElem);
  elem = yeGet(ywMapGetResources(state), id);

  texture = sdlLoasAndCachTexture(elem);

  if (texture) {
    int type = yeGetInt(yeGet(elem, "$sdl-type"));

    if (type == Y_SDL_TILD || type == Y_SDL_COLOR) {
      SDL_Rect srcR = {0, 0, 0, 0};
      SDL_Rect *srcRP = NULL;

      SDL_QueryTexture(texture, NULL, NULL, &srcR.w, &srcR.h);
      int diff = srcR.w - srcR.h;
      if (diff > 0) {
      	int bigger = srcR.w;
      	DestR.h = DestR.h * bigger / DestR.w;
	diff = DestR.w - DestR.h;
	DestR.y += (diff / 2);
      } else if (diff < 0) {
      	int bigger = srcR.h;
	DestR.w = DestR.w * bigger / DestR.h;
	diff = DestR.h - DestR.w;
	DestR.x += (diff / 2);
      }

      if (unlikely(mod) && !yeGetIntAt(mod, 0)) {
	DestR.x += yeGetIntAt(mod, 1);
	DestR.w -= yeGetIntAt(mod, 2);
	srcR.x += yuiPercentOf(srcR.w, yeGetIntAt(mod, 3));
	srcR.w -= yuiPercentOf(srcR.w, yeGetIntAt(mod, 4));
	srcRP = &srcR;
      }

      if (type == Y_SDL_COLOR)
	sdlDrawRect(NULL, DestR, *((SDL_Color *)texture));
      else
	SDL_RenderCopy(sg.renderer, texture, srcRP, &DestR);
    } else {
      SDL_QueryTexture(texture, NULL, NULL, &DestR.w, &DestR.h);
      DestR.y = y * h - (DestR.h - h) + wid->rect.y;
      SDL_RenderCopy(sg.renderer, texture, NULL, &DestR);
    }
  }
  return 0;
}
