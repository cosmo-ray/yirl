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

/* You can admirate the beautiful shape of the includes below */
/*      |      */
/*      V      */
#include <glib.h>
#include <assert.h>
#include <unistd.h>
#include <SDL_gpu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "sdl-internal.h"
#include "canvas-sdl.h"
#include "widget.h"
#include "utils.h"
#include "rect.h"
#include "map.h"
#include "game.h"
#include "canvas.h"

static int type = -1;

static SDL_Global sg;

#define MAX_GAMEPAD 128

static SDL_GameController *gamepads[MAX_GAMEPAD];
static SDL_Joystick *joysticks[MAX_GAMEPAD];

static inline void deinit_controllers(void)
{
	for (int i = 0; i < MAX_GAMEPAD; ++i) {
		if (gamepads[i])
			SDL_GameControllerClose(gamepads[i]);
		if (joysticks[i])
			SDL_JoystickClose(joysticks[i]);
	}
}

GPU_Rect getRect(void)
{
	GPU_Rect ret = {0, 0, sg.pWindow->w, sg.pWindow->h};

	return (ret);
}

TTF_Font *sgDefaultFont(void)
{
	return sg.font;
}

int ysdl2WindowMode(void)
{
	/* SDL_SetWindowGrab(sg.pWindow, 0); */
	return GPU_SetFullscreen(0, 1);
}

int ysdl2FullScreen(void)
{
	int r;
	r = GPU_SetFullscreen(1, 1);
	return r;
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
	return 0;
}

uint32_t sgGetTxtW(void)
{
	return sg.txtWidth;
}

uint32_t sgGetTxtH(void)
{
	return sg.txtHeight;
}

int sgGetFontSize(void)
{
	return sg.fontSize;
}

static int	sdlDraw(void)
{
	GPU_Flip(sg.pWindow);
	GPU_Clear(sg.pWindow);
	return 0;
}

void	sdlDrawRect(SDLWid *swid, GPU_Rect rect, SDL_Color color)
{
  if (swid) {
    rect.y += swid->rect.y;
    rect.x += swid->rect.x;
  }

  GPU_RectangleFilled2(sg.pWindow, rect, color);
}

int   sdlFillColorBg(SDLWid *swid, short r, short g, short b, short a)
{
	SDL_Color color = {r, g, b, a};

	sdlDrawRect(NULL, swid->rect, color);
	return 0;
}

static int    sdlFillImgBg(SDLWid *swid, YBgConf *cfg)
{
	const char *cimg = cfg->path;
	if (cimg) {
		SDL_Surface *img = IMG_Load(cimg);
		if (!img)
			return -1;
		GPU_Image *texture = GPU_CopyImageFromSurface(img);
		if (cfg->flag & YBG_FIT_TO_SCREEN) {
			GPU_Rect drect = swid->rect;

			if (cfg->flag & YBG_FIT_TO_SCREEN_H) {
				drect.w = img->w * drect.h / img->h;
				drect.x += (swid->rect.w / 2) - drect.w / 2;
			} else {
				drect.h = img->h * drect.w / img->w;
				drect.y += (swid->rect.h / 2) - drect.h / 2;
			}
			GPU_BlitRect(texture, NULL, sg.pWindow, &drect);
		} else {
			GPU_BlitRect(texture, NULL, sg.pWindow, &swid->rect);
		}
		SDL_FreeSurface(img);
		GPU_FreeImage(texture);
		return 0;
	}
	return -1;
}

int    sdlFillBg(SDLWid *swid, YBgConf *cfg)
{
	if (cfg->type == BG_COLOR)
		return sdlFillColorBg(swid, cfg->r, cfg->g, cfg->b, cfg->a);
	else if (cfg->type == BG_IMG)
		return sdlFillImgBg(swid, cfg);
	return -1;
}

void    ysdl2Destroy(void)
{
	if (type == -1)
		return;
	TTF_CloseFont(sg.font);
	GPU_FreeTarget(sg.pWindow);
	sg.pWindow = NULL;
	IMG_Quit();
	TTF_Quit();
	deinit_controllers();
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
	case SDLK_LSHIFT:
		return Y_LSHIFT_KEY;
	case SDLK_LCTRL:
		return Y_LCTRL_KEY;
	case SDLK_RSHIFT:
		return Y_RSHIFT_KEY;
	case SDLK_RCTRL:
		return Y_RCTRL_KEY;
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
	case SDL_QUIT:
	case SDL_APP_TERMINATING:
		printf("Quit requested by user");
		if (ygIsInit())
			ygTerminate();
		break;
	case SDL_WINDOWEVENT:
		yeveWindowGetFocus = 1;
		break;
	case SDL_KEYUP:
		yeCreateIntAt(YKEY_UP, eve, NULL, YEVE_TYPE);
		break;
	case SDL_KEYDOWN:
		yeCreateIntAt(YKEY_DOWN, eve, NULL, YEVE_TYPE);
		break;
	case SDL_MOUSEBUTTONUP:
	{
		Entity *mouse = ywPosCreateInts(event->button.x, event->button.y,
						NULL, NULL);
		yeCreateIntAt(YKEY_MOUSEUP, eve, NULL, YEVE_TYPE);
		yeCreateIntAt(event->button.button, eve, NULL, YEVE_KEY);
		yePushAt(eve, mouse, YEVE_MOUSE);
		yeDestroy(mouse);
		return eve;
	}
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
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:

		yeCreateIntAt(event->type == SDL_JOYBUTTONDOWN ?
			      YKEY_CT_DOWN : YKEY_CT_UP, eve, NULL, YEVE_TYPE);
		/* I resert 10 - 20 for directional arrow */
		yeCreateIntAt(event->jbutton.button > 10 ?
			      event->jbutton.button + 10 : event->jbutton.button,
			      eve, NULL, YEVE_KEY);
		return eve;
	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:
		yeCreateIntAt(event->type == SDL_CONTROLLERBUTTONDOWN ?
			      YKEY_CT_DOWN : YKEY_CT_UP, eve, NULL, YEVE_TYPE);
		yeCreateIntAt(event->cbutton.button, eve, NULL, YEVE_KEY);
		return eve;
	case SDL_JOYAXISMOTION:
	case SDL_CONTROLLERAXISMOTION:
	{
		static _Bool can_up[MAX_GAMEPAD][YEVE_MAX_AXIES][4];
		int ax_id = 0;
		int c_id;

		c_id = event->caxis.which;
		yeCreateIntAt(event->caxis.timestamp, eve, NULL, YEVE_TIMESTAMP);
		yeCreateIntAt(c_id, eve, NULL, YEVE_CONTROLLER_ID);
		yeCreateIntAt(abs(event->caxis.value), eve, NULL, YEVE_INTENSITY);
		switch (event->caxis.axis) {
		case SDL_CONTROLLER_AXIS_LEFTX:
			ax_id = YEVE_AX_L;
			yeCreateIntAt(ax_id, eve, NULL, YEVE_AXIS_ID);
			yeCreateIntAt(event->caxis.value < 0 ?
				      Y_LEFT_KEY: Y_RIGHT_KEY,
				      eve, NULL, YEVE_KEY);
			break;
		case SDL_CONTROLLER_AXIS_LEFTY:
			ax_id = YEVE_AX_L;
			yeCreateIntAt(YEVE_AX_L, eve, NULL, YEVE_AXIS_ID);
			yeCreateIntAt(event->caxis.value < 0 ?
				      Y_UP_KEY: Y_DOWN_KEY,
				      eve, NULL, YEVE_KEY);
			break;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			ax_id = YEVE_AX_R;
			yeCreateIntAt(YEVE_AX_R, eve, NULL, YEVE_AXIS_ID);
			yeCreateIntAt(event->caxis.value < 0 ?
				      Y_LEFT_KEY: Y_RIGHT_KEY,
				      eve, NULL, YEVE_KEY);
			break;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			ax_id = YEVE_AX_R;
			yeCreateIntAt(YEVE_AX_R, eve, NULL, YEVE_AXIS_ID);
			yeCreateIntAt(event->caxis.value < 0 ?
				      Y_UP_KEY: Y_DOWN_KEY,
				      eve, NULL, YEVE_KEY);
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			ax_id = YEVE_TRIGER_L;
			yeCreateIntAt(YEVE_TRIGER_L, eve, NULL, YEVE_KEY);
			yeCreateIntAt(YEVE_TRIGER_L, eve, NULL, YEVE_AXIS_ID);
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			ax_id = YEVE_TRIGER_R;
			yeCreateIntAt(YEVE_TRIGER_R, eve, NULL, YEVE_KEY);
			yeCreateIntAt(YEVE_TRIGER_R, eve, NULL, YEVE_AXIS_ID);
			break;
		}
		if (abs(event->caxis.value) < 8000) {
			if (ax_id < YEVE_TRIGER_L &&
			    can_up[c_id][ax_id][ywidEveKey(eve) - Y_DOWN_KEY]) {
				yeCreateIntAt(YKEY_CT_AXIS_UP, eve, NULL,
					      YEVE_TYPE);
				can_up[c_id][ax_id][ywidEveKey(eve) -
						    Y_DOWN_KEY] = 0;
			} else
				yeCreateIntAt(YKEY_NONE, eve, NULL, YEVE_TYPE);
		} else {
			yeCreateIntAt(YKEY_CT_AXIS_DOWN, eve, NULL, YEVE_TYPE);
			can_up[c_id][ax_id][ywidEveKey(eve) - Y_DOWN_KEY] = 1;
		}

		return eve;
	};
	default:
		yeCreateIntAt(YKEY_NONE, eve, NULL, YEVE_TYPE);
		return eve;
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
	/* todo: change that */
	/* SDL_SetWindowTitle(sg.pWindow, name); */
}

static int sdlChangeResolution(void)
{
	if (ywidWindowWidth < 0 || ywidWindowHight < 0) {
		DPRINT_ERR("%d x %d is not a valide resolution",
			   ywidWindowWidth, ywidWindowHight);
		return -1;
	}

	GPU_SetWindowResolution(ywidWindowWidth, ywidWindowHight);
	/* SDL_SetWindowPosition(sg.pWindow, SDL_WINDOWPOS_CENTERED, */
	/* 		      SDL_WINDOWPOS_CENTERED); */
	/* SDL_DestroyRenderer(sg.pWindow); */
	/* if (sdlRenderCreate() < 0) { */
	/* 	DPRINT_ERR("SDL is DEAD"); */
	/* 	ysdl2Destroy(); */
	/* 	return -1; */
	/* } */
	ywNeedTextureReload = 1;
	/* SDL_RenderClear(sg.pWindow); */
	return 0;
}

int    ysdl2Init(void)
{
  char path_buf[PATH_MAX];
  char ttf_path2[PATH_MAX + sizeof("/DejaVuSansMono.ttf")];
  if (type != -1)
    return type;

  int ttf_path_l = ygBinaryRootPath ?
	  strlen(ygBinaryRootPath) + sizeof("/DejaVuSansMono.ttf") :
	  sizeof("/DejaVuSansMono.ttf");
  char ttf_path[ttf_path_l];
  sprintf(ttf_path, "%s%s", ygBinaryRootPath ? ygBinaryRootPath : "",
	  "/DejaVuSansMono.ttf");

  /* Initialisation simple */
  if ((sg.pWindow = GPU_Init(ywidWindowWidth, ywidWindowHight,
			     GPU_DEFAULT_INIT_FLAGS)) == NULL) {
	  DPRINT_ERR("SDL GPU initialisation failed: (%s)\n", SDL_GetError());
	  return -1;
  }

  for (int i = 0; i < SDL_NumJoysticks() && i < 128; ++i) {
	  if (SDL_IsGameController(i)) {
		  gamepads[i] = SDL_GameControllerOpen(i);
		  if (gamepads[i] != NULL) {
			  printf("Gamepad Found: %s\n",
				 SDL_GameControllerNameForIndex(i));
		  } else {
			  DPRINT_ERR("Could not open gamepad %i: %s\n",
				     i, SDL_GetError());
		  }
	  } else {
		  joysticks[i] = SDL_JoystickOpen(i);
		  printf("try open joy %s: %s\n", SDL_JoystickNameForIndex(i),
			 !!joysticks[i] ? "[SUCESS]" : "[FAILURE]");
	  }
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


  sprintf(ttf_path2, "%sDejaVuSansMono.ttf", getcwd(path_buf, PATH_MAX));
  path_buf[PATH_MAX -1] = 0;
  if (sgSetDefaultFont(ttf_path) < 0 &&
      sgSetDefaultFont(ttf_path2) < 0 &&
      sgSetDefaultFont("/Library/Fonts/Tahoma.ttf") < 0 &&
      sgSetDefaultFont("/usr/share/fonts/liberation/LiberationMono-Regular.ttf") < 0 &&
      sgSetDefaultFont("/usr/share/fonts/TTF/DejaVuSansMono.ttf") < 0 &&
      sgSetDefaultFont("C:\\Windows\\Fonts\\constanb.ttf")) {
    DPRINT_ERR("Cannot load fonts\n");
    goto fail;
  }
  // fill the window with a black rectangle
  // SDL_Rect   rect = sg.getRect();

  /* GPU_SetBlendMode(sg.pWindow, GPU_BLEND_NORMAL); */
  GPU_Clear(sg.pWindow);
  type = ywidRegistreRender(sdlResize, SDLPollEvent, SDLWaitEvent,
			    sdlDraw, sdlChangeResolution,
			    changeWindName);
  return type;

 fail:
  TTF_Quit();
 img_fail:
  IMG_Quit();
 ttf_fail:
  GPU_FreeTarget(sg.pWindow);
  deinit_controllers();
  SDL_Quit();
  return -1;
}

static inline int sdlPrintLine(
	SDLWid *wid, char *str, SDL_Color color,
	GPU_Rect pos, int line, int alignementType,
	int lineSpace)
{
  int len = strlen(str);
  int text_width;
  GPU_Target *renderer = sg.pWindow;
  int caract_per_line = len;
  int ret = 0;
  int txth = sgGetTxtH() + lineSpace;

  if (((int)sgGetTxtW() * len) > pos.w) {
    caract_per_line = pos.w / sg.txtWidth;
  }

  pos.y += wid->rect.y + txth * line;
  pos.x += wid->rect.x;
  for (int i = 0; i < len; i += caract_per_line) {
      SDL_Surface *textSurface;
      GPU_Image* text;
      char tmp = 0;

      if ((len - i) > caract_per_line) {
	tmp = str[i + caract_per_line];
	str[i + caract_per_line] = 0;
	ret += 1;
      }

      if (pos.y >= wid->rect.y && pos.y + txth <= wid->rect.y + wid->rect.h) {
	textSurface = TTF_RenderUTF8_Solid(sgDefaultFont(), str + i, color);
	text = GPU_CopyImageFromSurface(textSurface);

	if (!text) {
	  SDL_FreeSurface(textSurface);
	  return -1;
	}
	text_width = textSurface->w;
	GPU_Rect renderQuad = { pos.x, pos.y, text_width, textSurface->h};
	SDL_FreeSurface(textSurface);

	if (alignementType == YSDL_ALIGN_CENTER)
		renderQuad.x = pos.x + ((pos.w / 2) - (text_width / 2));
	GPU_BlitRect(text, NULL, renderer,
		      &renderQuad);
	GPU_FreeImage(text);
      }
      pos.y += sgGetTxtH();
      if (tmp)
	str[i + caract_per_line] = tmp;
    }
  return ret;
}

int sdlPrintTextExt(SDLWid *wid, const char *str, SDL_Color color,
		    GPU_Rect pos, int alignementType, int lineSpace)
{
	if (!str)
		return 0;

	char **tmp = g_strsplit(str, "\n", 0);
	int ret = 0;
	int aditioner = 0;
	int end;

	for (end = 0; tmp[end]; ++end);

	for (int i = 0; i < end; ++i) {
		ret = sdlPrintLine(wid, tmp[i], color, pos,
				   i + aditioner, alignementType,
				   lineSpace);
		if (ret < 0)
			goto exit;
		aditioner += ret;
	}
exit:
	g_strfreev(tmp);
	return 0;
}

int sdlPrintText(SDLWid *wid,
		 const char *str,
		 SDL_Color color,
		 GPU_Rect pos,
		 int alignementType)
{
	return sdlPrintTextExt(wid, str, color, pos, alignementType, 0);
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
	GPU_FreeImage(txt);
}

void sdlFreeSurface(void *surface)
{
	SDL_FreeSurface(surface);
}

#define Y_SDL_TILD 1
#define Y_SDL_SPRITE 2
#define Y_SDL_COLOR 3

static GPU_Image *sdlLoasAndCachTexture(Entity *elem)
{
	const char *path = NULL;
	GPU_Image *texture = yeGetData(yeGet(elem, "$sdl-img"));
	Entity *data;

	if (texture)
		return texture;
	SDL_Surface *image;

	if (yeGet(elem, "map-tild") != NULL) {
		char *mod_path = g_strdup_printf("%s%s",
						 ygBinaryRootPath, "/modules/");

		yeStringReplace(yeGet(elem, "map-tild"),
				"YIRL_MODULES_PATH", mod_path);
		g_free(mod_path);
		yeCreateInt(Y_SDL_TILD, elem, "$sdl-type");
		path = yeGetString(yeGet(elem, "map-tild"));

	} else if (yeGet(elem, "map-sprite") != NULL) {
		char *mod_path = g_strdup_printf("%s%s", ygBinaryRootPath,
						 "/modules/");

		yeStringReplace(yeGet(elem, "map-sprite"), "YIRL_MODULES_PATH",
				mod_path);
		g_free(mod_path);
		yeCreateInt(Y_SDL_SPRITE, elem, "$sdl-type");
		path = yeGetString(yeGet(elem, "map-sprite"));
	} else if ((path = yeGetString(yeGet(elem, "map-color"))) != NULL) {
		SDL_Color *col = g_new(SDL_Color, 1);

		ywidColorFromString((char *)path, &col->r, &col->g, &col->b,
				    &col->a);
		yeCreateInt(Y_SDL_COLOR, elem, "$sdl-type");
		data = yeCreateData(col, elem, "$sdl-img");
		yeSetDestroy(data, g_free);
		return (GPU_Image *)col;
	}

	if (unlikely(!path || !(image = IMG_Load(path)))) {
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

	texture = GPU_CopyImageFromSurface(image);
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
	GPU_Image *texture;
	Entity *data;
	int w = 0, h = 0;

	image = TTF_RenderUTF8_Solid(sgDefaultFont(), str, color);
	texture = GPU_CopyImageFromSurface(image);
	w = texture->w;
	h = texture->h;
	data = yeCreateDataAt(texture, elem, "$img", YCANVAS_IMG_IDX);
	yeSetDestroy(data, sdlFreeTexture);
	ywSizeCreateAt(w, h, elem, "$size", YCANVAS_SIZE_IDX);
	data = yeCreateDataAt(image, elem, "$img-surface", YCANVAS_SURFACE_IDX);
	yeSetDestroy(data, sdlFreeSurface);

	yeGetPush(elem, resource, "$img");
	yeGetPush(elem, resource, "$size");
	yeGetPush(elem, resource, "$img-surface");
	return 0;
}

int sdlMergeText(Entity *dst, int x, int y, int w, int h, const char *str)
{
	SDL_Surface *dSurface = yeGetDataAt(dst, YCANVAS_SURFACE_IDX);
	struct SDL_Rect dr = {x, y, w, h};
	SDL_Color color = {0, 0, 0, 255};
	SDL_Surface *txt_surface = TTF_RenderUTF8_Solid(sgDefaultFont(), str,
							color);

	return SDL_BlitSurface(txt_surface, NULL, dSurface, &dr);
}

int sdlMergeRect(Entity *dst, int x, int y, int w, int h, const char *c)
{
	SDL_Surface *dSurface = yeGetDataAt(dst, YCANVAS_SURFACE_IDX);
	YBgConf cfg;
	struct SDL_Rect dr = {x, y, w, h};

	ywidColorFromString(c, &cfg.r, &cfg.g, &cfg.b, &cfg.a);
	return SDL_FillRect(dSurface, &dr, cfg.rgba);
}

int sdlMergeSurface(Entity *textSrc, Entity *srcRect,
		    Entity *textDest, Entity *destRect)
{
	SDL_Surface *sSurface = yeGetDataAt(textSrc, YCANVAS_SURFACE_IDX);
	SDL_Surface *dSurface = yeGetDataAt(textDest, YCANVAS_SURFACE_IDX);
	struct SDL_Rect sr = YRECT_MK_INIT(srcRect);
	struct SDL_Rect dr = YRECT_MK_INIT(destRect);

	if (unlikely(!dSurface))
		return -1;

	if (ywCanvasObjType(textSrc) == YCanvasString) {
		SDL_Color c = {0, 0, 0, 255};
		Entity *col = yeGet(textSrc, 3);

		if (col && yeType(col) == YSTRING) {
			ywidColorFromString(yeGetStringAt(textSrc, 3),
					    &c.r, &c.g, &c.b, &c.a);
		}

		SDL_Surface *txt_surface =
			TTF_RenderUTF8_Solid(sgDefaultFont(),
					     yeGetStringAt(textSrc, 2),
					     c);

		return SDL_BlitSurface(txt_surface, NULL, dSurface, destRect ? &dr : NULL);
	}

	if (ywCanvasObjType(textSrc) == YCanvasRect) {
		YBgConf cfg;

		ywidBgConfFill(yeGet(yeGet(textSrc, 2), 1), &cfg);
		return SDL_FillRect(dSurface, destRect ? &dr : NULL, cfg.rgba);
	}

	if (unlikely(!sSurface))
		return -1;

	return SDL_BlitScaled(sSurface, srcRect ? &sr : NULL,
			      dSurface, destRect ? &dr : NULL);
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
	if (!w || !h) {
		DPRINT_ERR("Surface size can't be 0");
		return NULL;
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


static int sdlCacheBigTexture(Entity *e, SDL_Surface *s,
				int tot_w, int tot_h)
{
	Entity *txts_h = yeCreateArrayAt(e, "$img", YCANVAS_IMG_IDX);
	GPU_Image *t;
	SDL_Surface *tmp_s;

	ywSizeCreateAt(tot_w, tot_h, e, "$size", YCANVAS_SIZE_IDX);
	for (int y = 0, h, o_tot_w = tot_w ;tot_h; tot_h -= h, y += 2048) {
		Entity *txts_w = yeCreateArray(txts_h, NULL);

		h = tot_h > 2048 ? 2048 : tot_h;

		for (int x = 0, w; tot_w; tot_w -= w, x += 2048) {
			yeAutoFree Entity *rdest;

			w = tot_w > 2048 ? 2048 : tot_w;
			rdest = ywRectCreateInts(x, y, w, h, NULL, NULL);
			tmp_s = sdlCopySurface(s, rdest);
			t = GPU_CopyImageFromSurface(tmp_s);
			Entity *d = yeCreateData(t, txts_w, NULL);
			yeSetDestroy(d, sdlFreeTexture);
		}
		tot_w = o_tot_w;
	}
	yeSetAt(e, 0, YCanvasBigTexture);
	return 0;
}

int sdlCanvasCacheImg2(Entity *elem, Entity *resource, const char *imgPath,
		       Entity *rEnt, int32_t flag)
{
	SDL_Surface *surface;
	GPU_Image *texture;
	Entity *data;
	int w, h, isText = 0;

	if (!rEnt)
		rEnt = yeGet(elem, "img-src-rect") ? :
			yeGet(resource, "img-src-rect");
	if (!imgPath) {
		surface = yeGetData(yeGet(resource, "$img-surface"));
		isText = 1;
	} else {
		if (!g_file_test(imgPath, G_FILE_TEST_EXISTS)) {
			char *cd = get_current_dir_name();

			DPRINT_ERR("no sure file %s(current dir: %s)", imgPath,
				   cd);
			free(cd); // but who still use CD today ?
			return -1;
		}
		surface = IMG_Load(imgPath);
		if (surface->format->BitsPerPixel == 8) {
			SDL_Surface *tmp = surface;
			SDL_PixelFormat *f = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
			surface = SDL_ConvertSurface(surface, f, 0);
			SDL_FreeFormat(f);
			SDL_FreeSurface(tmp);
		}

	}

	if (unlikely(!surface)) {
		DPRINT_ERR("fail to load %s", imgPath);
		return -1;
	}

	if (rEnt) {
		SDL_Surface *tmpSurface = surface;

		surface = sdlCopySurface(tmpSurface, rEnt);
		if (!surface) {
			return -1;
		}
		if (imgPath)
			SDL_FreeSurface(tmpSurface);
		/* trick to sdlFreeSurface anyway */
		imgPath = "";
	}
	if (!(flag & YSDL_CACHE_IMG_NO_TEXTURE)) {
		if (!resource &&
		    (surface->w > 2048 || surface->h > 2048)) {
			if (sdlCacheBigTexture(elem, surface,
					       surface->w, surface->h) < 0) {
				DPRINT_ERR("fail to create a texture from surface"
					   "(size: %d %d): %s",
					   surface->w, surface->h, SDL_GetError());
				goto free_surface;
			}
			goto store_surface;
		}
		texture = GPU_CopyImageFromSurface(surface);
		w = texture->w;
		h = texture->h;
		data = yeCreateDataAt(texture, elem, "$img", YCANVAS_IMG_IDX);
		yeSetDestroy(data, sdlFreeTexture);
		ywSizeCreateAt(w, h, elem, "$size", YCANVAS_SIZE_IDX);
		if (!isText) {
			yeGetPush(elem, resource, "$img");
			yeGetPush(elem, resource, "$size");
		}
	}
store_surface:
	data = yeCreateDataAt(surface, elem, "$img-surface",
			      YCANVAS_SURFACE_IDX);
	/* if no img path a texture was use */
	if (imgPath) {
		yeSetDestroy(data, sdlFreeSurface);
	}
	if (!isText)
		yeGetPush(elem, resource, "$img-surface");
	return 0;
free_surface:
	if (imgPath)
		sdlFreeSurface(surface);
	return -1;
}

void sdlCanvasCacheVoidTexture(Entity *obj, Entity *size)
{
	SDL_Surface *surface;
	Entity *data;

	surface = SDL_CreateRGBSurface(0, ywSizeW(size), ywSizeH(size),
				       32, 0, 0, 0, 0);
	data = yeCreateDataAt(surface, obj, "$img-surface",
			      YCANVAS_SURFACE_IDX);
	yeAttach(obj, size, YCANVAS_SIZE_IDX, "$size", 0);
	yeSetDestroy(data, sdlFreeSurface);
	ywSizePrint(ywCanvasObjSize(NULL, obj));
}

void sdlCanvasCacheBicolorImg(Entity *elem, uint8_t *img, Entity *info)
{
	Entity *size = yeGet(info, 0);
	int w = ywSizeW(size), h = ywSizeH(size);
	SDL_Surface *surface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
	int end = w * h;
	uint32_t *pixels = surface->pixels;
	uint32_t bg = yeGetIntAt(info, 1);
	uint32_t fg = yeGetIntAt(info, 2);
	GPU_Image *texture;
	Entity *data;

	assert(w);
	assert(h);
	for (int i = 0; i < end; ++i) {
		pixels[i] = img[i] ? fg : bg;
	}
	texture = GPU_CopyImageFromSurface(surface);
	data = yeCreateDataAt(texture, elem, "$img", YCANVAS_IMG_IDX);
	yeSetDestroy(data, sdlFreeTexture);
	ywSizeCreateAt(w, h, elem, "$size", YCANVAS_SIZE_IDX);
	data = yeCreateDataAt(surface, elem, "$img-surface",
			      YCANVAS_SURFACE_IDX);
	yeSetDestroy(data, sdlFreeSurface);
	return;
}

int sdlCanvasCacheImg(Entity *elem, Entity *resource, const char *imgPath,
		      Entity *rEnt)
{
	return sdlCanvasCacheImg2(elem, resource, imgPath, rEnt, 0);
}

int sdlCanvasCacheTexture(Entity *state, Entity *elem)
{
	int type = ywCanvasObjType(elem);
	Entity *resource;
	Entity *texture;
	const char *txt;

	if (type == YCanvasRect) {
		return 0;
	} else if (ywCanvasObjType(elem) == YCanvasString) {
		Entity *str = yeGet(elem, 2);
		int w = 0, h;

		h = yuiStrCountCh(yeGetString(str), '\n', &w);
		w = (w + 1) * (sg.txtWidth);
		h = (h + 1) * (sgGetFontSize() + 2);

		ywSizeCreateAt(w, h, elem, "$size", YCANVAS_SIZE_IDX);
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

	Entity *tmp = yeGet(resource, "$img");
	if (tmp && !ywNeedTextureReload) {
		yePushAt(elem, tmp, YCANVAS_IMG_IDX);
		yeRenameIdxStr(elem, YCANVAS_IMG_IDX, "$img");
		yeGetPush(resource, elem, "$size");
		yeGetPush(resource, elem, "$img-surface");
		return 0;
	}

	texture = yeGet(resource, "texture");
	if (texture) {
		int ret;
		Entity *imgSrcRect;

		sdlCanvasCacheImg(resource, texture, NULL, NULL);
		imgSrcRect = yeGet(resource, "img-src-rect");
		if (imgSrcRect)
			yeIncrRef(imgSrcRect);
		yeRemoveChild(resource, imgSrcRect);
		ret = sdlCanvasCacheImg(elem, resource, NULL, NULL);
		yePushBack(resource, imgSrcRect, "img-src-rect");
		yeDestroy(imgSrcRect);
		return ret;
	}


	txt = yeGetStringAt(resource, "img");
	if (txt)
		return sdlCanvasCacheImg(elem, resource, txt, NULL);
	txt = yeGetStringAt(resource, "text");
	if (txt)
		return sdlCanvasCacheText(state, elem, resource, txt);
	return -1;
}

static void sdlCanvasAplyModifier(Entity *img, GPU_Rect *dst,
				  GPU_Rect **src,
				  double *rotation, GPU_BatchFlagEnum *flip)
{
	Entity *mod = ywCanvasObjMod(img);

	if (!mod)
		return;
	*rotation = yeGetFloat(yeGet(mod, YCanvasRotate));
	return;
}

static int sdlCanvasRendImg(YWidgetState *state, SDLWid *wid, Entity *img,
			    Entity *cam, Entity *wid_pix)
{
	Entity *s = ywCanvasObjSize(state->entity, img);
	Entity *p = ywCanvasObjPos(img);
	GPU_Rect *sd = NULL;
	assert(p);
	assert(s);
	GPU_Rect rd = { ywPosXDirect(p) - ywPosX(cam),
			ywPosYDirect(p) - ywPosY(cam),
			ywSizeWDirect(s), ywSizeHDirect(s) };
	double rotation = 0;
	GPU_BatchFlagEnum flip = 0;
	GPU_Image *t = NULL;


	if (rd.x + rd.w < 0 || rd.y + rd.h < 0 || rd.y > ywRectHDirect(wid_pix)
	    || rd.x > ywRectWDirect(wid_pix)) {
		return 0;
	}
	t = yeGetData(yeGet(img, YCANVAS_IMG_IDX));
	if (!t || ywNeedTextureReload) {
		sdlCanvasCacheTexture(state->entity, img);
		t = yeGetData(yeGet(img, YCANVAS_IMG_IDX));
		if (unlikely(!t))
			return -1;
	}

	rd.x += ywRectX(wid_pix);
	rd.y += ywRectY(wid_pix);
	sdlCanvasAplyModifier(img, &rd, &sd, &rotation, &flip);
	GPU_BlitRectX(t, sd, sg.pWindow, &rd, rotation,
		      rd.x + (rd.w / 2),
		      rd.y + (rd.h / 2),
		      flip);
	free(sd);
	return 0;
}

uint32_t sdlCanvasPixInfo(Entity *obj, int x, int y)
{
  if (!obj)
    return 0;
  SDL_Surface *surface = yeGetDataAt(obj, YCANVAS_SURFACE_IDX);
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
/* I guess if Alpha is store at the begin, we're fuck */
	ret.rgba[3] = 255;

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

static int sdlCanvasRendBigImg(YWidgetState *state, SDLWid *wid,
			       Entity *obj, Entity *cam, Entity *wid_pix)
{
	Entity *p = ywCanvasObjPos(obj);
	int x0 = ywPosXDirect(p) - ywPosX(cam);
	int y0 = ywPosYDirect(p) - ywPosY(cam);
	int threshold_y = 0;
	SDL_Surface *surface = yeGetDataAt(obj, "$img-surface");
	Entity *s = ywCanvasObjSize(state->entity, obj);
	Entity *txts_h = yeGet(obj, YCANVAS_IMG_IDX);
	int wpercent = 100 * ywSizeW(s) / surface->w ;
	int wadd = 2048 * wpercent / 100;
	int hpercent = 100 * ywSizeH(s) / surface->h;
	int hadd = 2048 * hpercent / 100;

	YE_FOREACH(txts_h, txts_w) {
		int threshold_x = 0;
		YE_FOREACH(txts_w, texture) {
			GPU_Image *t = yeGetData(texture);
			int w, h;
			int x = x0 + threshold_x;
			int y = y0 + threshold_y;

			w = t->w;
			h = t->h;

			w = yuiPercentOf(w, wpercent);
			h = yuiPercentOf(h, hpercent);

			GPU_BlitRect(t, NULL, sg.pWindow,
				     &(GPU_Rect){x, y, w, h});
			threshold_x += wadd;
		}
		threshold_y += hadd;
	}
	return 0;
}

int sdlCanvasRendObj(YWidgetState *state, SDLWid *wid, Entity *obj,
		     Entity *cam, Entity *wid_pix)
{
	int type = yeGetIntAt(obj, 0);

	if (type == YCanvasBigTexture)
		return sdlCanvasRendBigImg(state, wid, obj, cam, wid_pix);
	if (type == YCanvasResource || type == YCanvasImg ||
	    type == YCanvasTexture || type == YCanvasBicolorImg)
		return sdlCanvasRendImg(state, wid, obj, cam, wid_pix);

	Entity *p = ywCanvasObjPos(obj);
	SDL_Color c = {0, 0, 0, 255};
	Entity *s = ywCanvasObjSize(state->entity, obj);
	GPU_Rect rect = {ywPosX(p) - ywPosX(cam), ywPosY(p) - ywPosY(cam),
			 ywSizeW(s), ywSizeH(s)};

	if (type == YCanvasRect) {
		YBgConf cfg;

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
		Entity *col = yeGet(obj, 3);

		if (col && yeType(col) == YSTRING) {
			ywidColorFromString((char *)yeGetStringAt(obj, 3),
					    &c.r, &c.g, &c.b, &c.a);
		}
		sdlPrintText(wid, yeGetStringAt(obj, 2), c,
			     rect, YSDL_ALIGN_LEFT);
		return 0;
	}
	return -1;
}

int sdlDisplaySprites(YWidgetState *state, SDLWid *wid,
		      int x, int y, Entity *mapElem,
		      int w, int h, int thresholdX,
		      int thresholdY, Entity *mod)
{
	GPU_Rect DestR = {x * w + wid->rect.x + thresholdX,
			  y * h + wid->rect.y + thresholdY,
			  w, h};
	GPU_Image *texture;
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
			GPU_Rect srcR = {0, 0, 0, 0};
			GPU_Rect *srcRP = NULL;

			if (type != Y_SDL_COLOR) {
				srcR.w = texture->w;
				srcR.h = texture->h;
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
			}

			if (unlikely(mod) && !yeGetIntAt(mod, 0)) {
				DestR.x += yeGetIntAt(mod, 1);
				DestR.w -= yeGetIntAt(mod, 2);
				srcR.x += yuiPercentOf(srcR.w,
						       yeGetIntAt(mod, 3));
				srcR.w -= yuiPercentOf(srcR.w,
						       yeGetIntAt(mod, 4));
				srcRP = &srcR;
			}

			if (type == Y_SDL_COLOR)
				sdlDrawRect(NULL, DestR,
					    *((SDL_Color *)texture));
			else
				GPU_BlitRect(texture, srcRP, sg.pWindow, &DestR);
		} else {
			Entity *rinfo = yeGet(mapElem, "rend_info");
			GPU_Rect srcR = YRECT_MK_INIT(yeGet(rinfo, "src"));
			GPU_Rect *rp = rinfo ? &srcR : NULL;
			Entity *pt = yeGet(rinfo, "threshold");
			int xt = ywPosX(pt);
			int yt = ywPosY(pt);

			if (!rinfo) {
				DestR.w = texture->w;
				DestR.h = texture->h;
			}
			else {
				DestR.w = srcR.w;
				DestR.h = srcR.h;
			}
			DestR.y = y * h - (DestR.h - h) + wid->rect.y + yt;
			DestR.x += xt;
			GPU_BlitRect(texture, rp, sg.pWindow, &DestR);
		}
	}
	return 0;
}
