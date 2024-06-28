/*
**Copyright (C) 2024 Matthias Gatto
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

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "sdl2/sdl-internal.h"

static int wid_t = -1;

struct YEBSState {
	YWidgetState sate;
};

static int init(YWidgetState *opac, Entity *entity, void *args)
{
	struct YEBSState *state = (struct YEBSState *)opac;
	(void)entity;
	(void)args;
	(void)state;

	return 0;
}

static int destroy(YWidgetState *opac)
{
	free(opac);
	return 0;
}

static int rend(YWidgetState *opac)
{
	ywidGenericRend(opac, wid_t, render);
	return 0;
}

static void *alloc(void)
{
	struct YEBSState *s = y_new0(struct YEBSState, 1);
	YWidgetState *ret = (YWidgetState *)s;
	ret->render = rend;
	ret->init = init;
	ret->destroy = destroy;
	ret->handleEvent = ywidEventCallActionSin;
	ret->type = wid_t;
	return ret;
}

int ywEBSInit(void)
{
	if (ysdl2Type() < 0) {
		DPRINT_ERR("sdl is needed for EBS");
 	}
	if (wid_t != -1)
		return wid_t;
	wid_t = ywidRegister(alloc, "EBS");
	return wid_t;
}

int ywEBSEnd(void)
{
	if (ywidUnregiste(wid_t) < 0)
		return -1;
	wid_t = -1;
	return 0;
}

static int sdl2Render(YWidgetState *state, int t)
{
	(void)state;
	printf("EBS renderer !\n");
	return 0;
}

static int sdl2Init(YWidgetState *wid, int t)
{
	wid->renderStates[t].opac = y_new(SDLWid, 1);
	sdlWidInit(wid, t);
	return 0;
}

int ysdl2RegistreEBS(void)
{
	int ret = ywidRegistreTypeRender("EBS", ysdl2Type(),
					 sdl2Render, sdl2Init, sdlWidDestroy);
	return ret;
}
