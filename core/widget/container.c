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
      wid->hasChange = 1;
      wid = ywidGetState(container);
      wid->hasChange = 1;
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

Entity *ywCntPopLastEntry(Entity *container)
{
  yePopBack(yeGet(container, "entries"));
  return ywCntGetLastEntry(container);
}

Entity *ywCntGetEntry(Entity *container, int idx)
{
  return yeGet(yeGet(container, "entries"), idx);
}

int ywReplaceEntry(Entity *container, int idx, Entity *entry)
{
  Entity *entries = yeGet(container, "entries");
  Entity *old = ywCntGetEntry(container, idx);
  Entity *new = getEntry(container, entry);
  int size = yeGetIntAt(old, "size");

  yeReCreateInt(size, new, "size");
  yeReplace(entries, old, new);
  return 0;
}

static inline CntType cntGetTypeFromEntity(Entity *entity) {
  const char *cntType = yeGetString(yeGet(entity, "cnt-type"));

  if (yuiStrEqual0(cntType, "vertical")) {
    return CNT_VERTICAL;
  } else if (yuiStrEqual0(cntType, "stacking"))
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
  int ret = yeLen(entries);

  if (yePushBack(entries, wid, NULL) < 0)
    return -1;
  if (dec_ref)
    yeDestroy(wid);
  yeSetAt(container, "current", ret);
  yeReplaceBackExt(wid, container, "$father-container", YE_FLAG_NO_COPY);
  return ret;
}

static void cntResize(YWidgetState *opac)
{
  Entity *entity = opac->entity;
  Entity *entries = yeGet(entity, "entries");
  YContainerState *cnt = ((YContainerState *)opac);
  Entity *pos = yeGet(entity, "wid-pos");
  int i = 0;
  size_t len = yeLen(entries);
  int widSize = 0;
  int usable;
  int casePos = 0;
  int caseLen = 0;
  Entity *bg = yeGet(entity, "$bg");
  cnt->type = cntGetTypeFromEntity(entity);

  widSize =  ywCntType(opac) == CNT_HORIZONTAL ?
    yeGetInt(yeGet(pos, "h")) : yeGetInt(yeGet(pos, "w"));
  usable = widSize;
  if (bg) {
    yeReplaceBack(bg, pos, "wid-pos");
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
    tmpPos = ywRectReCreateEnt(pos, ptr, "wid-pos");
    size = yeGetInt(yeGet(tmp, "size"));
    if (size <= 0) { /* We equally size the sub-widgets */
      caseLen = usable * (i + 1) / len;
    } else {
      caseLen = widSize * size / 100;
    }

    if (ywCntType(opac) == CNT_HORIZONTAL) {
      /* modify y and h pos in internal struct */
      yeSetAt(tmpPos, "y", casePos);
      yeSetAt(tmpPos, "h", caseLen);
    } else if (ywCntType(opac) == CNT_VERTICAL) {
      /* modify x and w pos in internal struct */
      yeSetAt(tmpPos, "x", casePos);
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
  YWidgetState *wid;

  (void)args;
  if (!entries) {
    DPRINT_ERR("no entries");
    return -1;
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

  yeTryCreateInt(0, entity, "current");
  YE_ARRAY_FOREACH(entries, tmp) {
    Entity *ptr = getEntry(entity, tmp);
    Entity *copyTmp = NULL;

    if (yeGet(tmp, "copy")) {
      copyTmp = yeCreateArray(NULL, NULL);
      yeCopy(ptr, copyTmp);
      ptr = copyTmp;
    }
    yeReplaceBackExt(ptr, entity, "$father-container", YE_FLAG_NO_COPY);
    if (ptr != tmp) {
      YE_ARRAY_FOREACH_EXT(tmp, entry, it) {
	const char *n = yBlockArrayIteratorGetPtr(it, ArrayEntry)->name;
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
    yeDestroy(bg);
  }
  YE_ARRAY_FOREACH(entries, tmp) {
    YWidgetState *cur = ywidGetState(tmp);

    yeRemoveChildByStr(tmp, "$father-container");
    YWidDestroy(cur);
    if (yeGet(tmp, "copy")) {
      yeDestroy(tmp);
    }
  }
  g_free(opac);
  return 0;
}

#define yCntIsOverloading(eve) 0

static InputStatue cntEvent(YWidgetState *opac, Entity *event)
{
  InputStatue ret;
  Entity *entries = yeGet(opac->entity, "entries");
  YWidgetState *cur;

  ret = ywidEventCallActionSin(opac, event);
  if (ret != NOTHANDLE)
    return ret;

  cur = ywidGetState(yeGet(entries, yeGetInt(yeGet(opac->entity, "current"))));
  if (cur)
    ret = ywidHandleEvent(cur, event);
  return ret;
}

static int cntRend(YWidgetState *opac)
{
  Entity *entries = yeGet(opac->entity, "entries");
  int needChange = 0;
  YWidgetState *bg_wid = ywidGetState(yeGet(opac->entity, "$bg"));

  if (!opac->hasChange)
    return 0;

  if (opac->hasChange == 2 || ywCntType(opac) == CNT_STACK)
    needChange = 1;

  if (bg_wid) {
    yeReplaceBack(bg_wid->entity, yeGet(opac->entity, "wid-pos"), "wid-pos");
    bg_wid->hasChange = 1;
    ywidSubRend(bg_wid);
    needChange = 1;
  }

  YE_ARRAY_FOREACH(entries, tmp) {
    YWidgetState *wid = ywidGetState(tmp);

    /* try to create the widget */
    if (unlikely(!wid)) {
      Entity *tmp2 = getEntry(opac->entity, tmp);

      if (tmp2 != tmp) {
	yeReplace(entries, tmp, tmp2);
	tmp = tmp2;
      }
      yeReplaceBackExt(tmp, opac->entity, "$father-container", YE_FLAG_NO_COPY);
      wid = ywidNewWidget(tmp, NULL);
      if (!wid)
	continue;
      cntResize(opac);
    }
    if (needChange)
      wid->hasChange = 2;

    ywidSubRend(wid);
    wid->hasChange = 0;
  }
  return 0;
}

static void cntMidRend(YWidgetState *opac, int percent)
{
  Entity *entries = yeGet(opac->entity, "entries");

  YE_ARRAY_FOREACH(entries, tmp) {
    YWidgetState *wid = ywidGetState(tmp);

    if (!wid)
      continue;

    ywidSubMidRend(wid, percent);
    if (wid->hasChange) {
      opac->hasChange = 1;
    }
  }
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
  YContainerState *ret = g_new0(YContainerState, 1);
  YWidgetState *wstate = (YWidgetState *)ret;

  wstate->render = cntRend;
  wstate->init = cntInit;
  wstate->destroy = cntDestroy;
  wstate->midRendEnd = midRendEnd;
  wstate->midRend = cntMidRend;
  wstate->handleEvent = cntEvent;
  wstate->resize = cntResize;
  wstate->type = t;
  ret->curent = -1;
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
