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
#include "game.h"
#include "timer.h"
#include "text-screen.h"

static int t = -1;
static int aleradyFormated = 0;

typedef struct {
  YWidgetState sate;
  char tmpChar;
  YTimer *timerTxtSpeed;
} YTextScreenState;

static int tsInit(YWidgetState *opac, Entity *entity, void *args)
{
  YTextScreenState *o_txt = (void *)opac;
  (void)args;

  yeCreateInt(0, entity, "text-threshold");
  yeCreateInt(16, entity, "font-size");
  if (yeGet(entity, "text-speed")) {
    o_txt->timerTxtSpeed = YTimerCreate();
  }
  ywidGenericCall(opac, t, init);
  return 0;
}

static int tsDestroy(YWidgetState *opac)
{
  YTextScreenState *o_txt = (void *)opac;
  g_free(o_txt->timerTxtSpeed);
  g_free(opac);
  return 0;
}

static int tsRend(YWidgetState *opac)
{
  YTextScreenState *o_txt = (void *)opac;
  uint64_t c_pos;
  Entity *txt;
  Entity *tmpEnd = NULL;
  YTimer *timer = o_txt->timerTxtSpeed;

  aleradyFormated = 0;
  txt = ywTextScreenTextEnt(opac->entity);
  if (timer) {
    c_pos = YTimerGet(timer) / yeGetIntAt(opac->entity, "text-speed");
    tmpEnd = yeStringAddTmpEnd(txt, c_pos, NULL, NULL);
  }
  ywidGenericRend(opac, t, render);
  if (tmpEnd) {
    yeStringRestoreEnd(txt, tmpEnd, 1);
  }
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

Entity *ywTextScreenTextEnt(Entity *wid)
{
  Entity *ret;
  Entity *txt;

  if (likely(!yeGet(wid, "fmt")))
    return yeGet(wid, "text");

  if (aleradyFormated)
    return yeGet(wid, "text");
  txt = yeGet(wid, "text");
  ret = yeCreateYirlFmtString(txt, wid, "$fmt");
  aleradyFormated = 1;
  return ret;
}

const char *ywTextScreenText(Entity *wid)
{
  return yeGetString(ywTextScreenTextEnt(wid));
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

