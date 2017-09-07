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
#include "rect.h"
#include "text-screen.h"

static int t = -1;

typedef struct {
  YWidgetState sate;
} YTextScreenState;

static int tsInit(YWidgetState *opac, Entity *entity, void *args)
{
  (void)entity;
  (void)args;

  yeCreateInt(0, entity, "text-threshold");
  yeCreateInt(16, entity, "font-size");
  ywidGenericCall(opac, t, init);
  return 0;
}

static int tsDestroy(YWidgetState *opac)
{
  g_free(opac);
  return 0;
}

static int tsRend(YWidgetState *opac)
{
  ywidGenericRend(opac, t, render);
  return 0;
}

static void *alloc(void)
{
  YTextScreenState *ret = g_new0(YTextScreenState, 1);
  YWidgetState *wstate = (YWidgetState *)ret;

  wstate->render = tsRend;
  wstate->init = tsInit;
  wstate->destroy = tsDestroy;
  wstate->handleEvent = ywidEventCallActionSin;
  wstate->type = t;
  return  ret;
}

int ywTextScreenPosAtEndOfText(Entity *wid)
{
  Entity *toPrint = yeGet(wid, "text");
  int nbLines;
  /* Entity *txtThreshold = yeGet(wid, "text-threshold"); */
  int h = ywRectH(yeGet(wid, "wid-pix"));

  if (!toPrint)
    return 0;
  nbLines = yeCountCharacters(toPrint, '\n', -1);
  nbLines *= yeGetInt(yeGet(wid, "font-size"));
  if (nbLines > h) {
    yeSetAt(wid, "text-threshold", h - nbLines);
  }
  return 0;
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

