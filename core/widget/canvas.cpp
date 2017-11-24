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
#include "yirl/canvas.h"
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

  int ywCanvasMoveObj(Entity *obj, Entity *pos)
  {
    ywPosAdd(yeGet(obj, 1), pos);
    return 0;
  }

  int ywCanvasMoveObjByIdx(Entity *wid, int objIdx, Entity *pos)
  {
    Entity *obj = yeGet(yeGet(wid, "objs"), objIdx);

    return ywCanvasMoveObj(obj, pos);
  }

  Entity *ywCanvasObjFromIdx(Entity *wid, int idx)
  {
    return yeGet(yeGet(wid, "objs"), idx);
  }

  Entity *ywCanvasNewColisionsArray(Entity *wid, Entity *obj)
  {
    Entity *ret = yeCreateArray(NULL, NULL);
    Entity *objs = yeGet(wid, "objs");
    Entity *objRect = ywRectCreatePosSize(ywCanvasObjPos(obj),
					  ywCanvasObjSize(wid, obj), NULL, NULL);

    YE_ARRAY_FOREACH(objs, tmpObj) {
      Entity *tmpRect = ywRectCreatePosSize(ywCanvasObjPos(tmpObj),
					    ywCanvasObjSize(wid, tmpObj), NULL, NULL);
      if (obj == tmpObj)
	continue;
      if (ywRectColision(objRect, tmpRect)) {
	yePushBack(ret, tmpObj, NULL);
      }
      yeDestroy(tmpRect);
    }
    if (!yeLen(ret)) {
      yeDestroy(ret);
      ret = NULL;
    }
    yeDestroy(objRect);
    return ret;
  }

  Entity *ywCanvasNewColisionsArrayWithRectangle(Entity *wid, Entity *objRect)
  {
    Entity *ret = yeCreateArray(NULL, NULL);
    Entity *objs = yeGet(wid, "objs");

    YE_ARRAY_FOREACH(objs, tmpObj) {
      Entity *tmpRect = ywRectCreatePosSize(ywCanvasObjPos(tmpObj),
					    ywCanvasObjSize(wid, tmpObj), NULL, NULL);
      if (ywRectColision(objRect, tmpRect)) {
	yePushBack(ret, tmpObj, NULL);
      }
      yeDestroy(tmpRect);
    }
    if (!yeLen(ret)) {
      yeDestroy(ret);
      ret = NULL;
    }
    return ret;
  }

  int ywCanvasIdxFromObj(Entity *wid, Entity *obj)
  {
    Entity *objs = yeGet(wid, "objs");
    YE_ARRAY_FOREACH_EXT(objs, tmpObj, it) {
      if (tmpObj == obj) {
	return yBlockArrayIteratorIdx(it);
      }
    }
    return -1;
  }

  YCanvasObjType ywCanvasObjType(Entity *obj)
  {
    return static_cast<YCanvasObjType>(yeGetInt(yeGet(obj, 0)));
  }

  Entity *ywCanvasObjPos(Entity *obj)
  {
    return yeGet(obj, 1);
  }

  void ywCanvasObjSetResourceId(Entity *obj, int id)
  {
    yeSetInt(yeGet(obj, 2), id);
    yeRemoveChildByStr(obj, "$size");
    yeRemoveChildByStr(obj, "$img");
  }

  void ywCanvasObjSetPos(Entity *obj, int x, int y)
  {
    ywPosSetInts(yeGet(obj, 1), x, y);
  }

  Entity *ywCanvasObjSize(Entity *wid, Entity *obj)
  {
    Entity *size;

    if (ywCanvasObjType(obj) == YCanvasRect) {
      return yeGet(yeGet(obj, 2), 0);
    }

    size = yeGet(obj, "$size");
    if (!size) {
      sdlCanvasCacheTexture(wid, obj);
      size = yeGet(obj, "$size");
    }
    return size;
  }

  static Entity *getOrCreateObjs(Entity *wid)
  {
    Entity *objs = yeGet(wid, "objs");

    if (unlikely(!objs)) {
      objs = yeCreateArray(wid, "objs");
      if (unlikely(!objs))
	return NULL;
    }
    return objs;
  }

  void ywCanvasStringSet(Entity *obj, Entity *newStr)
  {
    if (ywCanvasObjType(obj) != YCanvasString)
      return;
    yeReplaceAtIdx(obj, newStr, 2);
    yeRemoveChildByStr(obj, "$size");
  }

  void ywCanvasRemoveObj(Entity *wid, Entity *obj)
  {
    Entity *objs = getOrCreateObjs(wid);

    yeRemoveChild(objs, obj);
  }

  Entity *ywCanvasNewText(Entity *wid, int x, int y, Entity *string)
  {
    Entity *objs = getOrCreateObjs(wid);
    Entity *obj = yeCreateArray(objs, NULL);

    yeCreateInt(YCanvasString, obj, "canvas-type");
    ywPosCreateInts(x, y, obj, "pos");
    yePushBack(obj, string, "str");
    return obj;
  }

  Entity *ywCanvasNewRect(Entity *wid, int x, int y, Entity *rect)
  {
    Entity *objs = getOrCreateObjs(wid);
    Entity *obj = yeCreateArray(objs, NULL);

    yeCreateInt(YCanvasRect, obj, "canvas-type");
    ywPosCreateInts(x, y, obj, "pos");
    yePushBack(obj, rect, "rect");
    return obj;
  }

 Entity *ywCanvasNewObj(Entity *wid, int x, int y, int id)
  {
    Entity *objs = getOrCreateObjs(wid);
    Entity *obj = yeCreateArray(objs, NULL);

    yeCreateInt(YCanvasResource, obj, "canvas-type");
    ywPosCreateInts(x, y, obj, "pos");
    yeCreateInt(id, obj, "id");
    return obj;
  }

}

