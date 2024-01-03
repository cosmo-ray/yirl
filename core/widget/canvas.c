/*
**Copyright (C) 2017 Matthias Gatto
*
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

#include <math.h>
#include "rect.h"
#include "text-screen.h"
#include "sdl-driver.h"
#include "sdl2/canvas-sdl.h"
#include "yirl/canvas.h"
#include "yirl/entity-script.h"
#include "yirl/texture.h"

static int t = -1;

static int ywCanvasSetWeightInternal(Entity *wid, Entity *obj, int weight,
				     int newObj);

static void ywCanvasReleadMergeTexture(YWidgetState *ostate)
{
	YCanvasState *state = (YCanvasState *)ostate;

	if (!(state->flag & YC_MERGE))
		return;
	Entity *sz_dst = yeGet(ostate->entity, "merge_surface_size");
	yeAutoFree Entity *sz = NULL;

	if (!sz_dst) {
		sz = ywSizeCreate(ywWidth(ostate->entity),
				  ywHeight(ostate->entity),
				  NULL, NULL);
		sz_dst = sz;
	}

	yeDestroy(state->merge_texture);
	state->merge_texture = ywTextureNew(sz_dst, NULL, NULL);
}

static int init(YWidgetState *opac, Entity *entity, void *args)
{
	YCanvasState *state = (YCanvasState *)opac;
	int mergable = yeGetIntAt(entity, "mergable");
	int have_weight = yeGetIntAt(entity, "have_weight");
	(void)entity;
	(void)args;

	state->flag = 0;
	ywidGenericCall(opac, t, init);
	state->flag |= !!mergable * YC_MERGE;
	state->flag |= !!have_weight * YC_HAS_WEIGHT;
	if (mergable == 2) {
		state->flag |= YC_MERGE_NO_MERGE;
	}
	ywCanvasReleadMergeTexture((YWidgetState *)state);
	return 0;
}

static int destroy(YWidgetState *opac)
{
	YCanvasState *state = (YCanvasState *)opac;

	if (state->flag & YC_MERGE) {
		yeDestroy(state->merge_texture);
	}
	free(opac);
	return 0;
}

static int rend(YWidgetState *opac)
{
	ywidGenericRend(opac, t, render);
	return 0;
}

static void *alloc(void)
{
	YCanvasState *s = y_new0(YCanvasState, 1);
	YWidgetState *ret = (YWidgetState *)s;
	ret->render = rend;
	ret->init = init;
	ret->resize = ywCanvasReleadMergeTexture;
	ret->destroy = destroy;
	ret->handleEvent = ywidEventCallActionSin;
	ret->type = t;
	return ret;
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
	ywPosAdd(yeGet(obj, YCANVAS_POS_IDX), pos);
	return 0;
}

int ywCanvasMoveObjXY(Entity *obj, int x, int y)
{
	ywPosAddXY(yeGet(obj, YCANVAS_POS_IDX), x, y);
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

_Bool ywCanvasCheckCollisions(Entity *wid, Entity *obj, Entity *colisionFunc,
			      Entity *colisionFuncArg)
{
	Entity *eret = ywCanvasNewCollisionsArrayExt(wid, obj,
						     colisionFunc,
						     colisionFuncArg);
	int ret = !!eret;

	yeDestroy(eret);
	return ret;
}

Entity *ywCanvasNewCollisionsArray(Entity *wid, Entity *obj)
{
	return ywCanvasNewCollisionsArrayExt(wid, obj, NULL, NULL);
}

void ywCanvasDisableWeight(Entity *w)
{
	if (!w)
		return;
	YCanvasState *s = ywidCastState(w, YCanvasState);

	s->flag &= ~YC_HAS_WEIGHT;
}

void ywCanvasEnableWeight(Entity *w)
{
	if (!w)
		return;
	YCanvasState *s = ywidCastState(w, YCanvasState);

	s->flag |= YC_HAS_WEIGHT;
}

/**
 * this function will always set obj into wid if newObj is true
 * depending if (s->flag & YC_HAS_WEIGHT) is true or not, it will reoganise every
 * object of the wid depending of they weight
 */
static int ywCanvasSetWeightInternal(Entity *wid, Entity *obj, int weight,
				     int newObj)
{
	YCanvasState *s = ywidCastState(wid, YCanvasState);
	Entity *objs = getOrCreateObjs(wid);
	uint32_t i = 0;
	Entity *toPush = obj;

	if (s && !(s->flag & YC_HAS_WEIGHT)) {
		yePush(objs, toPush, NULL);
		goto out;
	}
	if (!newObj) {
		if (yeGetIntAt(obj, "weight") == weight)
			return 0;
		yeIncrRef(toPush);
		if (!yeRemoveChildByEntity(objs, obj)) {
			yeDestroy(toPush);
			return -1;
		}
	}
	yeReCreateInt(weight, obj, "weight");
	for (; i < yeLen(objs); ++i) {
		Entity *c_obj = yeGet(objs, i);
		int c_w;

		if (!c_obj) {
			uint32_t j = i + 1;
			for (; j < yeLen(objs); ++j) {
				c_obj = yeGet(objs, j);

				if (c_obj) {
					c_w = yeGetIntAt(c_obj, "weight");
					break;
				}
			}

			if (c_obj && weight < c_w) {
				yePushAt(objs, toPush, i);
				goto out;
			}
			i = j;
 			continue;
		}
		c_w = yeGetIntAt(c_obj, "weight");

		if (weight < c_w) {
			yeIncrRef(c_obj);
			yePushAt(objs, toPush, i);
			yeDestroy(toPush);
			toPush = c_obj;
			++i;
			break;
		}
	}

	for (; i < yeLen(objs); ++i) {
		Entity *c_obj = yeGet(objs, i);

		yeIncrRef(c_obj);
		yePushAt(objs, toPush, i);
		if (!c_obj) {
			goto out;
		}
		yeDestroy(toPush);
		toPush = c_obj;
	}
	yePushBack(objs, toPush, NULL);
out:
	yeDestroy(toPush);
	/* for (unsigned int i = 0; i < yeLen(objs); ++i) { */
	/* 	printf("%d ", yeGet(objs, i) ? */
	/* 	       yeGetIntAt(yeGet(objs, i), "weight") : */
	/* 		-1); */
	/* } */
	/* printf("\n"); */
	return 0;
}

int ywCanvasSetWeight(Entity *wid, Entity *obj, int weight)
{
	YCanvasState *s = ywidCastState(wid, YCanvasState);

	if (unlikely(!(s->flag & YC_HAS_WEIGHT)))
		return -1;
	return ywCanvasSetWeightInternal(wid, obj, weight, 0);
}

int ywCanvasSwapObj(Entity *wid, Entity *obj0, Entity *obj1)
{
	int tmp = yeGetIntAt(obj1, "weight");
	yeSetInt(yeGet(obj1, "weight"), yeGetIntAt(obj0, "weight"));
	yeSetInt(yeGet(obj0, "weight"), tmp);
	return yeSwapElems(yeGet(wid, "objs"), obj0, obj1);
}

Entity *ywCanvasNewIntersectArray(Entity *wid, Entity *p0, Entity *p1)
{
 	int x0 = ywPosX(p0);
 	int x1 = ywPosX(p1);
 	int y0 = ywPosY(p0);
 	int y1 = ywPosY(p1);
	Entity *ret = yeCreateArray(NULL, NULL);
	Entity *objs = yeGet(wid, "objs");

	YE_FOREACH(objs, o) {
		Entity *p = ywCanvasObjPos(o);
		Entity *s = ywCanvasObjSize(wid, o);
		int x = ywPosX(p);
		int y = ywPosY(p);
		int w = ywSizeW(s);
		int h = ywSizeH(s);

		if (yuiLinesRectIntersect(x0, y0, x1, y1,
					  x, y, w, h,
					  NULL, NULL, NULL)) {
			yePushBack(ret, o, NULL);
		}
	}
	return ret;
}

Entity *ywCanvasNewCollisionsArrayWithRectangles(Entity *wid, Entity *objRects,
						 Entity *colisionFunc,
						 Entity *colisionFuncArg)
{
	Entity *ret = yeCreateArray(NULL, NULL);
	Entity *objs = yeGet(wid, "objs");

	YE_FOREACH(objRects, unused) {
		yeCreateArray(ret, NULL);
	}

	YE_ARRAY_FOREACH(objs, tmp) {
		int i = 0;

		YE_FOREACH(objRects, objRect) {
			if (ywRectCollisionWithPos(objRect, ywCanvasObjPos(tmp),
						   ywCanvasObjSize(wid, tmp)) &&
			    (!colisionFunc || yesCall(colisionFunc, wid,
						      tmp, NULL, colisionFuncArg))) {
				yePushBack(yeGet(ret, i), tmp, NULL);
			}
			++i;
		}
	}
	return ret;
}

Entity *ywCanvasNewCollisionsArrayWithRectangle_(Entity *wid, Entity *objRect,
						 Entity *obj,
						 Entity *colisionFunc,
						 Entity *colisionFuncArg)
{
	Entity *ret = yeCreateArray(NULL, NULL);
	Entity *objs = yeGet(wid, "objs");

	YE_ARRAY_FOREACH(objs, tmp) {
		if (unlikely(obj == tmp))
			continue;

		if (ywRectCollisionWithPos(objRect, ywCanvasObjPos(tmp),
					   ywCanvasObjSize(wid, tmp)) &&
		    (!colisionFunc || yesCall(colisionFunc, wid,
					      tmp, obj, colisionFuncArg))) {
			yePushBack(ret, tmp, NULL);
		}
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
	if (unlikely(!obj)) {
		return NULL;
	}

	Entity *objRect = ywRectCreatePosSize(ywCanvasObjPos(obj),
					      ywCanvasObjSize(wid, obj),
					      NULL, NULL);
	Entity *ret =
		ywCanvasNewCollisionsArrayWithRectangle_(wid, objRect, obj,
							 colisionFunc,
							 colisionFuncArg);
	yeDestroy(objRect);
	return ret;
}

Entity *ywCanvasNewCollisionsArrayWithRectangle(Entity *wid,
						Entity *objRect)
{
	return ywCanvasNewCollisionsArrayWithRectangle_(wid, objRect,
							NULL,
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
	return yeGetIntDirect(yeGetByIdxDirect(obj, 0));
}

Entity *ywCanvasObjPos(Entity *obj)
{
	return yeGet(obj, YCANVAS_POS_IDX);
}


void ywCanvasObjClearCache(Entity *obj)
{
	yeRemoveChildByStr(obj, "$size");
	yeRemoveChildByStr(obj, "$img");
	yeRemoveChildByStr(obj, "$img-surface");
}

void ywCanvasObjSetResourceId(Entity *obj, int id)
{
	yeSetInt(yeGet(obj, 2), id);
	ywCanvasObjClearCache(obj);
}

void ywCanvasObjReplacePos(Entity *obj, Entity *p)
{
	yePushAt2(obj, p, YCANVAS_POS_IDX, "pos");
}

void ywCanvasObjSetPos(Entity *obj, int x, int y)
{
	ywPosSetInts(yeGet(obj, YCANVAS_POS_IDX), x, y);
}

void ywCanvasObjSetPosByEntity(Entity *obj, Entity *p)
{
	return ywCanvasObjSetPos(obj, ywPosX(p), ywPosY(p));
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

	size = yeGet(obj, YCANVAS_SIZE_IDX);
	if (!size) {
		sdlCanvasCacheTexture(wid, obj);
		size = yeGet(obj, YCANVAS_SIZE_IDX);
	}
	return size;
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
	Entity *objs = yeGet(wid, "objs");

	yeRemoveChild(objs, obj);
}

Entity *ywCanvasNewText(Entity *wid, int x, int y, Entity *string)
{
	if (!string)
		return NULL;
	Entity *obj = yeCreateArray(NULL, NULL);

	yeCreateInt(YCanvasString, obj, "canvas-type");
	ywPosCreateInts(x, y, obj, "pos");
	yePushBack(obj, string, "str");
	ywCanvasSetWeightInternal(wid, obj, 0, 1);
	sdlCanvasCacheTexture(wid, obj);
	return obj;
}

Entity *ywCanvasNewTextByStr(Entity *wid, int x, int y, const char *str)
{
	if (!str)
		return NULL;
	Entity *obj = yeCreateArray(NULL, NULL);

	yeCreateInt(YCanvasString, obj, "canvas-type");
	ywPosCreateInts(x, y, obj, "pos");
	yeCreateString(str, obj, "str");
	ywCanvasSetWeightInternal(wid, obj, 0, 1);
	sdlCanvasCacheTexture(wid, obj);
	return obj;
}

Entity *ywCanvasNewTextExt(Entity *wid, int x, int y, Entity *string,
			   const char *color)
{
	if (!string)
		return NULL;
	Entity *obj = yeCreateArray(NULL, NULL);

	yeCreateInt(YCanvasString, obj, "canvas-type");
	ywPosCreateInts(x, y, obj, "pos");
	yePushBack(obj, string, "str");
	yeCreateString(color, obj, "color");
	ywCanvasSetWeightInternal(wid, obj, 0, 1);
	sdlCanvasCacheTexture(wid, obj);
	return obj;
}

int ywCanvasCacheBicolorImg(Entity *obj, const uint8_t *map, Entity *info)
{
	sdlCanvasCacheBicolorImg(obj, map, info);
	return 0;
}

Entity *ywCanvasNewBicolorImg(Entity *c, int x, int y, uint8_t *img,
			      Entity *info)
{
	Entity *obj = yeCreateArray(NULL, NULL);

	assert(c);
	assert(img);
	assert(info);
	yeCreateInt(YCanvasBicolorImg, obj, "canvas-type");
	ywPosCreateInts(x, y, obj, "pos");
	ywCanvasSetWeightInternal(c, obj, 0, 1);
	sdlCanvasCacheBicolorImg(obj, img, info);
	return obj;
}

int ywCanvasCacheHeadacheImg(Entity *obj, Entity *map, Entity *info)
{
	sdlCanvasCacheHeadacheImg(obj, map, info);
	return 0;
}

Entity *ywCanvasNewHeadacheImg(Entity *c, int x, int y, Entity *map, Entity *info)
{
	Entity *obj = yeCreateArray(NULL, NULL);

	assert(c);
	assert(map);
	assert(info);
	yeCreateInt(YCanvasHeadacheImg, obj, "canvas-type");
	ywPosCreateInts(x, y, obj, "pos");
	ywCanvasSetWeightInternal(c, obj, 0, 1);
	sdlCanvasCacheHeadacheImg(obj, map, info);
	return obj;
}

void ywCanvasClear(Entity *canvas)
{
	yeClearArray(yeGet(canvas, "objs"));
}

Entity *ywCanvasCreateYTexture(Entity *obj, Entity *father, const char *name)
{
	Entity * ret = yeCreateArray(father, name);
	Entity *data = yeGet(obj, "$img-surface");
	void *true_surface = sdlCopySurface((SDL_Surface*)yeGetData(data),
					    NULL);

	if (!true_surface)
		return NULL;
	data = yeCreateData(true_surface, ret, "$img-surface");
	yeSetDestroy(data, sdlFreeSurface);
	return ret;
}

Entity *ywCanvasNewImgFromTexture2(Entity *wid, int x, int y, Entity *yTexture,
				   Entity *img_src_rect, Entity *img_dst_rect)
{
	Entity *obj = yeCreateArray(NULL, NULL);

	yeCreateInt(YCanvasTexture, obj, "canvas-type");
	ywPosCreateInts(x, y, obj, "pos");
	yePushBack(obj, yTexture, "text");
	yeCreateCopy(img_src_rect, obj, "img-src-rect");
	ywCanvasSetWeightInternal(wid, obj, 0, 1);
	if (sdlCanvasCacheImg3(obj, yTexture, NULL, NULL,
			       0, img_dst_rect) < 0) {
		ywCanvasRemoveObj(wid, obj);
		return NULL;
	}
	return obj;
}

Entity *ywCanvasNewImgFromTexture(Entity *wid, int x, int y, Entity *yTexture,
				  Entity *img_src_rect)
{
	return ywCanvasNewImgFromTexture2(wid, x, y, yTexture,
					  img_src_rect, NULL);
}

Entity *ywCanvasNewImg(Entity *wid, int x, int y, const char *path,
		       Entity *img_src_rect)
{
	Entity *obj = yeCreateArray(NULL, NULL);

	yeCreateInt(YCanvasImg, obj, "canvas-type");
	ywPosCreateInts(x, y, obj, "pos");
	yeCreateString(path, obj, "img");
	yePushBack(obj, img_src_rect, "img-src-rect");
	ywCanvasSetWeightInternal(wid, obj, 0, 1);
	if (sdlCanvasCacheTexture(wid, obj) < 0) {
		ywCanvasRemoveObj(wid, obj);
		return NULL;
	}
	return obj;
}

Entity *ywCanvasNewImgByPath(Entity *wid, int x, int y, const char *path)
{
	return ywCanvasNewImg(wid, x, y, path, NULL);
}

int ywCanvasMergeObj(Entity *wid, Entity *obj)
{
	YCanvasState *s = (YCanvasState *)ywidGetState(wid);
	Entity *dst = s->merge_texture;

	yeAutoFree Entity *dst_rect = ywRectCreatePosSize(
		ywCanvasObjPos(obj),
		ywCanvasObjSize(wid, obj),
		NULL, NULL);

	ywTextureFastMerge(obj, NULL, dst, dst_rect);
	ywCanvasRemoveObj(wid, obj);
	return 0;
}


int ywCanvasMergeText(Entity *wid, int x, int y, int w, int h,
		      const char * txt)
{
	YCanvasState *state = (YCanvasState *)ywidGetState(wid);

	return sdlMergeText(state->merge_texture, x, y, w, h, txt);
}

int ywCanvasMergeRectangle(Entity *wid, int x, int y,
			   int w, int h, const char * col)
{
	YCanvasState *state = (YCanvasState *)ywidGetState(wid);

	return sdlMergeRect(state->merge_texture, x, y, w, h, col);
}

int ywCanvasMergeTexture(Entity *wid, Entity *yTexture,
			 Entity *srcRect, Entity *dstRect)
{
	YCanvasState *state = (YCanvasState *)ywidGetState(wid);

	return sdlMergeSurface(yTexture, srcRect,
			       state->merge_texture, dstRect);
}

Entity *ywCanvasNewRect(Entity *wid, int x, int y, Entity *rect)
{
	Entity *obj = yeCreateArray(NULL, NULL);

	yeCreateInt(YCanvasRect, obj, "canvas-type");
	ywPosCreateInts(x, y, obj, "pos");
	yePushBack(obj, rect, "rect");
	ywCanvasSetWeightInternal(wid, obj, 0, 1);
	yeCreateIntAt(1, obj, "filled", YCANVAS_RECT_IS_FILLED_IDX);
	return obj;
}

Entity *ywCanvasNewTriangleExt(Entity *wid,
			       int x1, int y1, int x2, int y2,
			       int x3, int y3, const char *color,
			       int filled)
{
	Entity *obj = yeCreateArray(NULL, NULL);

	yeCreateInt(YCanvasTriangle, obj, "canvas-type");
	yeCreateQuadInt(x1, y1, 0, filled, obj, "pos");
	yeCreateString(color, obj, "color");
	yeCreateQuadIntAt(x2, y2, x3, y3, obj, "size",
			  YCANVAS_SIZE_IDX);
	ywCanvasSetWeightInternal(wid, obj, 0, 1);
	return obj;	
}

Entity *ywCanvasNewCircleExt(Entity *wid, int x, int y, int radius, const char *color, int fill)
{
	Entity *obj = yeCreateArray(NULL, NULL);

	yeCreateInt(YCanvasCircle, obj, "canvas-type");
	yeCreateQuadInt(x, y, radius, fill, obj, "pos");
	yeCreateString(color, obj, "color");
	yeCreateQuadIntAt(radius, radius, 0, 0, obj, "size", YCANVAS_SIZE_IDX);
	ywCanvasSetWeightInternal(wid, obj, 0, 1);
	return obj;
}

Entity *ywCanvasNewCircle(Entity *wid, int x, int y, int radius, const char *color)
{
  return ywCanvasNewCircleExt(wid, x, y, radius, color, 0);
}


static void pixMod(Entity *obj, Entity *rect, Entity *mod,
		   int *x, int *y, int w, int h)
{
	Entity *forcedSize = yeGet(mod, YCanvasForceSize);
	Entity *rotate = yeGet(mod, YCanvasRotate);

	if (unlikely(rotate)) {
		Entity *size = ywCanvasObjSize(NULL, obj);
		double r = -(yeGetFloat(rotate) / 180 * M_PI);
		int sw = ywSizeW(size), sh = ywSizeH(size);
		int ox = *x, oy = *y;

		// center in 0,0
		ox = ox - w / 2;
		oy = oy - h / 2;

		// turn x (and y, but turn y isn't a gundam)
		*x = ox * cos(r) - oy * sin(r);
		*y = oy * cos(r) + ox * sin(r);

		// hight left top is 0,0 again (all hail 0)
		*x += w / 2;
		*y += h / 2;

		// reset x and y to they original "relative" position
		if (sw > sh)
			*y = *y - sw / 2 + sh / 2;
		else
			*x = *x - sh / 2 + sw / 2;
		w = sw;
		h = sh;
	}

	if (unlikely(forcedSize)) {
		Entity *size = yeGet(obj, YCANVAS_SIZE_IDX);
		int realH = ywSizeH(size);
		int realW = ywSizeW(size);

		*x = *x * realW / w;
		*y = *y * realH / h;
	}
}

static Entity *rectRotationMod(Entity *obj, Entity *r, Entity *mod)
{
	Entity *rotate = yeGet(mod, YCanvasRotate);

	if (unlikely(rotate)) {
		int rw = ywRectW(r);
		int rh = ywRectH(r);

		if (rw > rh) {
			ywRectSetY(r, ywRectY(r) + rh / 2 - rw / 2 );
			ywRectSetH(r, rw);
		} else {
			ywRectSetX(r, ywRectX(r) + rw / 2 - rh / 2);
			ywRectSetW(r, rh);
		}
	}
	return r;
}

int ywidColisionXPresision = 1;
int ywidColisionYPresision = 1;

_Bool	ywCanvasCheckColisionsRectObj(Entity *r0, Entity *obj1)
{
	Entity *mod1 = ywCanvasObjMod(obj1);
	yeAutoFree Entity *r1 =
		rectRotationMod(obj1, ywRectCreatePosSize(ywCanvasObjPos(obj1),
							  ywCanvasObjSize(NULL,
									  obj1),
							  NULL, NULL),
				mod1);
	yeAutoFree Entity *colisionRects = ywRectColisionRect(r0, r1, NULL,
							      NULL);
	Entity *crect1 = yeGet(colisionRects, 1);

	if (!colisionRects) {
		return 0;
	}

	for (int i = 0; i < ywRectH(crect1); i += ywidColisionYPresision) {
		for (int j = 0; j < ywRectW(crect1);
		     j += ywidColisionXPresision) {
			YCanvasPixiel pix;
			int x, y;

			x = ywRectX(crect1) + j;
			y = ywRectY(crect1) + i;
			pixMod(obj1, r1, mod1, &x, &y,
			       ywRectW(r1), ywRectH(r1));
			pix.i = sdlCanvasPixInfo(obj1, x, y);
			if (pix.rgba[3] != 0) {
				return 1;
			}
		}
	}
	return 0;
}


Entity *ywCanvasProjectedArColisionArray(Entity *wid, Entity *rect,
					 Entity *add_pos)
{
	Entity *ret;
	yeAutoFree Entity *cr = ywRectCreateEnt(rect, NULL, NULL);

	ywPosAdd(cr, add_pos);
	ret = ywCanvasNewCollisionsArrayWithRectangle(wid, cr);
	return ret;
}

Entity *ywCanvasProjectedColisionArray(Entity *wid, Entity *obj,
				       Entity *add_pos)
{
	Entity *ret;
	Entity *s = ywCanvasObjSize(wid, obj);
	Entity *p = ywCanvasObjPos(obj);
	yeAutoFree Entity *cr = ywRectCreatePosSize(p, s, NULL, NULL);

	ywPosAdd(cr, add_pos);
	ret = ywCanvasNewCollisionsArrayWithRectangle(wid, cr);
	return ret;
}

_Bool	ywCanvasObjectsCheckColisions(Entity *obj0, Entity *obj1)
{
	Entity *mod0 = ywCanvasObjMod(obj0);
	Entity *mod1 = ywCanvasObjMod(obj1);
	Entity *r0 =
		rectRotationMod(obj0, ywRectCreatePosSize(ywCanvasObjPos(obj0),
							  ywCanvasObjSize(NULL,
									  obj0),
							  NULL, NULL),
				mod0);
	Entity *r1 =
		rectRotationMod(obj1, ywRectCreatePosSize(ywCanvasObjPos(obj1),
							  ywCanvasObjSize(NULL,
									  obj1),
							  NULL, NULL),
				mod1);
	Entity *colisionRects = ywRectColisionRect(r0, r1, NULL, NULL);
	Entity *crect0 = yeGet(colisionRects, 0);
	Entity *crect1 = yeGet(colisionRects, 1);
	int ret = 0;

	if (!colisionRects) {
		goto exit;
	}

	if (yeGetIntAt(obj0, 0) == YCanvasString ||
	    yeGetIntAt(obj1, 0) == YCanvasString) {
		ret = 1;
		goto exit;
	}

	for (int i = 0; i < ywRectH(crect0); i += ywidColisionYPresision) {
		for (int j = 0; j < ywRectW(crect0);
		     j += ywidColisionXPresision) {
			YCanvasPixiel pix;
			int x = ywRectX(crect0) + j, y = ywRectY(crect0) + i;

			pixMod(obj0, r0, mod0, &x, &y, ywRectW(r0), ywRectH(r0));
			pix.i = sdlCanvasPixInfo(obj0, x, y);
			if (pix.rgba[3] != 0) {
				x = ywRectX(crect1) + j;
				y = ywRectY(crect1) + i;
				pixMod(obj1, r1, mod1, &x, &y,
				       ywRectW(r1), ywRectH(r1));
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
	Entity *obj = yeCreateArray(NULL, NULL);

	yeCreateInt(YCanvasResource, obj, "canvas-type");
	ywPosCreateInts(x, y, obj, "pos");
	yeCreateInt(id, obj, "id");
	ywCanvasSetWeightInternal(wid, obj, 0, 1);
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

double ywCanvasObjAngle(Entity *obj)
{
	return yeGetFloat(yeGet(ywCanvasObjMod(obj), YCanvasRotate));
}

int ywCanvasAdvenceObj(Entity *obj, int speed, double direction)
{
	direction = direction / 180 * M_PI;
	int x = speed * sin(direction);
	int y = speed * cos(direction);

	ywCanvasMoveObjXY(obj, x, -y);
	return 0;
}

int ywCanvasRotate(Entity *obj, double angle)
{
	Entity *mod = getOrCreateMod(obj);

	yeCreateFloatAt(angle, mod, NULL, YCanvasRotate);
	return 0;
}

int ywCanvasForceSize(Entity *obj, Entity *size)
{
	if (!obj || !size)
		return -1;
	Entity *mod = getOrCreateMod(obj);

	yePushAt(mod, size, YCanvasForceSize);
	return 0;
}

int ywCanvasObjUnsetVFlip(Entity *obj)
{
	Entity *mod = getOrCreateMod(obj);

	yeCreateIntAt(0, mod, NULL, YCanvasVFlip);
	return 0;
}

int ywCanvasObjUnsetHFlip(Entity *obj)
{
	Entity *mod = getOrCreateMod(obj);

	yeCreateIntAt(0, mod, NULL, YCanvasHFlip);
	return 0;
}

int ywCanvasVFlip(Entity *obj)
{
	Entity *mod = getOrCreateMod(obj);

	yeCreateIntAt(1, mod, NULL, YCanvasVFlip);
	return 0;
}

int ywCanvasHFlip(Entity *obj)
{
	Entity *mod = getOrCreateMod(obj);

	yeCreateIntAt(1, mod, NULL, YCanvasHFlip);
	return 0;
}

int ywCanvasObjIsOut(Entity *wid, Entity *obj)
{
	Entity *wid_r = yeGet(wid, "wid-pix");
	Entity *mod = ywCanvasObjMod(obj);
	Entity *obj_r =
		rectRotationMod(obj, ywRectCreatePosSize(ywCanvasObjPos(obj),
							 ywCanvasObjSize(NULL,
									 obj),
							 NULL, NULL),
				mod);
	int ret = !ywRectCollision(wid_r, obj_r);

	yeDestroy(obj_r);
	return ret;
}

