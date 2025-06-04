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
#include "events.h"
#include "entity-script.h"
#include "event-base-objs.h"

static int wid_t = -1;

struct YEBSState {
	YWidgetState state;
	YTimer *timer;
	int dir_mask;
};

void ywEBSRemoveObj(Entity *wid, int grp, Entity *obj)
{
	Entity *groups = yeGet(wid, "groups");
	Entity *objs = yeGet(groups, grp);

	yeRemoveChild(objs, obj);
}

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
		DPRINT_ERR("target is superior to last groups");
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
	state->dir_mask = 0;
	yeTryCreateInt(0, entity, "cur_grp");
	yeTryCreateInt(5, entity, "nb_grp");
	yeTryCreateArray(entity, "on-down");
	yeTryCreateArray(entity, "on-up");
	Entity *grps = yeTryCreateArray(entity, "groups");
	yeTryCreateArray(entity, "grps-no-cam");
	yeTryCreateArray(entity, "grps-col-callbacks");
	yeTryCreateArray(entity, "grps-oob-callbacks");
	yeTryCreateArray(entity, "grps-mv-callbacks");
	yeTryCreateArray(entity, "grps-allow-oob");
	Entity *grps_spd = yeTryCreateArray(entity, "grps-spd");
	Entity *grps_spd_rest = yeTryCreateArray(entity, "grps-rest-spd");
	Entity *grps_dir = yeTryCreateArray(entity, "grps-dir");

	ywSetTurnLengthOverwrite(-1);

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


/* see arrow as this: < ^ V >, then if you put a flag under each on, it give:
 * < ^ V >
 * 8,4,2,1
 *
 * rad circle:
 * 2pi / 3 ; pi / 2   ; pi / 4
 * pi;                ; 0
 * 5pi//4;  3 pi / 2  ; 7pi / 4
 */
static void compute_speed_n_dir(struct YEBSState *state, int grp)
{
	switch (state->dir_mask) {
	case 4:
		ywSetGroupeDir(state->state.entity, grp, M_PI_2);
		break;
	case 4 | 1:
		ywSetGroupeDir(state->state.entity, grp, M_PI_4);
		break;
	case 1 | 2:
		ywSetGroupeDir(state->state.entity, grp, 7 * M_PI / 4);
		break;
	case 1:
		ywSetGroupeDir(state->state.entity, grp, 0);
		break;
	case 2 | 8:
		ywSetGroupeDir(state->state.entity, grp, 5 * M_PI / 4);
		break;
	case 2:
		ywSetGroupeDir(state->state.entity, grp, 3 * M_PI / 2);
		break;
	case 8:
		ywSetGroupeDir(state->state.entity, grp, M_PI);
		break;
	case 8 | 4:
		ywSetGroupeDir(state->state.entity, grp, 3 * M_PI / 4);
		break;
	}

	if (state->dir_mask)
		ywSetGroupeSpeed(state->state.entity, grp, 10);
	else
		ywSetGroupeSpeed(state->state.entity, grp, 0);
}

static InputStatue event(YWidgetState *opac, Entity *event)
{
	(void)opac;
	Entity *entity = opac->entity;
	Entity *eve = event;
	Entity *clasic_movement = yeGet(entity, "grp-classic-movement");
	int mv_style = yeGetIntAt(entity, "classic-movement-style");
	struct YEBSState *state = (struct YEBSState *)opac;

	if (!mv_style)
		mv_style = 1;

	YEVE_FOREACH(eve, event) {
		if (ywidEveType(eve) == YKEY_DOWN) {
			Entity *on = yeGet(entity, "on-down");
			Entity *any_down = yeGet(entity, "any-down");

			if (clasic_movement) {
				int k = ywidEveKey(eve);
				if ((k == 'a' && mv_style & EBS_WASD) ||
				    (k == Y_LEFT_KEY && mv_style & EBS_ARROW)) {
					state->dir_mask |= 8;
				}
				if ((k == 'd' && mv_style & EBS_WASD) ||
				    (k == Y_RIGHT_KEY && mv_style & EBS_ARROW)) {
					state->dir_mask |= 1;
				}
				if ((k == 'w' && mv_style & EBS_WASD) ||
				    (k == Y_UP_KEY && mv_style & EBS_ARROW)) {
					state->dir_mask |= 2;
				}
				if ((k == 's' && mv_style & EBS_WASD) ||
				    (k == Y_DOWN_KEY && mv_style & EBS_ARROW)) {
					state->dir_mask |= 4;
				}
			}

			YE_FOREACH(on, on_entry) {
				if (yeGetIntAt(on_entry, 0) == ywidEveKey(eve)) {
					yesCall(yeGet(on_entry, 1), entity);
				}
			}
			if (any_down)
				yesCall(any_down, entity, eve);
		}
		if (ywidEveType(eve) == YKEY_UP) {
			Entity *on = yeGet(entity, "on-up");
			Entity *any_up;

			if (clasic_movement) {
				int k = ywidEveKey(eve);

				if ((k == 'w' && mv_style & EBS_WASD) ||
				    (k == Y_UP_KEY && mv_style & EBS_ARROW)) {
					state->dir_mask ^= 2;
				}
				if ((k == 'd' && mv_style & EBS_WASD) ||
				    (k == Y_RIGHT_KEY && mv_style & EBS_ARROW)) {
					state->dir_mask ^= 1;
				}
				if ((k == 'a' && mv_style & EBS_WASD) ||
				    (k == Y_LEFT_KEY && mv_style & EBS_ARROW)) {
					state->dir_mask ^= 8;
				}
				if ((k == 's' && mv_style & EBS_WASD) ||
				    (k == Y_DOWN_KEY && mv_style & EBS_ARROW)) {
					state->dir_mask ^= 4;
				}
			}

			YE_FOREACH(on, on_entry) {
				if (yeGetIntAt(on_entry, 0) == ywidEveKey(eve)) {
					yesCall(yeGet(on_entry, 1), entity);
				}
			}
			if ((any_up = yeGet(entity, "any-up")) != NULL)
				yesCall(any_up, entity, eve);
		}
	}

	if (clasic_movement) {
		int grp = yeGetInt(clasic_movement);

		compute_speed_n_dir((void *)opac, grp);
	}

	if (!event)
		return NOTHANDLE;
	return ACTION;
}

static void *alloc(void)
{
	struct YEBSState *s = y_new0(struct YEBSState, 1);
	YWidgetState *ret = (YWidgetState *)s;
	ret->render = rend;
	ret->init = init;
	ret->destroy = destroy;
	ret->handleEvent = event;
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
	Entity *grps_no_cam = yeGet(entity, "grps-no-cam");
	struct YEBSState *state = (struct YEBSState *)opac;
	YTimer *timer_s = state->timer;
	unsigned long int timer = YTimerGet(timer_s);

	YTimerReset(timer_s);

	widPix = yeGet(entity, "wid-pix");
	if (cam_threshold) {
		cam = ywPosCreate(cam, 0, NULL, NULL);
		ywPosAdd(cam, cam_threshold);
	}

	ywidShowBG(entity, wid);

	/* here we move stuff that need to be move, and so on */
	Entity *speeds = yeGet(entity, "grps-spd");
	Entity *rads = yeGet(entity, "grps-dir");
	Entity *rest =  yeGet(entity, "grps-rest-spd");

	for (size_t i = 0; i < yeLen(speeds); ++i) {
		Entity *mv_callback = yeGet(yeGet(entity, "grps-mv-callbacks"), i);
		Entity *col_callback = yeGet(yeGet(entity, "grps-col-callbacks"), i);
		int g_speed = yeGetIntAt(speeds, i);
		double g_rad = yeGetFloatAt(rads, i);
		Entity *g_rest = yeGet(rest, i);
		Entity *grp = yeGet(grps, i);
		Entity *oob_callback = yeGet(yeGet(entity, "grps-oob-callbacks"), i);
		int allow_oob = yeGetIntAt(yeGet(entity, "grps-allow-oob"), i);
		double delta_x = 0;
		double delta_y = 0;

		if (g_speed) {
			// compute x/y advance here, + rest, store rest
			int cur_speed = (g_speed * 10000) / timer;
			double old_rest_x = yeGetQuadInt0(g_rest);
			double old_rest_y = yeGetQuadInt1(g_rest);
			old_rest_x = old_rest_x / 10000;
			old_rest_y = old_rest_y / 10000;

			delta_x = cur_speed * cos(g_rad) + old_rest_x;
			delta_y = cur_speed * sin(g_rad) + old_rest_y;
			int rest_x = (((double) (int)delta_x) - delta_x) * 10000;
			int rest_y = (((double) (int)delta_y) - delta_y) * 10000;

			yeSetAt(g_rest, 0, rest_x);
			yeSetAt(g_rest, 1, rest_y);
		} else if (!mv_callback) {
			continue;
		}

		// move everything by x/y
		YE_FOREACH(grp, o) {
			int have_oob = 0;
			Entity *o_pos = ywCanvasObjPos(o);
			Entity *o_size = ywCanvasObjSize(entity, o);

			if (mv_callback) {
				yeAutoFree Entity *new_delta = yesCall(mv_callback, wid, o, delta_x, delta_y);
				if (new_delta) {
					delta_x = ywPosX(new_delta);
					delta_y = ywPosY(new_delta);
				} else {
					delta_x = 0;
					delta_y = 0;
				}
			}


			if (ywPosX(o_pos) + ywSizeW(o_size) + delta_x > ywRectW(widPix)) {
				have_oob = 1;
				if (!allow_oob) {
					delta_x -= ywPosX(o_pos) + ywSizeW(o_size) +
						delta_x - ywRectW(widPix);
				}
			} else if (ywPosX(o_pos) + delta_x < 0) {
				have_oob = 1;
				if (!allow_oob) {
					delta_x -= ywPosX(o_pos) + delta_x;
				}
			}
			if (ywPosY(o_pos) + ywSizeH(o_size) + delta_y > ywRectH(widPix)) {
				have_oob = 1;
				if (!allow_oob) {
					delta_y -= ywPosY(o_pos) + ywSizeH(o_size) +
						delta_y - ywRectH(widPix);
				}
			} else if (ywPosY(o_pos) + delta_y < 0) {
				have_oob = 1;
				if (!allow_oob) {
					delta_y -= ywPosY(o_pos) + delta_y;
				}
			}
			ywCanvasMoveObjXY(o, delta_x, delta_y);
			if (have_oob && oob_callback) {
				yesCall(oob_callback, entity, o);
			}
#define CHECK_COL(target_gpr, call)					\
			do {						\
				Entity *o_grp = yeGet(grps, target_gpr); \
				YE_FOREACH(o_grp, other_obj) {		\
					if (ywCanvasObjectsCheckColisions(o, other_obj)) { \
						yesCall(call, entity, o, other_obj); \
					}				\
				}					\
			} while (0)

			if (col_callback) {
				Entity *target_grps = yeGet(col_callback, 0);
				Entity *callback = yeGet(col_callback, 1);

				if (yeType(target_grps) == YINT) {
					int target_gpr = yeGetInt(target_grps);
					CHECK_COL(target_gpr, callback);
				} else {
					for (int i = 0, target_gpr_l = yeLeni(target_grps);
					     i < target_gpr_l; ++i) {
						Entity *t_gpr = yeGet(target_grps, i);
						int target_gpr = yeGetInt(t_gpr);
						Entity *call = callback;
						if (yeType(call) == YARRAY)
							call = yeGet(callback, i);

						CHECK_COL(target_gpr, call);
					}
				}
			}
#undef CHECK_COL

		}

	}


	for (size_t i = 0, len = yeLen(grps); i < len; ++i) {
		Entity *g = yeGet(grps, i);

		if (!g)
			continue;
		int no_cam = yeGetIntAt(grps_no_cam, i);
		YE_FOREACH(g, o) {
			sdlCanvasRendObj(opac, wid, o, no_cam ? NULL : cam, widPix);
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
