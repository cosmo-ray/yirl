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
#include "contener.h"

static int t = -1;

static void widDestroyWrapper(void *wid)
{
  YWidDestroy(wid);
}

#define yCntType(opac) CNT_HORIZONTAL
static int cntInit(YWidgetState *opac, Entity *entity, void *args)
{
  Entity *entries = yeGet(entity, "entries");
  Entity *pos = yeGet(entity, "pos");
  int i = 0;
  size_t len = yeLen(entries);
  int caseSize = 0;
  int casePos;

  (void)opac;
  (void)args;

  caseSize =  yCntType(opac) == CNT_HORIZONTAL ?
    yeGetInt(yeGet(pos, "h")) / len : yeGetInt(yeGet(pos, "w")) / len;

  YE_ARRAY_FOREACH(entries, tmp) {
    Entity *ptr = yeFindLink(entity, yeGetString(yeGet(tmp, "name")), 0);
    Entity *tmpPos;
    YWidgetState *wid = ywidNewWidget(ptr, NULL);
    Entity *widData = yeCreateData(wid, tmp, "$wid");

    tmpPos = yeGet(ptr, "pos");
    casePos = i * caseSize;

    if (yCntType(opac) == CNT_HORIZONTAL) {
      /* modify y and h pos in internal struct */
      yeSet(yeGet(tmpPos, "y"), casePos);
      yeSet(yeGet(tmpPos, "h"), caseSize);
    } else if (yCntType(opac) == CNT_VERTICAL) {

      /* modify x and w pos in internal struct */
      yeSet(yeGet(tmpPos, "x"), casePos);
      yeSet(yeGet(tmpPos, "w"), caseSize);
    }

    /* else nothing */
    ywidResize(wid);
    yeSetDestroy(widData, widDestroyWrapper);
    ++i;
  }

  return 0;
}
#undef yCntType

static int cntDestroy(YWidgetState *opac)
{
  g_free(opac);
  return 0;
}

static InputStatue cntEvent(YWidgetState *opac, YEvent *event)
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

static int cntRend(YWidgetState *opac)
{
  Entity *entries = yeGet(opac->entity, "entries");

  YE_ARRAY_FOREACH(entries, tmp) {
    ywidRend(yeGetData(yeGet(tmp, "$wid")));
  }
  return 0;
}

static void *alloc(void)
{
  YContenerState *ret = g_new0(YContenerState, 1);
  YWidgetState *wstate = (YWidgetState *)ret;

  wstate->render = cntRend;
  wstate->init = cntInit;
  wstate->destroy = cntDestroy;
  wstate->handleEvent = cntEvent;
  wstate->type = t;
  ret->curent = -1;
  return  ret;
}

int ywContenerInit(void)
{
  if (t != -1)
    return t;
  t = ywidRegister(alloc, "contener");
  return t;
}

int ywContenerEnd(void)
{
  if (ywidUnregiste(t) < 0)
    return -1;
  t = -1;
  return 0;
}
