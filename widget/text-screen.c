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

static int t = -1;

static int tsInit(YWidgetState *opac, Entity *entity, void *args)
{
  opac->entity = entity;
  ywidGenericInit(opac, t);
  (void)args;
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
    event = ywidGenericWaitEvent();

  if (!event)
    return NOTHANDLE;

  if (event->key == Y_ESC_KEY)
    ret = ACTION;
  else if (event->key == '\n')
    ret = ACTION;

  g_free(event);
  return ret;
}

static int tsRend(YWidgetState *opac)
{
  return ywidGenericRend(opac, t);
}

static void *alloc(void)
{
  YWidgetState *ret = g_new0(YWidgetState, 1);

  ret->render = tsRend;
  ret->init = tsInit;
  ret->destroy = tsDestroy;
  ret->handleEvent = tsEvent;
  ret->type = t;
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

