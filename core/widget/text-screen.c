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
#include "text-screen.h"
#include "widget-callback.h"

static int t = -1;

typedef struct {
  YWidgetState sate;
  unsigned int hasChange;
  int actionIdx;

} YTextScreenState;

static int tsInit(YWidgetState *opac, Entity *entity, void *args)
{
  (void)entity;
  (void)args;

  ywidGenericInit(opac, t);
  ((YTextScreenState *)opac)->actionIdx = ywidAddSignal(opac, "action");
  ywidBind(opac, "action", yeGetString(yeGet(entity, "action")));
  ywidCallCallbackByStr(yeGetString(yeGet(entity, "init")), opac, NULL, NULL);
  return 0;
}

static int tsDestroy(YWidgetState *opac)
{
  g_free(opac);
  return 0;
}

static InputStatue tsEvent(YWidgetState *opac, YEvent *event)
{
  InputStatue ret = NOTHANDLE;

  (void)opac;
 
  if (!event)
    return NOTHANDLE;

  if (event->key == Y_ESC_KEY)
    ret = ACTION;
  else if (event->key == '\n') {
    ret = ywidCallSignal(opac, event, NULL,
			 ((YTextScreenState *)opac)->actionIdx);
  }
  return ret;
}

static int tsRend(YWidgetState *opac)
{
  int ret = 0;

  if (opac->hasChange)
    ret = ywidGenericRend(opac, t);
  opac->hasChange = 0;
  return ret;
}

static void *alloc(void)
{
  YTextScreenState *ret = g_new0(YTextScreenState, 1);
  YWidgetState *wstate = (YWidgetState *)ret;

  wstate->render = tsRend;
  wstate->init = tsInit;
  wstate->destroy = tsDestroy;
  wstate->handleEvent = tsEvent;
  wstate->type = t;
  return  ret;
}

int ywTextScreenInit(void)
{
  if (t != -1)
    return t;
  t = ywidRegister(alloc, "text-screen");
  return t;
}

int ywTextScreenEnd(void)
{
  if (ywidUnregiste(t) < 0)
    return -1;
  t = -1;
  return 0;
}

