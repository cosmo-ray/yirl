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
#include "menu.h"

static int t = -1;

typedef struct {
  YWidgetState sate;
  unsigned int current;
} YMenuState;

static int mnInit(YWidgetState *opac, Entity *entity, void *args)
{
  opac->entity = entity;
  ywidGenericInit(opac, t);
  (void)args;
  return 0;
}

static int mnDestroy(YWidgetState *opac)
{
  g_free(opac);
  return 0;
}

static int mnRend(YWidgetState *opac)
{
  return ywidGenericRend(opac, t);
}

static InputStatue mnEvent(YWidgetState *opac, YEvent *event)
{
  InputStatue ret = NOTHANDLE;

  (void)opac;
  if (!event)
    event = ywidGenericWaitEvent();

  if (!event)
    return NOTHANDLE;

  if (event->key == Y_ESC_KEY) {
    ret = ACTION;
  } else if (event->key == '\n') {
    ret = ACTION;
  } else if (event->key == Y_DOWN_KEY) {
    ((YMenuState *)opac)->current += 1;
    ret = NOACTION;
    if (((YMenuState *)opac)->current > yeLen(yeGet(opac->entity, "entries")) - 1)
      ((YMenuState *)opac)->current = 0;

  } else if (event->key == Y_UP_KEY) {
    ((YMenuState *)opac)->current -= 1;
    ret = NOACTION;
    if (((YMenuState *)opac)->current > yeLen(yeGet(opac->entity, "entries")))
      ((YMenuState *)opac)->current = yeLen(yeGet(opac->entity, "entries")) - 1;

  }

  g_free(event);
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
  return  ret;
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
  return t;
}

int ywMenuEnd(void)
{
  return ywidUnregiste(t);
}
