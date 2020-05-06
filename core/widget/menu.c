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
#include "menu.h"
#include "container.h"
#include "rect.h"
#include "native-script.h"
#include "entity-script.h"
#include "game.h"

static int t = -1;

typedef struct {
	YWidgetState sate;
	unsigned int current;
} YMenuState;

static void *tryCallMoveOn(YWidgetState *wid)
{
	Entity *moveOn = yeGet(wid->entity, "moveOn");
	if (moveOn)
		return yesCall(moveOn, wid->entity,
			       ((YMenuState *)wid)->current,
			       ywMenuGetCurrentEntry(wid->entity));
	return (void *)ACTION;
}

void *ywMenuMove(Entity *ent, uint32_t at)
{
	YMenuState *s = (YMenuState *)ywidGetState(ent);

	if (at > yeLen(yeGet(ent, "entries")))
		return (void *)NOTHANDLE;
	s->current = at;
	return tryCallMoveOn((YWidgetState *)s);
}

static void *nmMenuDown(YWidgetState *wid)
{
	((YMenuState *)wid)->current += 1;

	if (((YMenuState *)wid)->current > yeLen(yeGet(wid->entity,
						       "entries")) - 1)
		((YMenuState *)wid)->current = 0;
	if (yeGetInt(yeGet(ywMenuGetCurrentEntry(wid->entity), "hiden")))
		return nmMenuDown(wid);
	return tryCallMoveOn(wid);
}

static void *nmMenuUp(YWidgetState *wid)
{
	((YMenuState *)wid)->current -= 1;

	if (((YMenuState *)wid)->current > yeLen(yeGet(wid->entity, "entries")))
		((YMenuState *)wid)->current =
			yeLen(yeGet(wid->entity, "entries")) - 1;
	if (yeGetInt(yeGet(ywMenuGetCurrentEntry(wid->entity), "hiden")))
		return nmMenuUp(wid);
	return tryCallMoveOn(wid);
}

static void *nmMenuMove(int nb, union ycall_arg *args, int *types)
{
	Entity *wid = args[0].e;
	Entity *eve = args[1].e;
	Entity *cu = ywMenuGetCurrentEntry(wid);
	Entity *s;

	if ((s = yeGet(cu, "slider"))) {
		Entity *si = yeGet(cu, "slider_idx");

		if (ywidEveKey(eve) == Y_RIGHT_KEY) {
			yeAdd(si, 1);
		} else if (ywidEveKey(eve) == Y_LEFT_KEY) {
			yeAdd(si, -1);
		}
		if (yeGetInt(si) < 0)
			yeSetInt(si, yeLen(s) - 1);
		else if ((unsigned int)yeGetInt(si) > (yeLen(s) - 1))
			yeSetInt(si, 0);
	}

	if (ywidEveKey(eve) == Y_DOWN_KEY) {
		return nmMenuDown(ywidGetState(wid));
	} else if (ywidEveKey(eve) == Y_UP_KEY) {
		return nmMenuUp(ywidGetState(wid));
	}
	return (void *)NOTHANDLE;
}

static void *nmPanelMove(int nb, union ycall_arg *args, int *types)
{
	YWidgetState *wid = ywidGetState(args[0].e);
	Entity *eve = args[1].e;

	if (ywidEveKey(eve) == Y_RIGHT_KEY) {
		return nmMenuDown(wid);
	} else if (ywidEveKey(eve) == Y_LEFT_KEY) {
		return nmMenuUp(wid);
	}
	return (void *)NOTHANDLE;
}

static void *nmMenuNext(int nb, union ycall_arg *args, int *types)
{
	Entity *e = args[0].e;
	YWidgetState *wid = ywidGetState(e);
	Entity *next = yeGet(e, "entries");

	next = yeGet(next, ((YMenuState *)wid)->current);
	next = yeGet(next, "next");

	return ywidNext(next, e) ? (void *)BUG : (void *)ACTION;
}


InputStatue mnActions_(Entity *menu, Entity *event, Entity *current_entry)
{
	Entity *s;

	if ((s = yeGet(current_entry, "slider"))) {
		Entity *option =
			yeGet(s, yeGetIntAt(current_entry, "slider_idx"));

		return ywidActions(menu, option, event);
	}
	return ywidActions(menu, current_entry, event);
}

static void *mnActions(int nb, union ycall_arg *args, int *types)
{
	Entity *wid = args[0].e;
	Entity *eve = args[1].e;

	return (void *)mnActions_(wid, eve, ywMenuGetCurrentEntry(wid));
}

void ywMenuUp(Entity *wid)
{
	nmMenuUp(ywidGetState(wid));
}

void ywMenuDown(Entity *wid)
{
	nmMenuDown(ywidGetState(wid));
}

static int mnInit(YWidgetState *opac, Entity *entity, void *args)
{
	ywidGenericCall(opac, t, init);

	yeRemoveChildByStr(entity, "move");
	if (!yeStrCmp(yeGet(entity, "mn-type"), "panel")) {
		yeCreateFunction("panelMove", ysNativeManager(),
				 entity, "move");
	} else {
		yeCreateFunction("menuMove", ysNativeManager(),
				 entity, "move");
	}
	((YMenuState *)opac)->current = yeGetInt(yeGet(entity, "current"));
	return 0;
}

static int mnDestroy(YWidgetState *opac)
{
	g_free(opac);
	return 0;
}

static int mnRend(YWidgetState *opac)
{
	ywidGenericRend(opac, t, render);
	return 0;
}

InputStatue ywMenuCallActionOnByEntity(Entity *opac, Entity *event,
				       int idx)
{
	return ywMenuCallActionOnByState(ywidGetState(opac), event, idx);
}

InputStatue ywMenuCallActionOnByState(YWidgetState *opac, Entity *event,
				      int idx)
{
	InputStatue ret;

	if (idx < 0)
		return NOTHANDLE;
	((YMenuState *)opac)->current = idx;

	ret = mnActions_(opac->entity, event, ywMenuGetCurrentEntry(opac->entity));
	if (ret == NOTHANDLE)
		return NOACTION;
	return ret;
}

static InputStatue mnEvent(YWidgetState *opac, Entity *event)
{
	InputStatue ret = NOTHANDLE;

	(void)opac;

	if (!event)
		return NOTHANDLE;

	if (ywidEveType(event) == YKEY_DOWN) {
		Entity *on = yeGet(opac->entity, "on");

		YE_FOREACH(on, on_entry) {
			if (yeGetIntAt(on_entry, 0) == ywidEveKey(event)) {
				return (InputStatue)yesCall(
					yeGet(on_entry, 1), opac->entity,
					((YMenuState *)opac)->current,
					ywMenuGetCurrentEntry(opac->entity));
			}
		}

		if (ywidEveKey(event) == Y_ESC_KEY) {
			Entity *onEsc = yeGet(opac->entity, "onEsc");

			if (onEsc) {
				return (InputStatue)yesCall(
					onEsc,
					opac->entity,
					((YMenuState *)opac)->current,
					ywMenuGetCurrentEntry(opac->entity));
			}
		} else if (ywidEveKey(event) == '\n') {
			ret = ywMenuCallActionOn(opac, event,
						 ((YMenuState *)opac)->current);
		} else {
			ret = (InputStatue)yesCall(yeGet(opac->entity, "move"),
						   opac->entity, event);
		}
	} else if (ywidEveType(event) == YKEY_MOUSEDOWN) {
		ret = ywMenuCallActionOn(
			opac, event,
			ywMenuPosFromPix(opac->entity,
					 ywPosX(ywidEveMousePos(event)),
					 ywPosY(ywidEveMousePos(event))));
	}
	return ret;
}


static void *alloc(void)
{
	YMenuState *ret = g_new0(YMenuState, 1);
	YWidgetState *wstate = (YWidgetState *)ret;

	if (!ret)
		return NULL;
	wstate->render = mnRend;
	wstate->init = mnInit;
	wstate->destroy = mnDestroy;
	wstate->handleEvent = mnEvent;
	wstate->type = t;
	return  ret;
}

int ywMenuHasChange(YWidgetState *opac)
{
	return 1;
}

int ywMenuPosFromPix(Entity *wid, uint32_t x, uint32_t y)
{
	Entity *entries = yeGet(wid, "entries");
	Entity *pos = yeGet(wid, "wid-pix");

	YE_ARRAY_FOREACH_EXT(entries, entry, it) {
		Entity *rect = yeGet(entry, "$rect");
		if (ywRectContain(rect, x - ywRectX(pos), y - ywRectY(pos), 1))
			return it.pos;
	}
	return -1;
}

Entity *ywMenuPushEntry(Entity *menu, const char *name, Entity *func)
{
	Entity *entries = yeGet(menu, "entries");
	if (unlikely(!entries))
		entries = yeCreateArray(menu, "entries");
	Entity *entry = yeCreateArray(entries, name);

	yeCreateString(name, entry, "text");
	yePushBack(entry, func, "action");
	return entry;
}

Entity *ywMenuPushSlider(Entity *menu, const char *name, Entity *slider_array)
{
	Entity *entries = yeGet(menu, "entries");
	if (unlikely(!entries))
		entries = yeCreateArray(menu, "entries");
	Entity *entry = yeCreateArray(entries, name);

	yeCreateString(name, entry, "text");
	yePushBack(entry, slider_array, "slider");
	yeCreateInt(0, entry, "slider_idx");
	return entry;

}

int ywMenuGetCurrent(YWidgetState *opac)
{
	return ((YMenuState *)opac)->current;
}

void ywMenuSetCurrentEntry(Entity *entity, Entity *entry)
{
	ywMenuMove(yeGet(entity, "entries"), yeArrayIdx(entity, entry));
}

Entity *ywMenuGetCurrentEntry(Entity *entity)
{
	return yeGet(yeGet(entity, "entries"),
		     ywMenuGetCurrent(ywidGetState(entity)));
}

int ywMenuInit(void)
{
	if (t != -1)
		return t;
	t = ywidRegister(alloc, "menu");
	ysRegistreNativeFunc("menuMove", nmMenuMove);
	ysRegistreNativeFunc("panelMove", nmPanelMove);
	ysRegistreNativeFunc("menuNext", nmMenuNext);
	ysRegistreNativeFunc("menuActions", mnActions);
	return t;
}

int ywMenuEnd(void)
{
	if (ywidUnregiste(t) < 0)
		return -1;
	t = -1;
	return 0;
}

Entity *ywMenuGetEntry(Entity *container, int idx)
{
	return ywCntGetEntry(container, idx);
}
