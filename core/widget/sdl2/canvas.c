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
	Entity *cam_threshold = yeGet(entity, "cam-threshold");
	Entity *widPix;
	Entity *dst = s->merge_texture;
	YBgConf cfg;

	if (cam_threshold) {
		cam = ywPosCreate(cam, 0, NULL, NULL);
		ywPosAdd(cam, cam_threshold);
	}

	wid = sddComputeMargin(state, wid);
	widPix = yeGet(state->entity, "wid-pix");
	ywidShowBG(entity, wid);

	if (s->flag & YC_MERGE_NO_MERGE) {
		yeAutoFree Entity *dst_rect = ywRectCreateInts(
			ywPosX(cam),
			ywPosY(cam),
			ywRectW(widPix),
			ywRectH(widPix),
			NULL, NULL);
		Entity *e = ywCanvasNewImgFromTexture2(entity, 0,
						       0, dst, NULL, dst_rect);
		sdlCanvasRendObj(state, wid, e, NULL, widPix);
		ywCanvasRemoveObj(entity, e);
	} else if (s->flag & YC_MERGE) {
		if (yeType(objs) == YARRAY) {
			YE_ARRAY_FOREACH(objs, obj) {
				yeAutoFree Entity *dst_rect = ywRectCreatePosSize(
					ywCanvasObjPos(obj),
					ywCanvasObjSize(entity, obj),
					NULL, NULL);

				ywTextureFastMerge(obj, NULL, dst, dst_rect);
			}
		} else {
			int l = yeLen(objs);

			for (int i = 0; i < l; ++i) {
				Entity *obj = yeGet(objs, i);

				yeAutoFree Entity *dst_rect = ywRectCreatePosSize(
					ywCanvasObjPos(obj),
					ywCanvasObjSize(entity, obj),
					NULL, NULL);

				ywTextureFastMerge(obj, NULL, dst, dst_rect);
			}
		}
		ywCanvasClear(entity);
		sdlCanvasRendObj(state, wid,
				 ywCanvasNewImgFromTexture(entity, 0,
							   0, dst, NULL),
				 cam, widPix);
		ywCanvasClear(entity);
		goto forground;
	}
	if (yeType(objs) == YARRAY) {
		YE_ARRAY_FOREACH(objs, obj) {
			sdlCanvasRendObj(state, wid, obj, cam, widPix);
		}
	} else {
		Entity *obj;
		YE_VECTORE_FOREACH(objs, obj) {
			sdlCanvasRendObj(state, wid, obj, cam, widPix);
		}
	}

forground:
	if (ywidBgConfFill(yeGet(state->entity, "foreground"), &cfg) >= 0)
		sdlFillBg(wid, &cfg);
	if (cam_threshold)
		yeDestroy(cam);
	return 0;
}

static int sdl2Init(YWidgetState *wid, int t)
{
	wid->renderStates[t].opac = y_new(SDLWid, 1);
	sdlWidInit(wid, t);
	return 0;
}

int ysdl2RegistreCanvas(void)
{
	int ret = ywidRegistreTypeRender("canvas", ysdl2Type(),
					 sdl2Render, sdl2Init, sdlWidDestroy);
	return ret;
}

