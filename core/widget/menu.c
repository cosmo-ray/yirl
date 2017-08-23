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
#include "entity-script.h"
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
  if (yeGetInt(yeGet(ywMenuGetCurrentEntry(wid->entity), "hiden")))
    return nmMenuDown(wid);
  return (void *)NOACTION;
}

static void *nmMenuUp(YWidgetState *wid)
{
  ((YMenuState *)wid)->current -= 1;

  if (((YMenuState *)wid)->current > yeLen(yeGet(wid->entity, "entries")))
    ((YMenuState *)wid)->current = yeLen(yeGet(wid->entity, "entries")) - 1;
  if (yeGetInt(yeGet(ywMenuGetCurrentEntry(wid->entity), "hiden")))
    return nmMenuUp(wid);
  return (void *)NOACTION;
}

static void *nmMenuMove(va_list ap)
{
  Entity *wid = va_arg(ap, Entity *);
  Entity *eve = va_arg(ap, Entity *);

  if (ywidEveKey(eve) == Y_DOWN_KEY) {
    return nmMenuDown(ywidGetState(wid));
  } else if (ywidEveKey(eve) == Y_UP_KEY) {
    return nmMenuUp(ywidGetState(wid));
  }
  return (void *)NOTHANDLE;
}

static void *nmPanelMove(va_list ap)
{
  YWidgetState *wid = ywidGetState(va_arg(ap, Entity *));
  Entity *eve = va_arg(ap, Entity *);

  if (ywidEveKey(eve) == Y_RIGHT_KEY) {
    return nmMenuDown(wid);
  } else if (ywidEveKey(eve) == Y_LEFT_KEY) {
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

  return ywidNext(next) ? (void *)BUG : (void *)ACTION;
}

static void *mnActions(va_list ap)
{
  Entity *wid = va_arg(ap, Entity *);
  Entity *eve = va_arg(ap, Entity *);
  void *arg = va_arg(ap, void *);
  Entity *actions = yeGet(ywMenuGetCurrentEntry(wid), "actions");
  InputStatue ret = NOTHANDLE;

  YE_ARRAY_FOREACH(actions, action) {
    int cur_ret = 0;

    if (yeType(action) == YSTRING) {
      cur_ret = (size_t)yesCall(ygGet(yeGetString(action)), wid, eve, arg);
    } else if (yeType(action) == YFUNCTION) {
      cur_ret = (size_t)yesCall(action, wid, eve, arg);
    } else {
      Entity *arg1 = yeLen(action) > 1 ? yeGet(action, 1) : Y_END_VA_LIST;
      Entity *arg2 = yeLen(action) > 2 ? yeGet(action, 2) : Y_END_VA_LIST;
      Entity *arg3 = yeLen(action) > 3 ? yeGet(action, 3) : Y_END_VA_LIST;

      cur_ret = (size_t)yesCall(ygGet(yeGetString(yeGet(action, 0))),
				wid, eve, arg, arg1, arg2, arg3);
    }
    if (cur_ret > ret)
      ret = cur_ret;
  }
  return (void *)ret;
}

void ywMenuUp(Entity *wid)
{
  nmMenuUp(ywidGetState(wid));
}

void ywMenuDown(Entity *wid)
{
  nmMenuDown(ywidGetState(wid));
}

int ywMenuReBind(Entity *entity)
{
  YWidgetState *opac = ywidGetState(entity);
  YMenuState *state = (YMenuState *)opac;
  Entity *entries = yeGet(entity, "entries");

  yeRemoveChildByStr(entity, "signals");
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
      if (yeType(action) == YFUNCTION) {
	ywidBindBySinIdx(opac, ret, action);
      } else if (yeType(action) == YARRAY) {
	ygBindBySinIdx(opac, ret,
		       yeGetString(yeGet(yeGet(entry, "action"), 0)));
      } else {
	ygBindBySinIdx(opac, ret, yeGetString(yeGet(entry, "action")));
      }
    } else if ((action = yeGet(entry, "actions")) != NULL) {
      ygBindBySinIdx(opac, ret, "menuActions");
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

InputStatue ywMenuCallActionOnByEntity(Entity *opac, Entity *event,
				       int idx, void *arg)
{
  YWidgetState *cur = ywidGetState(opac);

  return ywMenuCallActionOnByState(cur, event, idx, arg);
}

InputStatue ywMenuCallActionOnByState(YWidgetState *opac, Entity *event,
				      int idx, void *arg)
{
  InputStatue ret;
  Entity *entries = yeGet(opac->entity, "entries");
  Entity *action = yeGet(yeGet(entries, idx), "action");
  Entity *arg1 = Y_END_VA_LIST;

  if (idx < 0)
    return NOTHANDLE;
  ((YMenuState *)opac)->current = idx;

  if (unlikely(yeType(action) == YARRAY))
    arg1 = yeGet(action, 1);
  ret = (size_t)yesCall(yeGet(opac->signals,
			      idx + ((YMenuState *)opac)->actionSin0),
			opac->entity, event, arg, arg1);
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
    if (ywidEveKey(event) == '\n') {
      ret = ywMenuCallActionOn(opac, event, ((YMenuState *)opac)->current, NULL);
    } else {
      ret = ywidCallSignal(opac, event, NULL,  ((YMenuState *)opac)->moveSinIdx);
    }
  } else if (ywidEveType(event) == YKEY_MOUSEDOWN) {
    ret = ywMenuCallActionOn(opac, event,
			     ywMenuPosFromPix(opac->entity,
					      ywPosX(ywidEveMousePos(event)),
					      ywPosY(ywidEveMousePos(event))),
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
