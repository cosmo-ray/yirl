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
  YWidgetState *wid = va_arg(ap, YWidgetState *);
  YEvent *eve = va_arg(ap, YEvent *);

  if (eve->key == Y_DOWN_KEY) {
    return nmMenuDown(wid);
  } else if (eve->key == Y_UP_KEY) {
    return nmMenuUp(wid);
  }
  return (void *)NOTHANDLE;
}

static void *nmMenuNext(va_list ap)
{
  YWidgetState *wid = va_arg(ap, YWidgetState *);
  Entity *next = yeGet(wid->entity, "entries");

  next = yeGet(next, ((YMenuState *)wid)->current);
  next = yeGet(next, "next");

  return ywidNext(next) ? (void *)BUG : (void *)ACTION ;
}

#define yeForeach(entity, idx, entry)					\
  Entity *entry = yeGet(entity, 0);					\
  for (int idx = 0; (entry = yeGet(entries, idx)); ++idx)

static int mnInit(YWidgetState *opac, Entity *entity, void *args)
{
  (void)args;
  YMenuState *state = ((YMenuState *)opac);
  Entity *entries = yeGet(entity, "entries");

  ywidGenericCall(opac, t, init);
  state->moveSinIdx = ywidAddSignal(opac, "move");
  ygBind(opac, "move", "menuMove");
  state->actionSin0 = state->moveSinIdx + 1;
  yeForeach(entries, i, entry) {
    char *tmp = g_strdup_printf("action-%d", i);
    int ret = ywidAddSignal(opac, tmp);
    Entity *action;

    g_free(tmp);
    if (ret != state->actionSin0 + i)
      return -1;
    action = yeGet(entry, "action");
    if (action)
      ygBindBySinIdx(opac, ret, yeGetString(yeGet(entry, "action")));
  }
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
  opac->hasChange = 0;
  return 0;
}

static InputStatue mnEvent(YWidgetState *opac, YEvent *event)
{
  InputStatue ret = NOTHANDLE;

  (void)opac;

  if (!event)
    return NOTHANDLE;

  if (event->type != YKEY_DOWN)
    goto exit;
  if (event->key == '\n') {
    ret = ywidCallSignal(opac, event, NULL,
			 ((YMenuState *)opac)->current +
			 ((YMenuState *)opac)->actionSin0);
  } else {
    ret = ywidCallSignal(opac, event, NULL,  ((YMenuState *)opac)->moveSinIdx);
  }

 exit:
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
