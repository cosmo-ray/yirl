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

#include <ctype.h>
#include "menu.h"
#include "container.h"
#include "rect.h"
#include "native-script.h"
#include "entity-script.h"
#include "sdl2/canvas-sdl.h"
#include "game.h"

static int t = -1;

typedef struct {
	YWidgetState sate;
	int current;
	int threshold;
	int shift_push;
} YMenuState;

static void *MoveOn(YWidgetState *wid, uint32_t at)
{
	YMenuState *s = (YMenuState *)wid;
	Entity *ent = wid->entity;
	Entity *moveOn = yeGet(wid->entity, "moveOn");

	if (yeStrCmp(yeGet(ent, "mn-type"), "panel")) {
		int y = 1;
		Entity *entries = yeGet(ent, "entries");
		Entity *wid_size = yeGet(ent, "wid-pix");
		int wid_h = ywRectH(wid_size);
		Entity *pre_txt = yeGet(ent, "pre-text");

		if (pre_txt) {
			int txt_cnt = 1 + yeCountCharacters(pre_txt, '\n', -1);

			y += (txt_cnt * sgGetTxtH());
		}

		for (uint32_t i = 0; i <= at; ++i) {
			Entity *entry = yeGet(entries, i);
			/* Entity *e_r = yeGet(entry, "$rect"); */
			/* int entry_h = ywRectH(e_r); */
			if (yeGetInt(yeGet(entry, "hiden")))
				continue;
			y += sgGetTxtH();
		}

		if (s->threshold + y > wid_h) {
			s->threshold = wid_h - y;
			// I think we need to roung up
			/* printf("- %d - ", s->threshold % sgGetFontSize()); */
			/* s->threshold += s->threshold % sgGetFontSize(); */
		}
		/* first we check the botom of last entrym then the top */
		y -= sgGetTxtH() + 1;
		if (s->threshold + y - (int)sgGetTxtH() + 1 < 0) {
			s->threshold -= (s->threshold + y);
		}
	}

	if (moveOn)
		return yesCall(moveOn, ent, at,
			       ywMenuGetCurrentEntry(ent));
	return (void *)ACTION;
}

void *ywMenuMove(Entity *ent, uint32_t at)
{
	YMenuState *s = (YMenuState *)ywidGetState(ent);

	if (at > yeLen(yeGet(ent, "entries")))
		return (void *)NOTHANDLE;
	s->current = at;
	return MoveOn((YWidgetState *)s, at);
}

static void *mn_up_down(YWidgetState *wid, int to_add)
{
	Entity *cur = ywMenuGetCurrentEntry(wid->entity);
	Entity *subentries = yeGet(cur, "subentries");
	int len = yeLen(yeGet(wid->entity, "entries"));
	if (subentries) {
		if (!yeGetIntAt(cur, "is-click"))
			goto out_sub;
		yeAddAt(cur, "slider_idx", to_add);
		int cur_sub_idx = yeGetIntAt(cur, "slider_idx");
		/* -1 mean it was 0, so we need to be pos on the submenu button */
		if (cur_sub_idx == -1)
			goto skip_add;
		else if (cur_sub_idx == -2) {
			yeSetAt(cur, "slider_idx", -1);
			goto out_sub;
		} else if (cur_sub_idx >= yeLeni(subentries)) {
			yeSetAt(cur, "slider_idx", -1);
			goto out_sub;
		}
		return NULL;
	out_sub:
	}
	((YMenuState *)wid)->current += to_add;
	if (((YMenuState *)wid)->current > len - 1)
		((YMenuState *)wid)->current = 0;
	else if (((YMenuState *)wid)->current < 0) {
		((YMenuState *)wid)->current = len - 1;
	}
	cur = ywMenuGetCurrentEntry(wid->entity);
	subentries = yeGet(cur, "subentries");
	if (to_add < 0 && subentries && yeGetIntAt(cur, "is-click")) {
		yeSetAt(cur, "slider_idx", yeLen(subentries) - 1);
	}
skip_add:
	if (yeGetInt(yeGet(ywMenuGetCurrentEntry(wid->entity), "hiden")))
		return mn_up_down(wid, to_add);
	return MoveOn(wid, ((YMenuState *)wid)->current);

}

static void *nmMenuDown(YWidgetState *wid)
{
	return mn_up_down(wid, 1);
}

static void *nmMenuUp(YWidgetState *wid)
{
	return mn_up_down(wid, -1);
}

static void *nmMenuMove(int nb, union ycall_arg *args, int *types)
{
	Entity *wid = args[0].e;
	Entity *eve = args[1].e;
	Entity *cu = ywMenuGetCurrentEntry(wid);
	int cur_k = ywidEveKey(eve);
	Entity *s;

	if ((s = yeGet(cu, "slider"))) {
		Entity *si = yeGet(cu, "slider_idx");

		if (cur_k == Y_RIGHT_KEY || cur_k == 'd') {
			yeAdd(si, 1);
		} else if (cur_k == Y_LEFT_KEY || cur_k == 'a') {
			yeAdd(si, -1);
		}
		if (yeGetInt(si) < 0)
			yeSetInt(si, yeLen(s) - 1);
		else if ((unsigned int)yeGetInt(si) > (yeLen(s) - 1))
			yeSetInt(si, 0);
	}

	if (cur_k == Y_DOWN_KEY || cur_k == 's') {
		return nmMenuDown(ywidGetState(wid));
	} else if (cur_k == Y_UP_KEY || cur_k == 'w') {
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

_Bool ywMenuRemoveLastEntry(Entity *menu)
{
	YMenuState *wid = (void *)ywidGetState(menu);
	Entity *entries = yeGet(menu, "entries");

	yePopBack(entries);
	if (wid->current == yeLeni(entries))
		--wid->current;
	return 1;
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

static void *nmMenuMainWidNext(int nb, union ycall_arg *args, int *types)
{
	Entity *e = args[0].e;
	YWidgetState *wid = ywidGetState(e);
	Entity *next = yeGet(e, "entries");

	next = yeGet(next, ((YMenuState *)wid)->current);
	next = yeGet(next, "next");

	return ywidNext(next, NULL) ? (void *)BUG : (void *)ACTION;
}


InputStatue mnActions_(Entity *menu, Entity *event, Entity *current_entry)
{
	Entity *s;

	if ((s = yeGet(current_entry, "slider")) || (s = yeGet(current_entry, "subentries"))) {
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

	return (void *)(intptr_t)mnActions_(wid, eve, ywMenuGetCurrentEntry(wid));
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
	((YMenuState *)opac)->threshold = 0;
	return 0;
}

static int mnDestroy(YWidgetState *opac)
{
	free(opac);
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
	return ret;
}

static InputStatue mnEvent(YWidgetState *opac, Entity *event)
{
	InputStatue ret = NOTHANDLE;
	YMenuState *mnstate = (void *)opac;

	if (!event)
		return NOTHANDLE;

	if (ywidEveType(event) == YKEY_DOWN) {
		Entity *on = yeGet(opac->entity, "on");
		Entity *cur_entry = ywMenuGetCurrentEntry(opac->entity);
		Entity *input_text = yeGet(cur_entry, "input-txt");
		Entity *on_entry;
		int cur_k;

		YE_FOREACH(on, on_entry) {
			if (yeGetIntAt(on_entry, 0) == ywidEveKey(event)) {
				return (InputStatue)(intptr_t)yesCall(
					yeGet(on_entry, 1), opac->entity,
					mnstate->current,
					ywMenuGetCurrentEntry(opac->entity));
			}
		}
		cur_k = ywidEveKey(event);

		if (cur_k == Y_LSHIFT_KEY || cur_k == Y_RSHIFT_KEY) {
			mnstate->shift_push = 1;
		}
		if (cur_k == Y_ESC_KEY) {
			Entity *onEsc = yeGet(opac->entity, "onEsc");

			if (onEsc) {
				return (InputStatue)(intptr_t)yesCall(
					onEsc,
					opac->entity,
					((YMenuState *)opac)->current,
					ywMenuGetCurrentEntry(opac->entity));
			}
		} else if (input_text && cur_k == '\b') {
			if (yeLen(input_text)) {
				yeStringTruncate(input_text, 1);
			}
		} else if (input_text && cur_k < 255 &&
			   (isalnum(cur_k) || cur_k == ' ' || cur_k == '_')) {
			Entity *wid_size = yeGet(opac->entity, "wid-pix");
			int wid_w = ywRectW(wid_size);
			/* -2 is kind of inpartial, 'caus ther's margin and shit */
			size_t max_char = wid_w / sgGetTxtW() - 2;

			if (yeLen(input_text) + yeLenAt(cur_entry, "text") > max_char)
				goto skip_txt;
			if (mnstate->shift_push && isalpha(cur_k))
				yeStringAddCh(input_text, cur_k + ('A' - 'a'));
			else
				yeStringAddCh(input_text, cur_k);
		skip_txt:
		} else if (cur_k == '\n' || cur_k == ' ') {
			/* is-click use here, to know if we have subentries */
			Entity *is_click = yeGet(cur_entry, "is-click");
			int sld_idx = yeGetIntAt(cur_entry, "slider_idx");
			if (sld_idx < 0 && is_click) {
				yeSet(is_click, !yeGetInt(is_click));
			} else {
				ret = ywMenuCallActionOn(opac, event,
							 ((YMenuState *)opac)->current);
			}
		} else {
 			ret = (InputStatue)(intptr_t)yesCall(yeGet(opac->entity, "move"),
						   opac->entity, event);
		}
	} else if (ywidEveType(event) == YKEY_MOUSEDOWN) {
		ret = ywMenuCallActionOn(
			opac, event,
			ywMenuPosFromPix(opac->entity,
					 ywPosX(ywidEveMousePos(event)),
					 ywPosY(ywidEveMousePos(event))));
	} else if (ywidEveType(event) == YKEY_MOUSEMOTION) {
		ywMenuMove(opac->entity,
			   ywMenuPosFromPix(opac->entity,
					    ywPosX(ywidEveMousePos(event)),
					    ywPosY(ywidEveMousePos(event))));
	} else if (ywidEveType(event) == YKEY_UP) {
		int cur_k = ywidEveKey(event);
		if (cur_k == Y_LSHIFT_KEY || cur_k == Y_RSHIFT_KEY) {
			mnstate->shift_push = 0;
		}
	}
	return ret;
}


static void *alloc(void)
{
	YMenuState *ret = y_new0(YMenuState, 1);
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
		if (ywRectContain(rect, x - ywRectX(pos),
				  y - ywRectY(pos),
				  1))
			return it.pos;
	}
	return -1;
}

Entity *ywMenuPushEntryByEnt(Entity *menu, const char *name, Entity *func)
{
	Entity *entries = yeTryCreateArray(menu, "entries");
	Entity *entry = yeCreateArray(entries, name);

	yeCreateString(name, entry, "text");
	yePushBack(entry, func, "action");
	return entry;
}

Entity *ywMenuPushSlideDownSubMenu(Entity *menu, const char *name, Entity *subentries)
{
	Entity *entries = yeTryCreateArray(menu, "entries");
	Entity *entry = yeCreateHash(entries, name);

	yeCreateString(name, entry, "text");
	if (subentries)
		yePushBack(entry, subentries, "subentries");
	else
		yeCreateVector(entry, "subentries");
	yeCreateInt(0, entry, "is-click");
	yeCreateInt(-1, entry, "slider_idx");
	return entry;
}

Entity *ywMenuPushSlider(Entity *menu, const char *name, Entity *slider_array)
{
	Entity *entries = yeTryCreateArray(menu, "entries");
	Entity *entry = yeCreateArray(entries, name);

	yeCreateString(name, entry, "text");
	yePushBack(entry, slider_array, "slider");
	yeCreateInt(0, entry, "slider_idx");
	return entry;

}

Entity *ywMenuPushTextInput(Entity *menu, const char *name)
{
	Entity *entries = yeTryCreateArray(menu, "entries");
	Entity *entry = yeCreateHash(entries, name);

	yeCreateString(name, entry, "text");
	yeCreateString("", entry, "input-txt");
	yeCreateInt(0, entry, "cursor-pos");
	return entry;
}

int ywMenuGetCurrent(YWidgetState *opac)
{
	return ((YMenuState *)opac)->current;
}

int ywMenuGetThreshold(YWidgetState *opac)
{
	return ((YMenuState *)opac)->threshold;
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

void ywMenuClear(Entity *menu)
{
	YMenuState *m_state = (YMenuState *)ywidGetState(menu);
	Entity *entries = yeGet(menu, "entries");

	yeClearArray(entries);
	if (m_state)
		m_state->current = 0;
}

int ywMenuInit(void)
{
	if (t != -1)
		return t;
	t = ywidRegister(alloc, "menu");
	ysRegistreNativeFunc("menuMove", nmMenuMove);
	ysRegistreNativeFunc("panelMove", nmPanelMove);
	ysRegistreNativeFunc("menuNext", nmMenuNext);
	ysRegistreNativeFunc("menuMainWidNext", nmMenuMainWidNext);
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
