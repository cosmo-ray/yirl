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
#include "contener.h"
#include "widget-callback.h"

static int t = -1;

int ywContenerUpdate(Entity *contener, Entity *widEnt)
{
  Entity *entries = yeGet(contener, "entries");

  YE_ARRAY_FOREACH(entries, tmp) {
    YWidgetState *wid;

    wid = ywidGetState(tmp);
    if ((tmp == widEnt) ||
	(wid->type == t && ywContenerUpdate(tmp, widEnt))) {
      wid->hasChange = 1;
      wid = ywidGetState(contener);
      wid->hasChange = 1;
      return 1;
    }
  }
  return 0;
}

Entity *ywContenerGetWidgetAt(Entity *contener, int posX, int posY)
{
  Entity *entries = yeGet(contener, "entries");

  YE_ARRAY_FOREACH(entries, tmp) {
    if (ywIsPixsOnWid(tmp, posX, posY))
      return tmp;
  }
  return NULL;
}

Entity *ywCntGetEntry(Entity *contener, int idx)
{
  return yeGetByIdx(yeGetByStrFast(contener, "entries"), idx);
}

static inline CntType cntGetTypeFromEntity(Entity *entity) {
  const char *cntType = yeGetString(yeGet(entity, "cnt-type"));

  if (yuiStrEqual0(cntType, "vertical")) {
    return CNT_VERTICAL;
  } else if (yuiStrEqual0(cntType, "stacking"))
    return CNT_STACK;
  return CNT_HORIZONTAL;
}

static inline Entity *getEntry(Entity *father, Entity *tmp)
{
  if (!yeGet(tmp, "<type>"))
    return yeFindLink(father, yeGetString(yeGet(tmp, "name")), 0);
  return tmp;
}

static void cntResize(YWidgetState *opac)
{
  Entity *entity = opac->entity;
  Entity *entries = yeGet(entity, "entries");
  YContenerState *cnt = ((YContenerState *)opac);
  Entity *pos = yeGet(entity, "wid-pos");
  int i = 0;
  size_t len = yeLen(entries);
  int widSize = 0;
  int usable;
  int casePos = 0;
  int caseLen = 0;
  cnt->type = cntGetTypeFromEntity(entity);

  widSize =  ywCntType(opac) == CNT_HORIZONTAL ?
    yeGetInt(yeGet(pos, "h")) : yeGetInt(yeGet(pos, "w"));
  usable = widSize;

  YE_ARRAY_FOREACH(entries, tmp) {
    YWidgetState *wid = ywidGetState(tmp);
    Entity *ptr;
    Entity *tmpPos;
    int size;

    if (unlikely(!wid))
      continue;

    ptr = wid->entity;
    size = yeGetInt(yeGet(tmp, "size"));
    tmpPos = yeReplaceBack(ptr, pos, "wid-pos");
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
  Entity *bg_tx;

  (void)args;
  if (!entries)
    return -1;

  if (bg) {
    bg_tx = yeCreateArray(entity, "$bg");
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

    if (yeGet(tmp, "copy")) {
      Entity *copyTmp;

      copyTmp = yeCreateArray(tmp, "$copy");
      yeCopy(ptr, copyTmp);
      ptr = copyTmp;
    }
    yeReplaceBack(ptr, entity, "$father-contener");
    wid = ywidNewWidget(ptr, NULL);
    if (!wid) {
      DPRINT_ERR("fail to create a sub widget");
      return -1;
    }
    if (ptr != tmp)
      yeReCreateData(wid, tmp, "$wid");
  }
  cntResize(opac);
  return 0;
}

static int cntDestroy(YWidgetState *opac)
{
  Entity *entries = yeGet(opac->entity, "entries");

  YE_ARRAY_FOREACH(entries, tmp) {
    YWidgetState *cur = ywidGetState(tmp);

    YWidDestroy(cur);
  }
  g_free(opac);
  return 0;
}

#define yCntIsOverloading(eve) 0

static InputStatue cntEvent(YWidgetState *opac, YEvent *event)
{
  InputStatue ret = NOTHANDLE;
  Entity *entries = yeGet(opac->entity, "entries");
  YWidgetState *cur;

  ret = ywidCallSignal(opac, event, NULL, opac->actionIdx);
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

  if (bg_wid) {
    yeReplaceBack(bg_wid->entity, yeGet(opac->entity, "wid-pos"), "wid-pos");
    ywidRend(bg_wid);
  }

  YE_ARRAY_FOREACH(entries, tmp) {
    YWidgetState *wid = ywidGetState(tmp);

    /* try to create the widget */
    if (!wid) {
      Entity *tmp2 = getEntry(opac->entity, tmp);

      wid = ywidNewWidget(tmp2, NULL);
      if (!wid)
	continue;
      if (tmp2 != tmp)
	yeReCreateData(wid, tmp, "$wid");
    }
    wid->shouldDraw = 0;
    if (needChange)
      wid->hasChange = 1;
    else if (ywCntType(opac) == CNT_STACK && wid->hasChange)
      needChange = 1;

    ywidRend(wid);
    wid->hasChange = 0;
    wid->shouldDraw = 1;
  }
  ywidDrawScreen();
  return 0;
}

static void cntMidRend(YWidgetState *opac, int percent)
{
  Entity *entries = yeGet(opac->entity, "entries");

  YE_ARRAY_FOREACH(entries, tmp) {
    YWidgetState *wid = ywidGetState(tmp);

    if (!wid)
      continue;
    ywidMidRend(wid, percent);
  }
  ywidDrawScreen();
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
  YContenerState *ret = g_new0(YContenerState, 1);
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

int ywContenerInit(void)
{
  if (t != -1)
    return t;
  t = ywidRegister(alloc, "contener");
  return t;
}

int ywContenerEnd(void)
{
  if (ywidUnregiste(t) < 0)
    return -1;
  t = -1;
  return 0;
}
