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
#include <math.h>
#include "rect.h"
#include "text-screen.h"
#include "sdl-driver.h"
#include "sdl2/canvas-sdl.h"
#include "yirl/canvas.h"
#include "yirl/entity-script.h"
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
  delete opac;
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
    if (idx < 0)
      idx = yeLen(yeGet(wid, "objs")) + idx;
    return yeGet(yeGet(wid, "objs"), idx);
  }

  int ywCanvasCheckCollisions(Entity *wid, Entity *obj, Entity *colisionFunc,
			      Entity *colisionFuncArg)
  {
    Entity *eret = ywCanvasNewCollisionsArrayExt(wid, obj, colisionFunc,
						colisionFuncArg);
    int ret = !!eret;

    yeDestroy(eret);
    return ret;
  }

  Entity *ywCanvasNewCollisionsArray(Entity *wid, Entity *obj)
  {
    return ywCanvasNewCollisionsArrayExt(wid, obj, NULL, NULL);
  }

  int ywCanvasSwapObj(Entity *wid, Entity *obj0, Entity *obj1)
  {
    return yeSwapElems(yeGet(wid, "objs"), obj0, obj1);
  }

  Entity *ywCanvasNewCollisionsArrayWithRectangle_(Entity *wid, Entity *objRect,
						   Entity *obj,
						   Entity *colisionFunc,
						   Entity *colisionFuncArg)
  {
    Entity *ret = yeCreateArray(NULL, NULL);
    Entity *objs = yeGet(wid, "objs");

    YE_ARRAY_FOREACH(objs, tmpObj) {
      Entity *tmpRect = ywRectCreatePosSize(ywCanvasObjPos(tmpObj),
					    ywCanvasObjSize(wid, tmpObj),
					    NULL, NULL);
      if (obj == tmpObj) {
	yeDestroy(tmpRect);
	continue;
      }

      if (ywRectCollision(objRect, tmpRect) &&
	  (!colisionFunc || yesCall(colisionFunc, wid,
				    tmpObj, obj, colisionFuncArg))) {
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

  Entity *ywCanvasNewCollisionsArrayExt(Entity *wid, Entity *obj,
					Entity *colisionFunc,
					Entity *colisionFuncArg)
  {
    Entity *objRect = ywRectCreatePosSize(ywCanvasObjPos(obj),
					  ywCanvasObjSize(wid, obj), NULL, NULL);

    Entity *ret = ywCanvasNewCollisionsArrayWithRectangle_(wid, objRect, obj,
							   colisionFunc,
							   colisionFuncArg);
    yeDestroy(objRect);
    return ret;
  }

  Entity *ywCanvasNewCollisionsArrayWithRectangle(Entity *wid, Entity *objRect)
  {
    return ywCanvasNewCollisionsArrayWithRectangle_(wid, objRect, NULL,
						    NULL, NULL);
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
    Entity *mod = ywCanvasObjMod(obj);

    if (unlikely(yeGet(mod, YCanvasForceSize)))
      return yeGet(mod, YCanvasForceSize);
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
    sdlCanvasCacheTexture(wid, obj);
    return obj;
  }

  Entity *ywCanvasNewImgByPath(Entity *wid, int x, int y, const char *path)
  {
    Entity *objs = getOrCreateObjs(wid);
    Entity *obj = yeCreateArray(objs, NULL);

    yeCreateInt(YCanvasImg, obj, "canvas-type");
    ywPosCreateInts(x, y, obj, "pos");
    yeCreateString(path, obj, "img");
    sdlCanvasCacheTexture(wid, obj);
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

  static void pixMod(Entity *obj, Entity *rect, Entity *mod,
		     int &x, int &y, int w, int h)
  {
    Entity *forcedSize = yeGet(mod, YCanvasForceSize);
    Entity *rotate = yeGet(mod, YCanvasRotate);

    if (unlikely(rotate)) {
      Entity *size = ywCanvasObjSize(NULL, obj);
      double r = -(yeGetFloat(rotate) / 180 * M_PI);
      int sw = ywSizeW(size), sh = ywSizeH(size);
      int ox = x, oy = y;

      // center in 0,0
      ox = x - w / 2;
      oy = y - h / 2;

      // turn x (and y, but turn y isn't a gundam)
      x = ox * cos(r) - oy * sin(r);
      y = oy * cos(r) + ox * sin(r);

      // hight left top is 0,0 again (all hail 0)
      x += w / 2;
      y += h / 2;

      // reset x and y to they original "relative" position
      if (sw > sh)
      	y = y - sw / 2 + sh / 2;
      else
	x = x - sh / 2 + sw / 2;
      w = sw;
      h = sh;
    }

    if (unlikely(forcedSize)) {
      Entity *size = yeGet(obj, "$size");
      int realH = ywSizeH(size);
      int realW = ywSizeW(size);

      x = x * realW / w;
      y = y * realH / h;
    }
  }

  static Entity *rectRotationMod(Entity *obj, Entity *r, Entity *mod)
  {
    Entity *rotate = yeGet(mod, YCanvasRotate);

    if (unlikely(rotate)) {
      if (ywRectW(r) > ywRectH(r)) {
	ywRectSetY(r, ywRectY(r) + ywRectH(r) / 2 - ywRectW(r) / 2 );
	ywRectSetH(r, ywRectW(r));
      } else {
	ywRectSetX(r, ywRectX(r) + ywRectW(r) / 2 - ywRectH(r) / 2);
	ywRectSetW(r, ywRectH(r));
      }
    }
    return r;
  }

  int ywidColisionXPresision = 1;
  int ywidColisionYPresision = 1;

  int	ywCanvasObjectsCheckColisions(Entity *obj0, Entity *obj1)
  {
    Entity *mod0 = ywCanvasObjMod(obj0);
    Entity *mod1 = ywCanvasObjMod(obj1);
    Entity *r0 =
      rectRotationMod(obj0, ywRectCreatePosSize(ywCanvasObjPos(obj0),
						ywCanvasObjSize(NULL, obj0),
						NULL, NULL),
		      mod0);
    Entity *r1 =
      rectRotationMod(obj1, ywRectCreatePosSize(ywCanvasObjPos(obj1),
						ywCanvasObjSize(NULL, obj1),
						NULL, NULL),
		      mod1);
    Entity *colisionRects = ywRectColisionRect(r0, r1, NULL, NULL);
    Entity *crect0 = yeGet(colisionRects, 0);
    Entity *crect1 = yeGet(colisionRects, 1);
    int ret = 0;

    // ywRectPrint(r0);
    // ywRectPrint(r1);
    if (!colisionRects) {
      goto exit;
    }
    // ywRectPrint(yeGet(colisionRects, 0));
    // ywRectPrint(yeGet(colisionRects, 1));

    for (int i = 0; i < ywRectH(crect0); i += ywidColisionYPresision) {
      for (int j = 0; j < ywRectW(crect0); j += ywidColisionXPresision) {
	YCanvasPixiel pix;
	int x = ywRectX(crect0) + j, y = ywRectY(crect0) + i;

	pixMod(obj0, r0, mod0, x, y, ywRectW(r0), ywRectH(r0));
	pix.i = sdlCanvasPixInfo(obj0, x, y);
	if (pix.rgba[3] != 0) {
	  x = ywRectX(crect1) + j;
	  y = ywRectY(crect1) + i;
	  pixMod(obj1, r1, mod1, x, y, ywRectW(r1), ywRectH(r1));
	  pix.i = sdlCanvasPixInfo(obj1, x, y);
 	  if (pix.rgba[3] != 0) {
	    ret = 1;
	    goto exit;
	  }
	}
      }
    }
  exit:
    yeMultDestroy(r0, r1, colisionRects);
    return ret;
  }

  Entity *ywCanvasNewObj(Entity *wid, int x, int y, int id)
  {
    Entity *objs = getOrCreateObjs(wid);
    Entity *obj = yeCreateArray(objs, NULL);

    yeCreateInt(YCanvasResource, obj, "canvas-type");
    ywPosCreateInts(x, y, obj, "pos");
    yeCreateInt(id, obj, "id");
    sdlCanvasCacheTexture(wid, obj);
    return obj;
  }

  static inline Entity *getOrCreateMod(Entity *obj)
  {
    Entity *mod = ywCanvasObjMod(obj);
    if (!mod) {
      mod = yeCreateArray(obj, "$mod");
    }
    return mod;
  }

  int ywCanvasRotate(Entity *obj, double angle)
  {
    Entity *mod = getOrCreateMod(obj);

    yeCreateFloatAt(angle, mod, NULL, YCanvasRotate);
    return 0;
  }

  int ywCanvasForceSize(Entity *obj, Entity *size)
  {
    Entity *mod = getOrCreateMod(obj);

    yePushAt(mod, size, YCanvasForceSize);
    return 0;
  }
}

