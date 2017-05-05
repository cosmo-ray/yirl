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
#include "widget-callback.h"
#include "menu.h"
#include "rect.h"
#include "native-script.h"
#include "game.h"

static int t = -1;


typedef struct {
  YWidgetState sate;
  unsigned int current;
  int moveSinIdx;
  int actionSin0;
} YMenuState;

static void *nmMenuDown(YWidgetState *wid)
{
  ((YMenuState *)wid)->current += 1;

  if (((YMenuState *)wid)->current > yeLen(yeGet(wid->entity, "entries")) - 1)
    ((YMenuState *)wid)->current = 0;
  return (void *)NOACTION;
}

static void *nmMenuUp(YWidgetState *wid)
{
  ((YMenuState *)wid)->current -= 1;

  if (((YMenuState *)wid)->current > yeLen(yeGet(wid->entity, "entries")))
    ((YMenuState *)wid)->current = yeLen(yeGet(wid->entity, "entries")) - 1;
  return (void *)NOACTION;
}

static void *nmMenuMove(va_list ap)
{
  Entity *wid = va_arg(ap, Entity *);
  YEvent *eve = va_arg(ap, YEvent *);

  if (eve->key == Y_DOWN_KEY) {
    return nmMenuDown(ywidGetState(wid));
  } else if (eve->key == Y_UP_KEY) {
    return nmMenuUp(ywidGetState(wid));
  }
  return (void *)NOTHANDLE;
}

static void *nmPanelMove(va_list ap)
{
  YWidgetState *wid = ywidGetState(va_arg(ap, Entity *));
  YEvent *eve = va_arg(ap, YEvent *);

  if (eve->key == Y_RIGHT_KEY) {
    return nmMenuDown(wid);
  } else if (eve->key == Y_LEFT_KEY) {
    return nmMenuUp(wid);
  }
  return (void *)NOTHANDLE;
}

static void *nmMenuNext(va_list ap)
{
  YWidgetState *wid = ywidGetState(va_arg(ap, Entity *));
  Entity *next = yeGet(wid->entity, "entries");

  next = yeGet(next, ((YMenuState *)wid)->current);
  next = yeGet(next, "next");

  return ywidNext(next) ? (void *)BUG : (void *)ACTION ;
}

int ywMenuReBind(Entity *entity)
{
  YWidgetState *opac = ywidGetState(entity);
  YMenuState *state = (YMenuState *)opac;
  Entity *entries = yeGet(entity, "entries");

  yeRemoveChildByStr(entries, "signals");
  state->moveSinIdx = ywidAddSignal(opac, "move");
  if (!yeStrCmp(yeGet(entity, "mn-type"), "panel")) {
    ygBind(opac, "move", "panelMove");
  } else {
    ygBind(opac, "move", "menuMove");
  }
  state->actionSin0 = state->moveSinIdx + 1;
  YE_ARRAY_FOREACH_EXT(entries, entry, i) {
    char *tmp = g_strdup_printf("action-%d", i.pos);
    int ret = ywidAddSignal(opac, tmp);
    Entity *action;

    g_free(tmp);
    if (ret != state->actionSin0 + i.pos)
      return -1;
    action = yeGet(entry, "action");
    if (action) {
      if (yeType(action) == YFUNCTION)
	ywidBindBySinIdx(opac, ret, action);
      else
	ygBindBySinIdx(opac, ret, yeGetString(yeGet(entry, "action")));
    }
  }
  return 0;
}

static int mnInit(YWidgetState *opac, Entity *entity, void *args)
{
  (void)args;
  ywidGenericCall(opac, t, init);
  return ywMenuReBind(entity);
}

static int mnDestroy(YWidgetState *opac)
{
  g_free(opac);
  return 0;
}

static int mnRend(YWidgetState *opac)
{
  ywidGenericRend(opac, t, render);
  opac->hasChange = 0;
  return 0;
}

InputStatue ywMenuCallActionOnByEntity(Entity *opac, YEvent *event,
				       int idx, void *arg)
{
  YWidgetState *cur = ywidGetState(opac);

  return ywMenuCallActionOnByState(cur, event, idx, arg);
}

InputStatue ywMenuCallActionOnByState(YWidgetState *opac, YEvent *event, int idx,
				      void *arg)
{
  InputStatue ret;

  if (idx < 0)
    return NOTHANDLE;
  ((YMenuState *)opac)->current = idx;

  ret = ywidCallSignal(opac, event, arg,
		       idx + ((YMenuState *)opac)->actionSin0);
  if (ret == NOTHANDLE)
    return NOACTION;
  return ret;
}

static InputStatue mnEvent(YWidgetState *opac, YEvent *event)
{
  InputStatue ret = NOTHANDLE;

  (void)opac;

  if (!event)
    return NOTHANDLE;

  if (ywidEveType(event) == YKEY_DOWN) {
    if (event->key == '\n') {
      ret = ywMenuCallActionOn(opac, event, ((YMenuState *)opac)->current, NULL);
    } else {
      ret = ywidCallSignal(opac, event, NULL,  ((YMenuState *)opac)->moveSinIdx);
    }
  } else if (ywidEveType(event) == YKEY_MOUSEDOWN) {
    ret = ywMenuCallActionOn(opac, event, ywMenuPosFromPix(opac->entity,
							   event->xMouse,
							   event->yMouse),
			     NULL);
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
  return opac->hasChange;
}

int ywMenuPosFromPix(Entity *wid, uint32_t x, uint32_t y)
{
  Entity *entries = yeGet(wid, "entries");
  Entity *pos = yeGet(wid, "wid-pix");

  YE_ARRAY_FOREACH_EXT(entries, entry, it) {
    Entity *rect = yeGet(entry, "$rect");
    if (ywRectIntersect(rect, x - ywRectX(pos), y - ywRectY(pos)))
      return it.pos;
  }
  return -1;
}

int ywMenuGetCurrent(YWidgetState *opac)
{
  return ((YMenuState *)opac)->current;
}

int ywMenuInit(void)
{
  if (t != -1)
    return t;
  t = ywidRegister(alloc, "menu");
  ysRegistreNativeFunc("menuMove", nmMenuMove);
  ysRegistreNativeFunc("panelMove", nmPanelMove);
  ysRegistreNativeFunc("menuNext", nmMenuNext);
  return t;
}

int ywMenuEnd(void)
{
  if (ywidUnregiste(t) < 0)
    return -1;
  t = -1;
  return 0;
}
