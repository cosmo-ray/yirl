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

#include <unistd.h>
#include <glib.h>
#include "map.h"
#include "widget-callback.h"
static int t = -1;

static int mapInit(YWidgetState *opac, Entity *entity, void *args)
{
  const char *action;
  Entity *initer = yeGet(entity, "init");

  ywidGenericCall(opac, t, init);

  ((YMapState *)opac)->resources = yeGet(entity, "resources");
  ((YMapState *)opac)->pos = ywMapCreatePos(0, 0, NULL, NULL);

  ((YMapState *)opac)->actionIdx = ywidAddSignal(opac, "action");
  action = yeGetString(yeGet(entity, "action"));
  ywidBind(opac, "action", action);
  if (initer) {
    YCallback *callback = ywinGetCallbackByStr(yeGetString(initer));

    if (callback)
      ywidCallCallback(callback, opac, NULL, entity);
  }

  if (yuiStrEqual0(yeGetString(yeGet(entity, "cam-type")), "center")) {
    ((YMapState *)opac)->renderType = YMAP_PARTIAL;
  } else {
    ((YMapState *)opac)->renderType = YMAP_FULL;
  }

  (void)args;
  return 0;
}

Entity *ywMapCreatePos(int posX, int posY, Entity *father, const char *str)
{
  Entity *ret = yeCreateArray(father, str);

  yeCreateInt(posX, ret, "x");
  yeCreateInt(posY, ret, "y");
  return ret;
}

int ywMapPushElem(YWidgetState *state, Entity *toPush,
		  Entity *pos, const char *name)
{
  int ret =  yePushBack(ywMapGetCase(state, pos), toPush, name);
  return ret;
}

Entity *ywMapGetPos(YWidgetState *state)
{
  return ((YMapState *)state)->pos;
}

Entity *ywMapGetCurrentCase(YWidgetState *state)
{
  Entity *pos = ywMapGetPos(state);
  
  return ywMapGetCase(state, pos);
}

Entity *ywMapGetCase(YWidgetState *state, Entity *pos)
{
  Entity *map = yeGet(state->entity, "map");
  int w = yeGetInt(yeGet(state->entity, "width"));

  return yeGet(map, yeGetInt(yeGet(pos, "x")) + (w * yeGetInt(yeGet(pos, "y"))));
}

static int mapDestroy(YWidgetState *opac)
{
  YE_DESTROY(((YMapState *)opac)->pos);
  g_free(opac);
  return 0;
}

static int mapRend(YWidgetState *opac)
{
  ywidGenericCall(opac, t, render);
  return 0;
}

static InputStatue mapEvent(YWidgetState *opac, YEvent *event)
{
  InputStatue ret = NOTHANDLE;

  /* set pos */
  ret = ywidCallSignal(opac, event, NULL,
		       ((YMapState *)opac)->actionIdx);

  opac->hasChange = ret == NOTHANDLE ? 0 : 1;
  return ret;
}

int ywMapHasChange(YWidgetState *state)
{
  return state->hasChange;
}

static void *alloc(void)
{
  YMapState *ret = g_new0(YMapState, 1);
  YWidgetState *wstate = (YWidgetState *)ret;

  if (!ret)
    return NULL;

  wstate->render = mapRend;
  wstate->init = mapInit;
  wstate->destroy = mapDestroy;
  wstate->handleEvent = mapEvent;
  wstate->type = t;
  return  ret;
}

int ywMapGetIdByElem(Entity *mapElem)
{
  if (yeType(mapElem) == YINT)
    return yeGetInt(mapElem);
  if (yeType(mapElem) == YARRAY)
    return yeGetInt(yeGet(mapElem, "id"));

  return -1;
}

int ywMapInit(void)
{
  ywidInitCallback();
  if (t != -1)
    return t;

  t = ywidRegister(alloc, "map");
  return t;
}

int ywMapEnd(void)
{
  if (ywidUnregiste(t) < 0)
    return -1;
  t = -1;
  return 0;
}
