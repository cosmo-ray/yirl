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

#include "container.h"
#include "game.h"
#include "yirl/pos.h"
#include "yirl/rect.h"

static int t = -1;

static inline Entity *getEntry(Entity *father, Entity *tmp)
{
	if (!yeGet(tmp, "<type>"))
		return ygGet(yeGetString(yeGet(tmp, "path")));
	return tmp;
}

int ywContainerUpdate(Entity *container, Entity *widEnt)
{
	Entity *entries = yeGet(container, "entries");

	YE_ARRAY_FOREACH(entries, tmp) {
		YWidgetState *wid;

		wid = ywidGetState(tmp);
		if ((tmp == widEnt) ||
		    (wid->type == t && ywContainerUpdate(tmp, widEnt))) {
			wid = ywidGetState(container);
			return 1;
		}
	}
	return 0;
}

Entity *ywContainerGetWidgetAt(Entity *container, int posX, int posY)
{
	Entity *entries = yeGet(container, "entries");

	YE_ARRAY_FOREACH(entries, tmp) {
		if (ywIsPixsOnWid(tmp, posX, posY))
			return tmp;
	}
	return NULL;
}

Entity *ywCntGetLastEntry(Entity *container)
{
	return yeGetLast(yeGet(container, "entries"));
}

void ywCntPopLastEntry(Entity *container)
{
	Entity *ret = ywCntGetLastEntry(container);
	Entity *entries;
	Entity *cur;

	if (!ret)
		return;
	cur = yeGet(container, "current");
	entries = yeGet(container, "entries");
	if (ywidGetState(ret)) {
		ywidMarkAsDestroyable(ywidGetState(ret));
		yeCreateInt(1, ret, "need_yedestroy");
		yeIncrRef(ret);
	}
	yePopBack(entries);
	if (yeGetInt(cur) && yeLen(entries) == yeGetUInt(cur)) {
		yeSubInt(cur, 1);
	}
}

Entity *ywCntGetEntry(Entity *container, int idx)
{
	Entity *entries = yeGet(container, "entries");
	if (idx < 0)
		return yeGet(entries, yeLen(entries) + idx);
	return yeGet(entries, idx);
}

int ywRemoveEntryByEntity(Entity *container, Entity *target)
{
	Entity *le = ywCntGetLastEntry(container);
	Entity *ets = yeGet(container, "entries");

	if (target == le)
		goto out;
	if (yeSwapElems(ets, target, le) < 0)
		return -1;
out:
	ywCntPopLastEntry(container);
	return 0;
}

int ywReplaceEntry(Entity *container, int idx, Entity *entry)
{
	Entity *entries = yeGet(container, "entries");
	Entity *old = ywCntGetEntry(container, idx);
	Entity *new = getEntry(container, entry);
	int size = yeGetIntAt(old, "size");

	yeIncrRef(old);
	ywidMarkAsDestroyable(ywidGetState(old));
	yeReCreateInt(size, new, "size");
	yeReplace(entries, old, new);
	return 0;
}

static inline CntType cntGetTypeFromEntity(Entity *entity) {
	const char *cntType = yeGetString(yeGet(entity, "cnt-type"));

	if (yuiStrEqual0(cntType, "vertical")) {
		return CNT_VERTICAL;
	} else if (yuiStrEqual0(cntType, "stacking") ||
		   yuiStrEqual0(cntType, "stack"))
		return CNT_STACK;
	return CNT_HORIZONTAL;
}

Entity *ywCntWidgetFather(Entity *wid)
{
	return yeGet(wid, "$father-container");
}

int ywPushNewWidget(Entity *container, Entity *wid, int dec_ref)
{
	Entity *entries = yeGet(container, "entries");
	int ret;

	if (unlikely(!entries)) {
		entries = yeCreateArray(container, "entries");
	}
	ret = yeLen(entries);

	if (yePushBack(entries, wid, NULL) < 0)
		return -1;
	if (dec_ref)
		yeDestroy(wid);
	yeReCreateInt(ret, container, "current");
	yeReplaceBackExt(wid, container, "$father-container", YE_FLAG_NO_COPY);
	return ret;
}

static void cntResize(YWidgetState *opac)
{
	Entity *entity = opac->entity;
	Entity *entries = yeGet(entity, "entries");
	YContainerState *cnt = ((YContainerState *)opac);
	Entity *rect = yeGet(entity, "wid-pos");
	int i = 0;
	size_t len = yeLen(entries);
	int widSize = 0;
	int usable;
	int casePos = 0;
	int caseLen = 0;
	Entity *bg = yeGet(entity, "$bg");
	cnt->type = cntGetTypeFromEntity(entity);

	widSize =  ywCntType(opac) == CNT_HORIZONTAL ?
		ywRectH(rect) : ywRectW(rect);
	usable = widSize;

	if (bg) {
		yeReplaceBack(bg, rect, "wid-pos");
		ywidResize(ywidGetState(bg));
	}

	YE_ARRAY_FOREACH(entries, tmp) {
		YWidgetState *wid = ywidGetState(tmp);
		Entity *ptr;
		Entity *tmpPos;
		int size;

		if (unlikely(!wid))
			continue;

		ptr = wid->entity;
		tmpPos = ywRectReCreateEnt(rect, ptr, "wid-pos");
		size = yeGetInt(yeGet(tmp, "size"));
		if (size <= 0) { /* We equally size the sub-widgets */
			caseLen = usable * (i + 1) / len;
		} else {
			caseLen = widSize * size / 100;
		}

		if (ywCntType(opac) == CNT_HORIZONTAL) {
			/* modify y and h pos in internal struct */
			ywPosAddXY(tmpPos, 0, casePos);
			yeSetAt(tmpPos, "h", caseLen);
		} else if (ywCntType(opac) == CNT_VERTICAL) {
			/* modify x and w pos in internal struct */
			ywPosAddXY(tmpPos, casePos, 0);
			yeSetAt(tmpPos, "w", caseLen);
		}

		/* else nothing */
		ywidResize(wid);
		usable -= caseLen;
		casePos = widSize - usable;
		++i;
	}
}

static int cntInit(YWidgetState *opac, Entity *entity, void *args)
{
	Entity *entries = yeGet(entity, "entries");
	Entity *bg = yeGet(entity, "background");
	Entity *eve_forwarding = yeGet(entity, "event_forwarding");
	YWidgetState *wid;
	YContainerState *cntState = (YContainerState *)opac;

	(void)args;
	if (!entries) {
		entries = yeCreateArray(entity, "entries");
	}

	if (bg) {
		Entity *bg_tx = yeCreateArray(entity, "$bg");

		yeCreateString("text-screen", bg_tx, "<type>");
		yeCreateString("", bg_tx, "text");
		yePushBack(bg_tx, bg, "background");
		if (!ywidNewWidget(bg_tx, NULL)) {
			DPRINT_ERR("background init fail");
			return -1;
		}
	}

	if (eve_forwarding && !yeStrCmp(eve_forwarding, "under mouse")) {
		cntState->fwStyle = Y_CNT_UNDER_MOUSE;
	}

	yeTryCreateInt(0, entity, "current");
	YE_ARRAY_FOREACH(entries, tmp) {
		Entity *ptr = getEntry(entity, tmp);
		Entity *copyTmp = NULL;

		if (yeGet(tmp, "copy")) {
			copyTmp = yeCreateArray(NULL, NULL);
			yeCopy(ptr, copyTmp);
			ptr = copyTmp;
		}
		yeReplaceBackExt(ptr, entity, "$father-container",
				 YE_FLAG_NO_COPY);
		if (ptr != tmp) {
			YE_ARRAY_FOREACH_EXT(tmp, entry, it) {
				const char *n =
					yBlockArrayIteratorGetPtr(
						it, ArrayEntry)->name;

				if (yuiStrEqual0(n, "name"))
					continue;
				yePushBack(ptr, entry, n);
			}
			yeReplace(entries, tmp, ptr);
		}
		yeDestroy(copyTmp);
		wid = ywidNewWidget(ptr, NULL);
		if (!wid) {
			DPRINT_ERR("fail to create a sub widget");
			return -1;
		}
		if (ptr != tmp)
			yeReCreateData(wid, ptr, "$wid");
	}
	cntResize(opac);
	return 0;
}

static int cntDestroy(YWidgetState *opac)
{
	Entity *entries = yeGet(opac->entity, "entries");
	Entity *bg = yeGet(opac->entity, "$bg");

	if (bg) {
		YWidDestroy(yeGetData(yeGet(bg, "$wid")));
		yeRemoveChild(opac->entity, bg);
	}

	YE_ARRAY_FOREACH(entries, tmp) {
		YWidgetState *cur = ywidGetState(tmp);

		yeRemoveChildByStr(tmp, "$father-container");
		YWidDestroy(cur);
		if (yeGet(tmp, "copy")) {
			yeDestroy(tmp);
		}
	}
	free(opac);
	return 0;
}

#define yCntIsOverloading(eve) 0

static InputStatue cntEvent(YWidgetState *opac, Entity *event)
{
	InputStatue ret;
	Entity *entries = yeGet(opac->entity, "entries");
	YWidgetState *cur;

	ret = ywidActions(opac->entity, opac->entity, event);

	if (ret != NOTHANDLE)
		return ret;

	if (((YContainerState *)opac)->fwStyle == Y_CNT_GOTO_CURRENT) {
		cur = ywidGetState(
			yeGet(entries,
			      yeGetInt(yeGet(opac->entity,
					     "current"))));
	} else {
		cur = ywidGetState(
			ywContainerGetWidgetAt(opac->entity,
					       ywidXMouseGlobalPos,
					       ywidYMouseGlobalPos));
	}
	if (cur)
		ret = ywidHandleEvent(cur, event);
	return ret;

}

_Bool ywCntInTree(Entity *cnt, Entity *widget)
{
	if (!yuiStrEqual0(yeGetStringAt(cnt, "<r_t>"), "container"))
		return 0;

	Entity *entries = yeGet(cnt, "entries");
	YE_FOREACH(entries, tmp) {
		int r = 0;
		if (widget == tmp)
			return 1;
		r = ywCntInTree(tmp, widget);
		if (r)
			return r;
	}
	return 0;
}

void ywCntConstructChilds(Entity *ent)
{
	Entity *entries = yeGet(ent, "entries");
	YWidgetState *opac = ywidGetState(ent);

	if (unlikely(!opac))
		return;

	YE_ARRAY_FOREACH(entries, tmp) {
		YWidgetState *wid = ywidGetState(tmp);

		/* try to create the widget */
		if (unlikely(!wid)) {
			Entity *tmp2 = getEntry(ent, tmp);

			if (tmp2 != tmp) {
				yeReplace(entries, tmp, tmp2);
				tmp = tmp2;
			}
			if (!tmp) {
				DPRINT_ERR("can't init NULL widget");
				continue;
			}
			yeReplaceBackExt(tmp, ent, "$father-container",
					 YE_FLAG_NO_COPY);
			wid = ywidNewWidget(tmp, NULL);
			if (!wid)
				continue;
			if (wid->type == t)
				ywCntConstructChilds(tmp);
			cntResize(opac);
		}
	}
}

static int cntRend(YWidgetState *opac)
{
	Entity *ent = opac->entity;
	Entity *entries = yeGet(ent, "entries");
	YWidgetState *bg_wid = ywidGetState(yeGet(opac->entity, "$bg"));
	Entity *auto_foreground;
	int32_t idx = 0;

	if (bg_wid) {
		yeReplaceBack(bg_wid->entity, yeGet(ent, "wid-pos"), "wid-pos");
		ywidSubRend(bg_wid);
	}

	auto_foreground = yeGet(ent, "auto_foreground");
	YE_ARRAY_FOREACH(entries, tmp) {
		YWidgetState *wid = ywidGetState(tmp);

		/* try to create the widget */
		if (unlikely(!wid)) {
			Entity *tmp2 = getEntry(ent, tmp);

			if (tmp2 != tmp) {
				yeReplace(entries, tmp, tmp2);
				tmp = tmp2;
			}
			yeReplaceBackExt(tmp, ent, "$father-container",
					 YE_FLAG_NO_COPY);
			wid = ywidNewWidget(tmp, NULL);
			if (!wid)
				continue;
			cntResize(opac);
		}
		if (ywCntType(opac) != CNT_STACK && auto_foreground) {
			int current = yeGetIntAt(opac->entity, "current");

			if (current == idx)
				yeRemoveChild(wid->entity, "foreground");
			else
				yeReplaceBack(wid->entity, auto_foreground,
					      "foreground");
			++idx;
		}
		ywidSubRend(wid);
	}
	return 0;
}

static void midRendEnd(YWidgetState *opac)
{
	Entity *entries = yeGet(opac->entity, "entries");

	YE_ARRAY_FOREACH(entries, tmp) {
		YWidgetState *wid = ywidGetState(tmp);
		if (!wid)
			continue;
		ywidMidRendEnd(wid);
	}
}

static void *alloc(void)
{
	YContainerState *ret = y_new0(YContainerState, 1);
	YWidgetState *wstate = (YWidgetState *)ret;

	wstate->render = cntRend;
	wstate->init = cntInit;
	wstate->destroy = cntDestroy;
	wstate->midRendEnd = midRendEnd;
	wstate->handleEvent = cntEvent;
	wstate->resize = cntResize;
	wstate->type = t;
	ret->fwStyle = Y_CNT_GOTO_CURRENT;
	return  ret;
}

int ywContainerInit(void)
{
	if (t != -1)
		return t;
	t = ywidRegister(alloc, "container");
	return t;
}

int ywContainerEnd(void)
{
	if (ywidUnregiste(t) < 0)
		return -1;
	t = -1;
	return 0;
}
