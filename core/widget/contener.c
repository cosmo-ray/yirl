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

static void widDestroyWrapper(void *wid)
{
  YWidDestroy(wid);
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

static int cntInit(YWidgetState *opac, Entity *entity, void *args)
{
  YContenerState *cnt = ((YContenerState *)opac);
  Entity *entries = yeGet(entity, "entries");
  Entity *pos = yeGet(entity, "wid-pos");
  int i = 0;
  size_t len = yeLen(entries);
  int widSize = 0;
  int usable;
  int casePos = 0;
  int caseLen = 0;

  (void)args;
  if (!entries)
    return -1;
  cnt->type = cntGetTypeFromEntity(entity);

  widSize =  ywCntType(opac) == CNT_HORIZONTAL ?
    yeGetInt(yeGet(pos, "h")) : yeGetInt(yeGet(pos, "w"));
  usable = widSize;

  yeCreateInt(0, entity, "current");
  YE_ARRAY_FOREACH(entries, tmp) {
    Entity *ptr = getEntry(entity, tmp);

    yeReplaceBack(ptr, entity, "$father-contener");
    YWidgetState *wid = ywidNewWidget(ptr, NULL);
    Entity *widData = yeCreateData(wid, tmp, "$wid");
    Entity *tmpPos = yeGet(ptr, "wid-pos");
    int size = yeGetInt(yeGet(tmp, "size"));

    if (!wid)
      continue;
    if (size < 0) { /* We equally size the sub-widgets */
      caseLen = usable * (i + 1) / len;
    } else {
      caseLen = usable * size / 100;
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
    yeSetDestroy(widData, widDestroyWrapper);
    usable -= caseLen;
    casePos = widSize - usable;
    ++i;
  }

  return 0;
}
#undef yCntType

static int cntDestroy(YWidgetState *opac)
{
  g_free(opac);
  return 0;
}

#define yCntIsOverloading(eve) 0

static InputStatue cntEvent(YWidgetState *opac, YEvent *event)
{
  InputStatue ret = NOTHANDLE;
  Entity *entries = yeGet(opac->entity, "entries");
  YWidgetState *cur;
  
  (void)opac;
  if (!event)
    event = ywidGenericWaitEvent();

  if (!event)
    return NOTHANDLE;

  ywidCallSignal(opac, event, NULL,
		 opac->actionIdx);
  ret = event->stat;

  if (ret != NOTHANDLE)
    return ret;

  cur = yeGetData(yeGet(
			yeGet(entries, yeGetInt(yeGet(opac->entity,
						      "current"))),
			"$wid"));
  if (cur)
    return ywidHandleEvent(cur, event);
  return ret;
}

static int cntRend(YWidgetState *opac)
{
  Entity *entries = yeGet(opac->entity, "entries");
  int needChange = 0;

  if (!opac->hasChange)
    return 0;
  YE_ARRAY_FOREACH(entries, tmp) {
    YWidgetState *wid = yeGetData(yeGet(tmp, "$wid"));

    if (!wid)
      continue;
    if (needChange)
      wid->hasChange = 1;
    else if (ywCntType(opac) == CNT_STACK && wid->hasChange)
      needChange = 1;

    ywidRend(wid);
    wid->hasChange = 0;
  }
  return 0;
}

static void *alloc(void)
{
  YContenerState *ret = g_new0(YContenerState, 1);
  YWidgetState *wstate = (YWidgetState *)ret;

  wstate->render = cntRend;
  wstate->init = cntInit;
  wstate->destroy = cntDestroy;
  wstate->handleEvent = cntEvent;
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
