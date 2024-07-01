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
#include "timer.h"
#include "canvas.h"

static int wid_t = -1;

struct YEBSState {
	YWidgetState sate;
	YTimer *timer;
};

int ywSetGroupeDir(Entity *wid, unsigned int grp, double radiant)
{
	unsigned int last = yeIntAt(wid, "nb_grp");
	Entity *groups_dir = yeGet(wid, "grps-dir");

	if (grp >= last) {
		DPRINT_ERR("groupe index is superior to last groups");
		return -1;
	}
	yeSetFloatAt(groups_dir, grp, radiant);
	return -1;
}

int ywSetGroupeSpeed(Entity *wid, unsigned int grp, int speed)
{
	unsigned int last = yeIntAt(wid, "nb_grp");
	Entity *groups_spds = yeGet(wid, "grps-spd");

	if (grp >= last) {
		DPRINT_ERR("groupe index is superior to last groups");
		return -1;
	}
	yeSetIntAt(groups_spds, grp, speed);
	return -1;
}

int ywEBSSwapGroup(Entity *wid, unsigned int target)
{
	unsigned int last = yeIntAt(wid, "nb_grp");
	Entity *groups = yeGet(wid, "groups");

	if (target >= last) {
		DPRINT_ERR("target is superiot to last groups");
		return -1;
	}
	yeReCreateInt(target, wid, "cur_grp");
	yeReplaceBack(wid, yeGet(groups, yeIntAt(wid, "cur_grp")), "objs");
	return 0;
}

void ywEBSWrapperPush(Entity *wid, Entity *wrapper)
{
	Entity *wrappers = yeTryCreateArray(wid, "grps_wrapper");
	int cur_grp = yeIntAt(wid, "cur_grp");
	Entity *groups = yeGet(wid, "groups");
	Entity *grp_wrapper;

	if ((grp_wrapper = yeGet(wrappers, cur_grp)) == NULL)
		grp_wrapper = yeCreateArrayAt(wrappers, NULL, cur_grp);
	yePushAt(grp_wrapper, wrappers, yeLen(yeGet(groups, cur_grp)) - 1);
}

static int init(YWidgetState *opac, Entity *entity, void *args)
{
	struct YEBSState *state = (struct YEBSState *)opac;

	state->timer = YTimerCreate();
	yeTryCreateInt(0, entity, "cur_grp");
	yeTryCreateInt(5, entity, "nb_grp");
	Entity *grps = yeTryCreateArray(entity, "groups");
	Entity *grps_spd = yeTryCreateArray(entity, "grps-spd");
	Entity *grps_spd_rest = yeTryCreateArray(entity, "grps-rest-spd");
	Entity *grps_dir = yeTryCreateArray(entity, "grps-dir");

	for (int i = 0; i < yeIntAt(entity, "nb_grp"); ++i) {
		yeCreateArray(grps, NULL);
		yeCreateInt(0, grps_spd, NULL);
		yeCreateFloat(0, grps_dir, NULL);
		yeCreateQuadInt2(0, 0, grps_spd_rest, NULL);
	}
	/* this is made so most canvas function are compatible with this */
	yeReplaceBack(entity, yeGet(grps, yeIntAt(entity, "cur_grp")), "objs");
	ywidGenericCall(opac, wid_t, init);
	return 0;
}

static int destroy(YWidgetState *opac)
{
	struct YEBSState *state = (struct YEBSState *)opac;
	free(state->timer);
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

static int sdl2Render(YWidgetState *opac, int t)
{
	Entity *entity = opac->entity;
	YBgConf cfg;
	SDLWid *wid = ywidGetRenderData(opac, t);
	Entity *cam = yeGet(entity, "cam");
	Entity *cam_threshold = yeGet(entity, "cam-threshold");
	Entity *grps = yeGet(entity, "groups");
	Entity *widPix;
	struct YEBSState *state = (struct YEBSState *)opac;
	YTimer *timer_s = state->timer;
	unsigned long int timer = YTimerGet(timer_s);

	YTimerReset(timer_s);

	widPix = yeGet(entity, "wid-pix");
	if (cam_threshold) {
		cam = ywPosCreate(cam, 0, NULL, NULL);
		ywPosAdd(cam, cam_threshold);
	}
	if (ywidBgConfFill(yeGet(entity, "background"), &cfg) >= 0)
		sdlFillBg(wid, &cfg);


	/* here we move stuff that need to be move, and so on */
	Entity *speeds = yeGet(entity, "grps-spd");
	Entity *rads = yeGet(entity, "grps-dir");
	Entity *rest =  yeGet(entity, "grps-rest-spd");

	for (size_t i = 0; i < yeLen(speeds); ++i) {
		int g_speed = yeGetIntAt(speeds, i);
		double g_rad = yeGetFloatAt(rads, i);
		Entity *g_rest = yeGet(rest, i);
		Entity *grp = yeGet(grps, i);

		if (!g_speed)
			continue;

		// compute x/y advance here, + rest, store rest
		int cur_speed = (g_speed * 10000) / timer;
		double old_rest_x = yeGetQuadInt0(g_rest);
		double old_rest_y = yeGetQuadInt1(g_rest);
		old_rest_x = old_rest_x / 10000;
		old_rest_y = old_rest_y / 10000;

		double delta_x = cur_speed * cos(g_rad) + old_rest_x;
		double delta_y = cur_speed * sin(g_rad) + old_rest_y;
		int rest_x = (((double) (int)delta_x) - delta_x) * 10000;
		int rest_y = (((double) (int)delta_y) - delta_y) * 10000;

		yeSetAt(g_rest, 0, rest_x);
		yeSetAt(g_rest, 1, rest_y);

		// move everything by x/y
		YE_FOREACH(grp, o) {
			ywCanvasMoveObjXY(o, delta_x, delta_y);
		}

	}


	YE_FOREACH(grps, g) {
		YE_FOREACH(g, o) {
			sdlCanvasRendObj(opac, wid, o, cam, widPix);
		}
	}

	if (ywidBgConfFill(yeGet(entity, "foreground"), &cfg) >= 0)
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

int ysdl2RegistreEBS(void)
{
	int ret = ywidRegistreTypeRender("EBS", ysdl2Type(),
					 sdl2Render, sdl2Init, sdlWidDestroy);
	return ret;
}
