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
  ywidGenericCall(opac, t, init);

  ((YMapState *)opac)->resources = yeGet(entity, "resources");

  if (yuiStrEqual0(yeGetString(yeGet(entity, "cam-type")), "center")) {
    ((YMapState *)opac)->renderType = YMAP_PARTIAL;
    if (!yeGet(entity, "cam-pos"))
      yeCreateInt(0, entity, "cam-pos");
  } else {
    ((YMapState *)opac)->renderType = YMAP_FULL;
  }

  (void)args;
  return 0;
}

Entity *ywMapPosFromInt(Entity *wid, int pos,
			Entity *father, const char *name)
{
  int w = yeGetInt(yeGet(wid, "width"));

  return ywPosCreate(pos % w, pos / w, father, name);
}

int ywMapIntFromPos(Entity *wid, Entity *pos)
{
  int w = yeGetInt(yeGet(wid, "width"));
  int x = yeGetInt(yeGet(pos, 0));
  int y = yeGetInt(yeGet(pos, 1));

  return x + y * w;
}


Entity *ywMapPushElem(Entity *state, Entity *toPush,
		      Entity *pos, const char *name)
{
  int ret = yePushBack(ywMapGetCase(state, pos), toPush, name);
  return ret ? toPush : NULL;
}

Entity *ywMapPushNbr(Entity *state, int toPush,
		     Entity *pos, const char *name)
{
  return yeCreateInt(toPush, ywMapGetCase(state, pos), name);
}

Entity *ywMapGetCase(Entity *state, Entity *pos)
{
  Entity *map = yeGet(state, "map");
  int w = yeGetInt(yeGet(state, "width"));
  Entity *ret;

  ret = yeGet(map, yeGetInt(yeGet(pos, "x")) + (w * yeGetInt(yeGet(pos, "y"))));
  if (unlikely(!ret)) {
    if (!state)
      DPRINT_ERR("entity is NULL");
    else if (!map)
      DPRINT_ERR("unable to get 'map'");
    else if (!w)
      DPRINT_ERR("unable to get 'width'");
    else if (!pos)
      DPRINT_ERR("pos is NULL");
    else if (!yeGet(pos, "x"))
      DPRINT_ERR("unable to get 'x' in pos");
    else if (!yeGet(pos, "y"))
      DPRINT_ERR("unable to get 'w' in pos");
  }
  return ret;
}

static int mapDestroy(YWidgetState *opac)
{
  g_free(opac);
  return 0;
}

static int mapRend(YWidgetState *opac)
{
  if (unlikely(!opac->hasChange))
    return 0;

  ywidGenericCall(opac, t, render);
  return 0;
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
  wstate->handleEvent = ywidEventCallActionSin;
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
