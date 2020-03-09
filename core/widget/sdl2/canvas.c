/*
**Copyright (C) 2017 Matthias Gatto
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
#include <glib.h>
#include "sdl-internal.h"
#include "yirl/texture.h"
#include "yirl/canvas.h"
#include "yirl/rect.h"
#include "yirl/entity.h"

static int sdl2Render(YWidgetState *state, int t)
{
	YCanvasState *s = (void *)state;
	SDLWid *wid = ywidGetRenderData(state, t);
	Entity *entity = state->entity;
	Entity *objs = yeGet(entity, "objs");
	Entity *cam = yeGet(entity, "cam");
	Entity *widPix = yeGet(state->entity, "wid-pix");
	YBgConf cfg;

	if (ywidBgConfFill(yeGet(entity, "background"), &cfg) >= 0)
		sdlFillBg(wid, &cfg);
	if (s->flag & YC_MERGE) {
		Entity *dst = s->merge_texture;
		char *str;

		YE_ARRAY_FOREACH(objs, obj) {
			str = yeToCStr(dst, -1, YE_FORMAT_PRETTY);
			printf("OBJ: %s\n", str);
			free(str);
			ywTextureMerge(obj, NULL, dst, NULL);
		}
		str = yeToCStr(dst, -1, YE_FORMAT_PRETTY);
		printf("DST: %s\n", str);
		free(str);
		ywCanvasClear(entity);
		ywCanvasNewImgFromTexture(entity, 0, 0, dst, NULL);
	}
	YE_ARRAY_FOREACH(objs, obj) {
		sdlCanvasRendObj(state, wid, obj, cam, widPix);
	}

	if (ywidBgConfFill(yeGet(state->entity, "foreground"), &cfg) >= 0)
		sdlFillBg(wid, &cfg);
	return 0;
}

static int sdl2Init(YWidgetState *wid, int t)
{
	wid->renderStates[t].opac = g_new(SDLWid, 1);
	sdlWidInit(wid, t);
	return 0;
}

int ysdl2RegistreCanvas(void)
{
	int ret = ywidRegistreTypeRender("canvas", ysdl2Type(),
					 sdl2Render, sdl2Init, sdlWidDestroy);
	return ret;
}

