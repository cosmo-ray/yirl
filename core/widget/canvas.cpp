/*
**Copyright (C) 2017 Matthias Gatto
5C**
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

extern "C" {
#include <glib.h>
#include "rect.h"
#include "text-screen.h"
#include "sdl-driver.h"
#include "sdl2/canvas-sdl.h"
}

static int t = -1;

class YCanvasState : public YWidgetState {
public:
  YCanvasState() {
  }
};

static int tsInit(YWidgetState *opac, Entity *entity, void *args)
{
  (void)entity;
  (void)args;

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
  YCanvasState *ret = new YCanvasState;

  ret->render = tsRend;
  ret->init = tsInit;
  ret->destroy = tsDestroy;
  ret->handleEvent = ywidEventCallActionSin;
  ret->midRend = NULL;
  ret->resize = NULL;
  ret->midRendEnd = NULL;
  ret->type = t;
  return  ret;
}

extern "C" {
  int ywCanvasInit(void)
  {
    if (ysdl2Type() < 0) {
      DPRINT_ERR("sdl is needed for canvas");
    }
    if (t != -1)
      return t;
    t = ywidRegister(alloc, "canvas");
    return t;
  }

  int ywCanvasEnd(void)
  {
    if (ywidUnregiste(t) < 0)
      return -1;
    t = -1;
    return 0;
  }

  int ywCanvasMoveObjByIdx(Entity *wid, int objIdx, Entity *pos)
  {
    Entity *obj = yeGet(yeGet(wid, "objs"), objIdx);

    ywPosAdd(yeGet(obj, "pos"), pos);
    return 0;
  }

  Entity *ywCanvasObjSize(Entity *wid, Entity *obj)
  {
    Entity *size = yeGet(obj, "$size");
    if (!size) {
      sdlCanvasCacheImg(wid, obj);
      size = yeGet(obj, "$size");
    }
    return size;
  }

  Entity *ywCanvasNewObj(Entity *wid, int x, int y, int id)
  {
    Entity *objs = yeGet(wid, "objs");
    Entity *obj;

    if (unlikely(!objs)) {
      yeCreateArray(wid, "objs");
      if (unlikely(!objs))
	return NULL;
    }
    obj = yeCreateArray(objs, NULL);
    ywPosCreateInts(x, y, obj, "pos");
    yeCreateInt(id, obj, "id");
    return obj;
  }

}

