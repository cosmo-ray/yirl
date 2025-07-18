/*
**Copyright (C) 2015-2025 Matthias Gatto
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
#include <assert.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "sdl-internal.h"
#include "canvas-sdl.h"
#include "gpu-compat.h"
#include "sdl-geometry.h"
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

SDL_Rect getRect(void)
{
	int w, h;
	SDL_GetWindowSize(sg.window, &w, &h);
	SDL_Rect ret = {0, 0, w, h};

	return ret;
}

TTF_Font *sgDefaultFont(void)
{
	return sg.font;
}

int ysdl2WindowMode(void)
{
#ifdef USING_EMCC
	return 0;
#else
	if (SDL_SetWindowFullscreen(sg.window, 0) != 0) {
            DPRINT_ERR("Error disabling fullscreen: %s\n", SDL_GetError());
	    return -1;
        }
	return 0;
#endif
}

int ysdl2FullScreen(void)
{
#ifdef USING_EMCC
	return 0;
#else
	int r;

	r = SDL_SetWindowFullscreen(sg.window, SDL_WINDOW_FULLSCREEN);
	if (r != 0) {
		DPRINT_ERR("Error enbling fullscreen: %s\n", SDL_GetError());
	}
	return r;
#endif
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
	SDL_RenderPresent(sg.pWindow);
	return 0;
}

/**
 * @return the SDLWid inside the magin
 */
SDLWid *sddComputeMargin(YWidgetState *w, SDLWid *swid)
{
	Entity *e = w->entity;
	static SDLWid marged_wid;
	Entity *m = yeGet(e, "margin");
	Entity *s_e = yeGet(m, "size");
	Entity *c_e = yeGet(m, "color");
	double s;
	const SDL_Rect *or = &swid->rect;
	SDL_Rect *dr = &marged_wid.rect;
	YBgConf cfg;

	if (!m)
		return swid;

	if (!s_e) {
		return swid;
	}

	s = yeGetInt(s_e);

	if (s * 2 >= or->w || s * 2 >= or->h)
		return swid;

	if (!c_e)
		goto out;

	if (ywidBgConfFill(c_e, &cfg) < 0)
		return swid;

	sdlDrawRect(swid, (SDL_Rect){0, 0, or->w, s},
		    SDL_COLOR_FROM_YBGCONF(cfg));
	sdlDrawRect(swid, (SDL_Rect){0, 0 + s, s, or->h - s * 2},
		    SDL_COLOR_FROM_YBGCONF(cfg));
	sdlDrawRect(swid, (SDL_Rect){or->w - s, s, s, or->h - s * 2},
		    SDL_COLOR_FROM_YBGCONF(cfg));
	sdlDrawRect(swid, (SDL_Rect){0, or->h - s, or->w, s},
		    SDL_COLOR_FROM_YBGCONF(cfg));

out:
	marged_wid.wid = w;
	dr->x = or->x + s;
	dr->y = or->y + s;
	dr->h = or->h - s * 2;
	dr->w = or->w - s * 2;
	return &marged_wid;
}

void	sdlDrawRect(SDLWid *swid, SDL_Rect rect, SDL_Color color)
{
	if (swid) {
		rect.y += swid->rect.y;
		rect.x += swid->rect.x;
	}
	SDL_SetRenderDrawColor(sg.pWindow, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(sg.pWindow, &rect);
}


static void	sdlDrawRect2(SDLWid *swid, SDL_Rect rect, SDL_Color color, int filled, float radius)
{
	if (swid) {
		rect.y += swid->rect.y;
		rect.x += swid->rect.x;
	}

	if (radius > 0) {
		if (filled)
			RendRectangleRoundFilled(sg.pWindow, rect, radius, color);
		else
			RendRectangleRound(sg.pWindow, rect, radius, color);
	} else {
		SDL_SetRenderDrawColor(sg.pWindow, color.r, color.g, color.b, color.a);

		if (filled)
			SDL_RenderFillRect(sg.pWindow, &rect);
		else
			SDL_RenderDrawRect(sg.pWindow, &rect);
	}
}

static void	sdlDrawPolyFilled(int nb_vertex, SDL_Vertex vertex[static nb_vertex])
{
	SDL_RenderGeometry(sg.pWindow, NULL, vertex, nb_vertex, NULL, 0);
}

static void     sdlDrawTriangle(float x1, float y1, float x2,
				float y2, float x3, float y3,
				SDL_Color c, int filled)
{
	if (!filled) {
		SDL_SetRenderDrawColor(sg.pWindow, c.r, c.g, c.b, c.a);
		SDL_RenderDrawLine(sg.pWindow, x1, y1, x2, y2);
		SDL_RenderDrawLine(sg.pWindow, x2, y2, x3, y3);
		SDL_RenderDrawLine(sg.pWindow, x3, y3, x1, y1);
	} else {
		// Define the vertices for the filled triangle
		SDL_Vertex verts[3] = {
			{ {x1, y1}, c , {0,0}},
			{ {x2, y2}, c , {0,0} },
			{ {x3, y3}, c , {0,0} }
		};

		// Render the filled triangle
		SDL_RenderGeometry(sg.pWindow, NULL, verts, 3, NULL, 0);
	}
}

static void     sdlDrawLine(float x1, float y1, float x2,
			    float y2, SDL_Color c)
{
	SDL_SetRenderDrawColor(sg.pWindow, c.r, c.g, c.b, c.a);
	SDL_RenderDrawLine(sg.pWindow, x1, y1, x2, y2);
}


static void	sdlDrawCircle(SDL_Rect rect, SDL_Color color, int filled)
{
	RenderCircle(sg.pWindow, NULL, color, rect.x, rect.y, rect.w, filled);
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
		SDL_Texture *texture = SDL_CreateTextureFromSurface(sg.pWindow, img);
		if (cfg->flag & YBG_FIT_TO_SCREEN) {
			SDL_Rect drect = swid->rect;

			if (cfg->flag & YBG_FIT_TO_SCREEN_H) {
				drect.w = img->w * drect.h / img->h;
				drect.x += (swid->rect.w / 2) - drect.w / 2;
			} else {
				drect.h = img->h * drect.w / img->w;
				drect.y += (swid->rect.h / 2) - drect.h / 2;
			}
			SDL_RenderCopy(sg.pWindow, texture, NULL, &drect);
		} else {
			SDL_RenderCopy(sg.pWindow, texture, NULL, &swid->rect);
		}
		SDL_FreeSurface(img);
		SDL_DestroyTexture(texture);
		return 0;
	}
	return -1;
}

int ywidShowBG(Entity *wid, SDLWid *sdl_wid)
{
	Entity *bg = yeGet(wid, "background");
	YBgConf cfg;

	if (yeType(bg) == YARRAY) {
		printf("bg is an array\n");
		yePrint(bg);
		YE_FOREACH(bg, this) {
			if (ywidBgConfFill(this, &cfg) >= 0)
				sdlFillBg(sdl_wid, &cfg);
		}
	} else {
		if (ywidBgConfFill(bg, &cfg) >= 0)
			sdlFillBg(sdl_wid, &cfg);
	}
	return 0;
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
	SDL_DestroyRenderer(sg.pWindow);
	sg.pWindow = NULL;
	SDL_DestroyWindow(sg.window);
	sg.window = NULL;
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
	case SDLK_BACKSPACE:
		return '\b';
	default:
		return -1;
	}
}

static inline Entity *mouse_create_pos(int bt_x, int bt_y)
{
	if (getRect().w == ywidWindowWidth) {
		return ywPosCreateInts(bt_x,
				       bt_y,
				       NULL, NULL);
	} else {
		int w = bt_x * ywidWindowWidth / getRect().w;
		int h = bt_y * ywidWindowHight / getRect().h;
		return ywPosCreateInts(w, h, NULL, NULL);
	}
}

static inline Entity *SDLConvertEvent(SDL_Event* event, int *is_repeat)
{
	Entity *eve = yeCreateArray(NULL, NULL);

	if (!event)
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
		if (event->key.repeat > 0) {
			*is_repeat = 1;
			goto cancel;
		}
		yeCreateIntAt(YKEY_DOWN, eve, NULL, YEVE_TYPE);
		break;
	case SDL_MOUSEBUTTONUP:
	{
		Entity *mouse = mouse_create_pos(event->button.x,
						 event->button.y);
		yeCreateIntAt(YKEY_MOUSEUP, eve, NULL, YEVE_TYPE);
		yeCreateIntAt(event->button.button, eve, NULL, YEVE_KEY);
		yePushAt(eve, mouse, YEVE_MOUSE);
		yeDestroy(mouse);
		return eve;
	}
	case SDL_MOUSEBUTTONDOWN:
	{
		Entity *mouse = mouse_create_pos(event->button.x,
						 event->button.y);
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
		Entity *mouse = mouse_create_pos(event->button.x,
						 event->button.y);

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
cancel:
	yeDestroy(eve);
	return NULL;
}

static Entity *SDLWaitEvent(void)
{
	static SDL_Event event;
	int is_repeat = 0;
	Entity *ret;

again:
	if (!SDL_WaitEvent(&event))
		return NULL;
	ret = SDLConvertEvent(&event, &is_repeat);
	if (is_repeat) {
		is_repeat = 0;
		goto again;
	}
	return ret;
}

static Entity *SDLPollEvent(void)
{
	static SDL_Event event;
	int is_repeat = 0;
	Entity *ret;

again:
	if (!SDL_PollEvent(&event))
		return NULL;
	ret = SDLConvertEvent(&event, &is_repeat);
	if (is_repeat) {
		is_repeat = 0;
		goto again;
	}
	return ret;
}

static void changeWindName(const char *name)
{
	/* todo: change that */
	/* SDL_SetWindowTitle(sg.pWindow, name); */
}

static int sdlChangeResolution(void)
{
#ifdef USING_EMCC
	return 0;
#else

	if (ywidWindowWidth < 0 || ywidWindowHight < 0) {
		DPRINT_ERR("%d x %d is not a valide resolution",
			   ywidWindowWidth, ywidWindowHight);
		return -1;
	}

	SDL_SetWindowSize(sg.window, ywidWindowWidth, ywidWindowHight);
	/* SDL_SetWindowPosition(sg.window, SDL_WINDOWPOS_CENTERED, */
	/* 		      SDL_WINDOWPOS_CENTERED); */
	/* SDL_DestroyRenderer(sg.pWindow); */
	/* if (sdlRenderCreate() < 0) { */
	/* 	DPRINT_ERR("SDL is DEAD"); */
	/* 	ysdl2Destroy(); */
	/* 	return -1; */
	/* } */
	ywNeedTextureReload = 1;
	/* SDL_RendererClear(sg.pWindow); */
	return 0;
#endif
}

//DejaVuSansMono.ttf
#define DEFAULTPOLICE "sazanami-mincho.ttf"

int    ysdl2Init(void)
{
  char path_buf[PATH_MAX];
  char ttf_path2[PATH_MAX + sizeof("/" DEFAULTPOLICE)];
  if (type != -1)
    return type;

  int ttf_path_l = ygBinaryRootPath ?
	  strlen(ygBinaryRootPath) + sizeof("/" DEFAULTPOLICE) :
	  sizeof("/" DEFAULTPOLICE);
  char ttf_path[ttf_path_l];
  sprintf(ttf_path, "%s%s", ygBinaryRootPath ? ygBinaryRootPath : "",
	  "/" DEFAULTPOLICE);

  /* Initialize the SDL library */
#ifdef USING_EMCC
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
	  DPRINT_ERR("Couldn't initialize SDL: %s\n", SDL_GetError());
	  return -1;
  }
#else
  if (SDL_InitSubSystem(SDL_INIT_EVERYTHING) < 0) {
	  DPRINT_ERR("Couldn't initialize SDL: %s\n", SDL_GetError());
	  return -1;
  }
#endif


  sg.window = SDL_CreateWindow("YIRL", SDL_WINDOWPOS_CENTERED,
			       SDL_WINDOWPOS_CENTERED, ywidWindowWidth, ywidWindowHight,
			       SDL_WINDOW_SHOWN);
  if (!sg.window) {
	  printf("Failed to create window: %s\n", SDL_GetError());
	  return -1;
  }

  /* Initialisation simple */
  if ((sg.pWindow = SDL_CreateRenderer(sg.window, -1, SDL_RENDERER_ACCELERATED |
				     SDL_RENDERER_PRESENTVSYNC)) == NULL) {
	  DPRINT_ERR("SDL GPU initialisation failed: (%s)\n", SDL_GetError());
	  return -1;
  }
  SDL_SetRenderDrawBlendMode(sg.pWindow, SDL_BLENDMODE_BLEND);

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

#ifdef USING_EMCC
#define SDL_INIT_FLAGS
#else
#define SDL_INIT_FLAGS IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF
  // Simple check of the Flags
  if(!IMG_Init(SDL_INIT_FLAGS)) {
	  DPRINT_ERR("SDL_image could not initialize! SDL_image Error: %s\n",
		     IMG_GetError());
	  goto img_fail;
  }
#endif
#undef SDL_INIT_FLAGS


  sprintf(ttf_path2, "%s" DEFAULTPOLICE, getcwd(path_buf, PATH_MAX));
  path_buf[PATH_MAX -1] = 0;
  if (sgSetDefaultFont(ttf_path) < 0 &&
      sgSetDefaultFont(ttf_path2) < 0 &&
      sgSetDefaultFont("/Library/Fonts/Tahoma.ttf") < 0 &&
      sgSetDefaultFont("/usr/share/fonts/liberation/LiberationMono-Regular.ttf") < 0 &&
      sgSetDefaultFont("/usr/share/fonts/TTF/" DEFAULTPOLICE) < 0 &&
      sgSetDefaultFont("C:\\Windows\\Fonts\\constanb.ttf")) {
    DPRINT_ERR("Cannot load fonts\n");
    goto fail;
  }

// fill the window with a black rectangle
  // SDL_Rect   rect = sg.getRect();

  /* SDL_SetBlendMode(sg.pWindow, SDL_BLEND_NORMAL); */
  SDL_Clear(sg.pWindow, NULL);
  type = ywidRegistreRender(sdlResize, SDLPollEvent, SDLWaitEvent,
			    sdlDraw, sdlChangeResolution,
			    changeWindName);
  return type;

 fail:
  TTF_Quit();
 img_fail:
  IMG_Quit();
 ttf_fail:
  SDL_DestroyRenderer(sg.pWindow);
  SDL_DestroyWindow(sg.window);
  deinit_controllers();
  SDL_Quit();
  return -1;
}

static SDL_Surface *makeHeadacheSurface(Entity *map, Entity *info)
{
	const char *map_pixiels = yeGetString(map);
	Entity *pix_mapping = yeGet(info, "mapping");
	Entity *pix_per_char = yeGet(info, "pix_per_char");
	Entity *size = yeGet(info, "size");
	int char_map[127] = {0}; // ascii table
	SDL_Surface *surface = SDL_CreateRGBSurface(
		0, ywSizeW(pix_per_char) * ywSizeW(size),
		ywSizeH(pix_per_char) * ywSizeH(size), 32,
		0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	uint32_t *pixels = surface->pixels;

	for (int i = 0, map_len = yeLen(pix_mapping); i < map_len; ++i) {
		int k = *yeGetKeyAt(pix_mapping, i);

		char_map[k] = yeGetIntAt(pix_mapping, i);
	}

	int size_h = ywSizeH(size);
	int size_w = ywSizeW(size);
	for (int iy = 0, dy = 0; iy < size_h;
	     iy += 1, dy += ywSizeH(pix_per_char)) {
		for (int ix = 0, dx = 0; ix < size_w;
		     ix += 1, dx += ywSizeW(pix_per_char)) {
			int pix_pos = dx + dy *
				ywSizeW(pix_per_char) *
				ywSizeW(size);
			int pix = map_pixiels[ix + iy * ywSizeW(size)];
			for (int  i = 0; i < ywSizeH(pix_per_char); ++i) {
				for (int  i = 0; i < ywSizeW(pix_per_char); ++i)
					pixels[pix_pos++] = char_map[pix];
				pix_pos += ywSizeW(pix_per_char) *
					ywSizeW(size) - ywSizeW(pix_per_char);

			}
		}
	}
	return surface;
}

static SDL_Surface *mk_print_surface(char *str, SDL_Color color)
{
	if (!*str)
		return 0;
	SDL_Surface *textSurface = TTF_RenderUTF8_Solid(sgDefaultFont(), str, color);

	return textSurface;
}

static int do_print(SDL_Surface **surfaces, int surfaces_cnt, SDL_Rect pos,
		    int alignementType, SDL_Renderer *renderer)
{
	int ret = 0;
	int mod_x = 0;
	SDL_Rect renderQuad = { pos.x + mod_x, pos.y, 0, 0};

	if (alignementType == YSDL_ALIGN_CENTER) {
		int tot_w = 0;

		for (int i = 0; i < surfaces_cnt; ++i) {
			tot_w += surfaces[i]->w;
		}
		renderQuad.x = pos.x + ((pos.w / 2) - (tot_w / 2));
	}
	for (int i = 0; i < surfaces_cnt; ++i) {
		SDL_Texture *text = SDL_CreateTextureFromSurface(sg.pWindow, surfaces[i]);

		if (!text) {
			ret = -1;
			goto out;
		}

		renderQuad.w = surfaces[i]->w;
		renderQuad.h = surfaces[i]->h;
		SDL_RenderCopy(renderer, text, NULL,
			     &renderQuad);
		SDL_DestroyTexture(text);
		renderQuad.x += surfaces[i]->w;
	}
out:
	for (int i = 0; i < surfaces_cnt; ++i)
		SDL_FreeSurface(surfaces[i]);
	return ret;
}

static int sdlPrintLine(
	SDLWid *wid, char *str, SDL_Color *color,
	SDL_Rect pos, int line, int alignementType,
	int lineSpace, SDL_Color orig_color)
{
	int len = strlen(str);
	SDL_Renderer *renderer = sg.pWindow;
	int caract_per_line = len;
	int ret = 0;
	int txth = sgGetTxtH() + lineSpace;
	SDL_Surface *surfaces_array[128];
	if (((int)sgGetTxtW() * len) > pos.w) {
		caract_per_line = pos.w / sg.txtWidth;
	}

	pos.y += wid->rect.y + txth * line;
	pos.x += wid->rect.x;
	for (int i = 0, minus_cpl; i < len;
	     i += caract_per_line - minus_cpl) {
		char *str_tmp;
		char tmp = 0;
		int surfaces_cnt  = 0;

		minus_cpl = 0;
		if ((len - i) > caract_per_line) {
			tmp = str[i + caract_per_line];
			str[i + caract_per_line] = 0;
			ret += 1;
		}

	find_mod:
		str_tmp = strchr(str + i, '\33');
		if (str_tmp) {
			*str_tmp = 0;
		}

		if (pos.y >= wid->rect.y &&
		    pos.y + txth <= wid->rect.y + wid->rect.h
		    && strlen(str + i)) {
			SDL_Surface *s = mk_print_surface(str + i, *color);
			if (!s)
				return -1;
			surfaces_array[surfaces_cnt++] = s;
		}

		if (str_tmp) {
			int mod_len;

			if (tmp)
				str[i + caract_per_line - minus_cpl] = tmp;
			minus_cpl += str_tmp - (str + i);
			*str_tmp = '\33';
			++str_tmp;
			if (*str_tmp == '[') {

				++str_tmp;
				if (*str_tmp == '0' && str_tmp[1] == 'm') {
					*color = orig_color;
					str_tmp += 2;
				} else if (*str_tmp == '3' && isdigit(str_tmp[1])) {
					int as_int;

					++str_tmp; // skip 3

					as_int = *str_tmp - '0';
					color->r = 255 * !!(as_int & 1);
					color->g = 255 * !!(as_int & 2);
					color->b = 255 * !!(as_int & 4);
					color->a = 255;
					++str_tmp; // skipp next digit
					if (*str_tmp == 'm') {
						++str_tmp;
					}
				} else {
					while (isdigit(*str_tmp)) {
						++str_tmp;
					}
					if (*str_tmp == 'm') {
						++str_tmp;
					}
				}
			}
			mod_len = (str_tmp - (str + i));
			i += mod_len;
			if (len > i + caract_per_line) {
				tmp = str[i + caract_per_line - minus_cpl];
				str[i + caract_per_line  - minus_cpl] = 0;
			} else {
				tmp = 0;
			}
			goto find_mod;
		}
		int ret = do_print(surfaces_array, surfaces_cnt, pos,
				   alignementType, renderer);
		if (ret < 0)
			return -1;


		pos.y += sgGetTxtH();
		if (tmp)
			str[i + caract_per_line  - minus_cpl] = tmp;
	}
	return ret;
}

int sdlPrintTextExt(SDLWid *wid, const char *str, SDL_Color color,
		    SDL_Rect pos, int alignementType, int lineSpace)
{
	if (!str)
		return 0;

	char *tmp = strdup(str);
	char *otmp = tmp;
	int ret = 0;
	int line_cnt = 0;
	SDL_Color ocolor = color;


again:
	{
		char *next = strchr(tmp, '\n');

		if (next)
			*next = 0;
		ret = sdlPrintLine(wid, tmp, &color, pos,
				   line_cnt, alignementType,
				   lineSpace, ocolor);
		if (ret < 0)
			goto exit;
		line_cnt += ret;

		if (next) {
			tmp = next + 1;
			++line_cnt;
			goto again;
		}
	}
exit:
	free(otmp);
	return 0;
}

int sdlPrintText(SDLWid *wid,
		 const char *str,
		 SDL_Color color,
		 SDL_Rect pos,
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

	free(swid);
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
	Entity *data = yeGet(elem, "$sdl-img");
	SDL_Texture *texture = yeGetData(data);

	if (texture)
		return texture;
	if (unlikely(data)) {
		yeRemoveChild(elem, data);
		data = NULL;
	}
	SDL_Surface *image;

	if (yeGet(elem, "map-tild") != NULL) {
		char *mod_path = y_strdup_printf("%s%s",
						 ygBinaryRootPath, "/modules/");

		yeStringReplace(yeGet(elem, "map-tild"),
				"YIRL_MODULES_PATH", mod_path);
		free(mod_path);
		yeCreateInt(Y_SDL_TILD, elem, "$sdl-type");
		path = yeGetString(yeGet(elem, "map-tild"));

	} else if (yeGet(elem, "map-sprite") != NULL) {
		char *mod_path = y_strdup_printf("%s%s", ygBinaryRootPath,
						 "/modules/");

		yeStringReplace(yeGet(elem, "map-sprite"), "YIRL_MODULES_PATH",
				mod_path);
		free(mod_path);
		yeCreateInt(Y_SDL_SPRITE, elem, "$sdl-type");
		path = yeGetString(yeGet(elem, "map-sprite"));
	} else if (yeGet(elem, "map-pixels") != NULL) {
		/* need to be made into a subfunction usable by canvas */
		Entity *map_pixiels = yeGet(elem, "map-pixels");
		Entity *info = yeGet(elem, "map-pixels-info");

		yeReCreateInt(Y_SDL_TILD, elem, "$sdl-type");
		image = makeHeadacheSurface(map_pixiels, info);

		goto finish;
	} else if ((path = yeGetString(yeGet(elem, "map-color"))) != NULL) {
		SDL_Color *col = y_new(SDL_Color, 1);

		ywidColorFromString((char *)path, &col->r, &col->g, &col->b,
				    &col->a);
		yeCreateInt(Y_SDL_COLOR, elem, "$sdl-type");
		data = yeCreateData(col, elem, "$sdl-img");
		yeSetDestroy(data, free);
		return (SDL_Texture *)col;
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

finish:

	texture = SDL_CreateTextureFromSurface(sg.pWindow, image);
	data = yeCreateData(texture, elem, "$sdl-img");
	yeSetDestroy(data, sdlFreeTexture);
	SDL_FreeSurface(image);
	return texture;
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
	texture = SDL_CreateTextureFromSurface(sg.pWindow, image);
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);
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

	if (cfg.a == 255)
		return SDL_FillRect(dSurface, &dr, cfg.rgba);
	SDL_Surface *tmp = SDL_CreateRGBSurface(0, w, h, 32, 0xFF000000, 0x00FF0000,
						0x0000FF00, 0x000000FF);
	SDL_FillRect(tmp, &(struct SDL_Rect){0, 0, w, h}, cfg.rgba);
	int ret = SDL_BlitSurface(tmp, NULL, dSurface, &dr);
	SDL_FreeSurface(tmp);
	return ret;
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

	if (ywCanvasObjType(textSrc) == YCanvasCircle) {
		DPRINT_ERR("merge surface not yet implemented for Circle\n");
		ygDgbAbort();
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
	SDL_Texture *t;
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
			t = SDL_CreateTextureFromSurface(sg.pWindow, tmp_s);
			Entity *d = yeCreateData(t, txts_w, NULL);
			yeSetDestroy(d, sdlFreeTexture);
		}
		tot_w = o_tot_w;
	}
	yeSetAt(e, 0, YCanvasBigTexture);
	return 0;
}

int sdlCanvasCacheImg3(Entity *elem, Entity *resource, const char *imgPath,
		       Entity *rEnt, int32_t flag, Entity *img_dst_rect)
{
	SDL_Surface *surface;
	SDL_Texture *texture;
	Entity *data;
	int w, h, isText = 0;

	if (!rEnt)
		rEnt = yeGet(elem, "img-src-rect") ? :
			yeGet(resource, "img-src-rect");
	if (!imgPath) {
		surface = yeGetData(yeGet(resource, "$img-surface"));
		isText = 1;
	} else {
		if (access(imgPath, F_OK) < 0) {
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
		SDL_Rect *surface_rect = NULL;
		SDL_Rect tmp_surface_rect;

		if (img_dst_rect) {
			tmp_surface_rect.x = ywRectX(img_dst_rect);
			tmp_surface_rect.y = ywRectY(img_dst_rect);
			tmp_surface_rect.w = ywRectW(img_dst_rect);
			tmp_surface_rect.h = ywRectH(img_dst_rect);
			surface_rect = &tmp_surface_rect;
		}

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
		texture = GPU_CopyImageFromSurfaceRect(sg.pWindow, surface, surface_rect);
		if (unlikely(!texture)) {
			const char *e = SDL_GetError();
			DPRINT_ERR("fail to create texture %s\n", e);
			goto free_surface;
		}
		SDL_QueryTexture(texture, NULL, NULL, &w, &h);
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

int sdlCanvasCacheImg2(Entity *elem, Entity *resource, const char *imgPath,
		       Entity *rEnt, int32_t flag)
{
	return sdlCanvasCacheImg3(elem, resource, imgPath, rEnt, flag, NULL);
}

void sdlCanvasCacheVoidTexture(Entity *obj, Entity *size)
{
	SDL_Surface *surface;
	Entity *data;

	surface = SDL_CreateRGBSurface(0, ywSizeW(size), ywSizeH(size),
				       32, 0xFF000000, 0x00FF0000,
				       0x0000FF00, 0x000000FF);
	data = yeCreateDataAt(surface, obj, "$img-surface",
			      YCANVAS_SURFACE_IDX);
	yeAttach(obj, size, YCANVAS_SIZE_IDX, "$size", 0);
	yeSetDestroy(data, sdlFreeSurface);
	ywSizePrint(ywCanvasObjSize(NULL, obj));
}

void sdlCanvasCacheBicolorImg(Entity *elem, const uint8_t *img, Entity *info)
{
	Entity *size = yeGet(info, 0);
	int w = ywSizeW(size), h = ywSizeH(size);
	int end = w * h;
	uint32_t bg = yeGetIntAt(info, 1);
	uint32_t fg = yeGetIntAt(info, 2);
	uint32_t flags = yeGetIntAt(info, 3);
	SDL_Surface *surface = SDL_CreateRGBSurface(0, w, h, 32,
						    flags & 1 ? 0xFF000000 : 0,
						    flags & 1 ? 0x00FF0000 : 0,
						    flags & 1 ? 0x0000FF00 : 0,
						    flags & 1 ? 0x000000FF : 0);
	uint32_t *pixels = surface->pixels;
	SDL_Texture *texture;
	Entity *data;

	assert(w);
	assert(h);
	for (int i = 0; i < end; ++i) {
		if (!(flags & 1 && img[i] == (uint8_t)-1))
			pixels[i] = img[i] ? fg : bg;
	}
	texture = SDL_CreateTextureFromSurface(sg.pWindow, surface);
	data = yeCreateDataAt(texture, elem, "$img", YCANVAS_IMG_IDX);
	yeSetDestroy(data, sdlFreeTexture);
	ywSizeCreateAt(w, h, elem, "$size", YCANVAS_SIZE_IDX);
	data = yeCreateDataAt(surface, elem, "$img-surface",
			      YCANVAS_SURFACE_IDX);
	yeSetDestroy(data, sdlFreeSurface);
	return;
}

void sdlCanvasDrawableSetPix(Entity *elem, int x, int y, int color)
{
	SDL_Surface *surface = yeGetData(yeGet(elem, YCANVAS_SURFACE_IDX));
	uint32_t *pixels = surface->pixels;
	int w = ywSizeW(yeGet(elem, YCANVAS_SIZE_IDX));

	pixels[y * w + x] = color;
}

void sdlCanvasDrawableFinalyze(Entity *elem)
{
	SDL_Surface *surface = yeGetData(yeGet(elem, YCANVAS_SURFACE_IDX));
	SDL_Texture *texture = SDL_CreateTextureFromSurface(sg.pWindow, surface);
	Entity *data = yeCreateDataAt(texture, elem, "$img", YCANVAS_IMG_IDX);	
	yeSetDestroy(data, sdlFreeTexture);
}

void sdlCanvasDrawableNew(Entity *elem, Entity *size)
{
	Entity *size_cpy = yeCreateCopy(size, NULL, NULL);

	SDL_Surface *surface = SDL_CreateRGBSurface(
		0, ywSizeW(size), ywSizeH(size), 32,
		0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	yePushAt2(elem, size_cpy, YCANVAS_SIZE_IDX, "$size");

	Entity *data = yeCreateDataAt(surface, elem, "$img-surface",
				      YCANVAS_SURFACE_IDX);
	yeSetDestroy(data, sdlFreeSurface);
}

void sdlCanvasCacheHeadacheImg(Entity *elem, Entity *map, Entity *info)
{
	Entity *pix_per_char = yeGet(info, "pix_per_char");
	Entity *size = yeGet(info, "size");
	SDL_Surface *surface = makeHeadacheSurface(map, info);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(sg.pWindow, surface);
	Entity *data = yeCreateDataAt(texture, elem, "$img", YCANVAS_IMG_IDX);

	yeSetDestroy(data, sdlFreeTexture);
	ywSizeCreateAt(ywSizeW(pix_per_char) * ywSizeW(size),
		       ywSizeH(pix_per_char) * ywSizeH(size),
		       elem, "$size", YCANVAS_SIZE_IDX);
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

	if (type == YCanvasRect || type == YCanvasCircle ||
	    type == YCanvasTriangle || type == YCanvasPolygone ||
	    type == YCanvasLine) {
		return 0;
	} else if (ywCanvasObjType(elem) == YCanvasString) {
		Entity *str = yeGet(elem, 2);
		int w = 0, h;

		h = yuiStrCountCh(yeGetString(str), '\n', &w);
		w = (w + 1) * (sg.txtWidth);
		h = (h + 1) * (sg.txtHeight);

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

static void sdlCanvasAplyModifier(Entity *img,
				  double *rotation, SDL_RendererFlip *flip,
				  SDL_Color *mod_color)
{
	Entity *mod = ywCanvasObjMod(img);
	Entity *color_mod;

	if (!mod)
		return;
	color_mod = yeGet(mod, YCanvasColorMod);
	if (color_mod) {
		int a = yeGetIntAt(color_mod, 3);
		mod_color->r = yeGetIntAt(color_mod, 0);
		mod_color->g = yeGetIntAt(color_mod, 1);
		mod_color->b = yeGetIntAt(color_mod, 2);
		mod_color->a = a ? a : 255;
	}
	if (rotation)
		*rotation = yeGetFloat(yeGet(mod, YCanvasRotate));
	if (flip) {
		*flip |= (SDL_FLIP_VERTICAL * yeGetIntAt(mod, YCanvasVFlip));
		*flip |= (SDL_FLIP_HORIZONTAL * yeGetIntAt(mod, YCanvasHFlip));
	}
	return;
}

static int sdlCanvasRendImg(YWidgetState *state, SDLWid *wid, Entity *img,
			    Entity *cam, Entity *wid_pix)
{
	Entity *s = ywCanvasObjSize(state->entity, img);
	Entity *p = ywCanvasObjPos(img);
	SDL_Rect *sd = NULL;
	ygAssert(p);
	ygAssert(s);
	SDL_FRect rd = { ywPosXDirect(p) - ywPosX(cam),
		ywPosYDirect(p) - ywPosY(cam),
		ywSizeWDirect(s), ywSizeHDirect(s) };
	double rotation = 0;
	SDL_RendererFlip flip = 0;
	SDL_Texture *t = NULL;
	SDL_Color mod_color = {0};

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
	sdlCanvasAplyModifier(img, &rotation, &flip, &mod_color);
	if (mod_color.r || mod_color.g || mod_color.b || mod_color.a) {
		SDL_SetTextureColorMod(t, mod_color.r, mod_color.g, mod_color.b);
		SDL_SetTextureAlphaMod(t, mod_color.a);
	}
	SDL_RenderCopyExF(sg.pWindow, t, sd, &rd, rotation,
			  NULL, flip);
	free(sd);
	return 0;
}

static int point_in_poly(Entity *poly, int x, int y)
{
	Entity *vertices = yeGet(poly, YCANVAS_VERTICES_IDX);
	int nvert = yeLen(vertices);
	int i, j, c = 0;
	for (i = 0, j = nvert-1; i < nvert; j = i++) {
		Entity *vi = yeGet(vertices, i);
		Entity *vj = yeGet(vertices, j);
		int vy_i = ywPosY(vi), vx_i = ywPosX(vi);
		int vx_j = ywPosX(vj), vy_j = ywPosY(vj);

		if (((vy_i > y) != (vy_j > y)) &&
		     (x < (vx_j - vx_i) * (y - vy_i) / (vy_j - vy_i) + vx_i))
			c = !c;
	}
	return c;
}

uint32_t sdlCanvasPixInfo(Entity *obj, int x, int y)
{
  if (!obj)
    return 0;
  SDL_Surface *surface = yeGetDataAt(obj, YCANVAS_SURFACE_IDX);
  int type = yeGetIntAt(obj, 0);

  if (type == YCanvasCircle || type == YCanvasTriangle || type == YCanvasLine) {
	  DPRINT_ERR("pixel info not net implemented for most shapes\n");
	  ygDgbAbort();
  } else if (type == YCanvasPolygone) {
	  // I cheat here because I need this only for colision detection
	  return point_in_poly(obj, x, y) * 0xffffffff;
  } else if (type == YCanvasRect) {
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
	SDL_Color mod_color = {0};

	sdlCanvasAplyModifier(obj, NULL, NULL, &mod_color);

	YE_FOREACH(txts_h, txts_w) {
		int threshold_x = 0;
		YE_FOREACH(txts_w, texture) {
			SDL_Texture *t = yeGetData(texture);
			int w, h;
			int x = x0 + threshold_x;
			int y = y0 + threshold_y;

			SDL_QueryTexture(t, NULL, NULL, &w, &h);

			w = yuiPercentOf(w, wpercent);
			h = yuiPercentOf(h, hpercent);

			if (mod_color.r || mod_color.g || mod_color.b || mod_color.a) {
				SDL_SetTextureColorMod(t, mod_color.r, mod_color.g, mod_color.b);
				SDL_SetTextureAlphaMod(t, mod_color.a);
			}
			SDL_RenderCopy(sg.pWindow, t, NULL,
				     &(SDL_Rect){x, y, w, h});
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
	    type == YCanvasTexture || type == YCanvasBicolorImg ||
	    type == YCanvasHeadacheImg)
		return sdlCanvasRendImg(state, wid, obj, cam, wid_pix);

	Entity *p = ywCanvasObjPos(obj);
	SDL_Color c = {0, 0, 0, 255};
	Entity *s = ywCanvasObjSize(state->entity, obj);
	SDL_Rect rect = {ywPosX(p) - ywPosX(cam), ywPosY(p) - ywPosY(cam),
			 ywSizeW(s), ywSizeH(s)};

	if (type == YCanvasCircle) {
		YBgConf cfg;

		ywidBgConfFill(yeGet(obj, 2), &cfg);
		c.r = cfg.r;
		c.g = cfg.g;
		c.b = cfg.b;
		c.a = cfg.a;
		rect.x += ywRectX(wid_pix);
		rect.y += ywRectY(wid_pix);

		sdlDrawCircle(rect, c, yeGetIntAt(p, 3));
		return 0;
	} else if (type == YCanvasTriangle) {
		YBgConf cfg;

		ywidBgConfFill(yeGet(obj, 2), &cfg);
		c.r = cfg.r;
		c.g = cfg.g;
		c.b = cfg.b;
		c.a = cfg.a;
		rect.x += ywRectX(wid_pix);
		rect.y += ywRectY(wid_pix);

		sdlDrawTriangle(rect.x, rect.y, rect.w,
				rect.h, yeGetQuadInt2(s),
				yeGetQuadInt3(s),
				c, yeGetQuadInt3(p));
		return 0;
	} else if (type == YCanvasLine) {
		YBgConf cfg;

		ywidBgConfFill(yeGet(obj, 2), &cfg);
		c.r = cfg.r;
		c.g = cfg.g;
		c.b = cfg.b;
		c.a = cfg.a;
		rect.x += ywRectX(wid_pix);
		rect.y += ywRectY(wid_pix);

		sdlDrawLine(rect.x, rect.y, yeGetQuadInt2(p),
				yeGetQuadInt3(p), c);
		return 0;
	} else if (type == YCanvasPolygone) {
		YBgConf cfg;
		SDL_Vertex vertices[256]; /*assuming smaller*/
		Entity *pos0 = ywCanvasObjPos(obj);
		Entity *vertices_ent = yeGet(obj, YCANVAS_VERTICES_IDX);
		int nb_vertices = yeLen(vertices_ent);
		int i = 0;

		ywidBgConfFill(yeGet(obj, 2), &cfg);
		for (; i < nb_vertices; i++) {
			/* if I want a semblace of size, I should compute it here */
			vertices[i] = (SDL_Vertex){
				{
					yeGetIntAt(yeGet(vertices_ent, i), 0) + ywPosX(pos0),
					yeGetIntAt(yeGet(vertices_ent, i), 1) + ywPosY(pos0)
				}, {cfg.r, cfg.g, cfg.b, cfg.a}, {0, 0}
			};
		}

		/* int isFilled = yeGetQuadInt2(s); */
		sdlDrawPolyFilled(nb_vertices, vertices);
		return 0;
	} else if (type == YCanvasRect) {
		YBgConf cfg;

		ywidBgConfFill(yeGet(yeGet(obj, 2), 1), &cfg);
		c.r = cfg.r;
		c.g = cfg.g;
		c.b = cfg.b;
		c.a = cfg.a;
		rect.x += ywRectX(wid_pix);
		rect.y += ywRectY(wid_pix);
		// stuff to do here
		sdlDrawRect2(NULL, rect, c,
			     yeGetIntAt(obj, YCANVAS_RECT_IS_FILLED_IDX),
			     yeGetFloatAt(obj, YCANVAS_ROUNDED_RADIUS_IDX));
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

			if (type != Y_SDL_COLOR) {
				SDL_QueryTexture(texture, NULL, NULL, &srcR.w, &srcR.h);
				int diff = srcR.w - srcR.h;
				if (diff > 0) {
					int bigger = srcR.w;
					DestR.h = DestR.h * srcR.h / bigger;
					diff = DestR.w - DestR.h;
					DestR.y += (diff / 2);
				} else if (diff < 0) {
					int bigger = srcR.h;
					DestR.w = DestR.w * srcR.w / bigger;
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
				SDL_RenderCopy(sg.pWindow, texture, srcRP, &DestR);
		} else {
			Entity *rinfo = yeGet(mapElem, "rend_info");
			SDL_Rect srcR = YRECT_MK_INIT(yeGet(rinfo, "src"));
			SDL_Rect *rp = rinfo ? &srcR : NULL;
			Entity *pt = yeGet(rinfo, "threshold");
			int xt = ywPosX(pt);
			int yt = ywPosY(pt);

			if (!rinfo) {
				SDL_QueryTexture(texture, NULL, NULL, &DestR.w, &DestR.h);
			}
			else {
				DestR.w = srcR.w;
				DestR.h = srcR.h;
			}
			DestR.y = y * h - (DestR.h - h) + wid->rect.y + yt;
			DestR.x += xt;
			SDL_RenderCopy(sg.pWindow, texture, rp, &DestR);
		}
	}
	return 0;
}
