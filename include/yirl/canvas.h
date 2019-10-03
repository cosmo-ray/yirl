/*
**Copyright (C) 2017 Matthias Gatto
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

#ifndef	_YIRL_CANVAS_H_
#define	_YIRL_CANVAS_H_

#include "yirl/widget.h"

typedef enum  {
  YCanvasResource,
  YCanvasRect,
  YCanvasString,
  YCanvasTexture,
  YCanvasImg,
  YCanvasBigTexture,
  YCnvasEndType
} YCanvasObjType;

typedef enum  {
  YCanvasForceSize,
  YCanvasRotate,
} YCanvasModifier;

#define YCANVAS_TYPE_IDX 0
#define YCANVAS_POS_IDX 1
#define YCANVAS_DATA_IDX 2
#define YCANVAS_IMG_IDX 6
#define YCANVAS_SIZE_IDX 7

typedef union {
  uint32_t i;
  uint8_t rgba[4];
} YCanvasPixiel;

int ywCanvasInit(void);
int ywCanvasEnd(void);
int ysdl2RegistreCanvas(void);

void ywCanvasClear(Entity *canvas);

static Entity *ywCreateCanvasEnt(Entity *father, char *name)
{
  Entity *ret = yeCreateArray(father, name);

  yeCreateString("canvas", ret, "<type>");
  return ret;
}

/**
 * add @pos to object position at @objIdx
 */
int ywCanvasMoveObjByIdx(Entity *wid, int objIdx, Entity *pos);

Entity *ywCanvasObjSize(Entity *wid, Entity *obj);

Entity *ywCanvasObjPos(Entity *obj);

static inline int ywCanvasObjPosX(Entity *obj) {
	return ywPosX(ywCanvasObjPos(obj));
}

static inline int ywCanvasObjPosY(Entity *obj) {
	return ywPosY(ywCanvasObjPos(obj));
}

double ywCanvasObjAngle(Entity *obj);

/**
 * resize obj to @size, result can be ugly
 */
int ywCanvasForceSize(Entity *obj, Entity *size);
int ywCanvasRotate(Entity *obj, double angle);

int ywCanvasAdvenceObj(Entity *obj, int speed, double direction);
int ywCanvasMoveObj(Entity *obj, Entity *pos);
int ywCanvasMoveObjXY(Entity *obj, int x, int y);
Entity *ywCanvasObjFromIdx(Entity *wid, int idx);
int ywCanvasIdxFromObj(Entity *wid, Entity *obj);

void ywCanvasObjSetPos(Entity *obj, int x, int y);
void ywCanvasObjSetPosByEntity(Entity *obj, Entity *p);

int ywCanvasObjIsOut(Entity *wid, Entity *obj);

void ywCanvasDisableWeight(Entity *w);
void ywCanvasEnableWeight(Entity *w);

int ywCanvasSetWeight(Entity *wid, Entity *canvas, int weight);

Entity *ywCanvasNewObj(Entity *wid, int x, int y, int id);
/**
 * @param rect is not a rect but an array entity containing
 * the size at index 0, and a string of the color at index 1
 * x, and y are alerady used to define the starting point of the rect
 */
Entity *ywCanvasNewRect(Entity *wid, int x, int y, Entity *rect);

static inline  Entity *ywCanvasNewRectangle(Entity *wid, int x, int y, int w,
					    int h, const char *str)
{
	yeAutoFree Entity *rect = yeCreateArray(NULL, NULL);
	Entity *obj;

	ywPosCreateInts(w, h, rect, NULL);
	yeCreateString(str, rect, NULL);
	obj = ywCanvasNewRect(wid, x, y, rect);
	return obj;
}

Entity *ywCanvasNewTextByStr(Entity *wid, int x, int y, const char *str);
Entity *ywCanvasNewText(Entity *wid, int x, int y, Entity *string);
Entity *ywCanvasNewTextExt(Entity *wid, int x, int y, Entity *string,
			   const char *color);


Entity *ywCanvasNewImgByPath(Entity *wid, int x, int y, const char *path);
Entity *ywCanvasNewImg(Entity *wid, int x, int y, const char *path,
		       Entity *size);
Entity *ywCanvasNewImgFromTexture(Entity *wid, int x, int y, Entity *yTexture,
				  Entity *img_src_rect);

void ywCanvasRemoveObj(Entity *wid, Entity *obj);

static inline void ywCanvasPopObj(Entity *wid)
{
  yePopBack(yeGet(wid, "objs"));
}

void ywCanvasStringSet(Entity *obj, Entity *newStr);
void ywCanvasObjSetResourceId(Entity *obj, int id);
void ywCanvasObjClearCache(Entity *obj);

/**
 * @brief Swap obj0 position with obj1 in z order
 * this won't move image on the screen, but can put obj0
 * on top of obj1 or the oposit depending of they position
 * this operation change the weight of obj0 and obj1
 */
int ywCanvasSwapObj(Entity *wid, Entity *obj0, Entity *obj1);

YCanvasObjType ywCanvasObjType(Entity *obj);

/**
 * @return 1 if @obj0 is in colision with @obj1
 */
_Bool ywCanvasObjectsCheckColisions(Entity *obj0, Entity *obj1);

/**
 * @return 1 if r0 is in colision with obj1
 */
_Bool ywCanvasCheckColisionsRectObj(Entity *r0, Entity *obj1);

/**
 * @return 1 if @obj colide with any object appart itself in @wid
 */
int ywCanvasCheckCollisions(Entity *wid, Entity *obj, Entity *colisionFunc,
			    Entity *colisionFuncArg);

Entity *ywCanvasNewCollisionsArray(Entity *wid, Entity *obj);
Entity *ywCanvasNewCollisionsArrayExt(Entity *wid, Entity *obj,
				      Entity *colisionFunc,
				      Entity *colisionFuncArg);
Entity *ywCanvasNewCollisionsArrayWithRectangle(Entity *wid, Entity *rectangle);

/**
 * @return colision array of obj where add_pos have been add to obj pos
 */
Entity *ywCanvasProjectedColisionArray(Entity *wid, Entity *ojb, Entity *add_pos);

Entity *ywCanvasProjectedArColisionArray(Entity *wid, Entity *rect,
					 Entity *add_pos);

/**
 * turn @obj so the top of the sprite point in the direction of @point
 */
static inline void ywCanvasObjPointTopTo(Entity *obj, Entity *point)
{
  Entity *pos = ywCanvasObjPos(obj);
  Entity *s = ywCanvasObjSize(NULL, obj);

  ywPosAddXY(pos, ywSizeW(s) / 2, ywSizeH(s) / 2) ;
  ywCanvasRotate(obj, ywPosAngle(pos, point) - 90);
  ywPosSubXY(pos, ywSizeW(s) / 2, ywSizeH(s) / 2) ;
}

static inline void ywCanvasObjPointRightTo(Entity *obj, Entity *point)
{
  Entity *pos = ywCanvasObjPos(obj);
  Entity *s = ywCanvasObjSize(NULL, obj);

  ywPosAddXY(pos, ywSizeW(s) / 2, ywSizeH(s) / 2) ;
  ywCanvasRotate(obj, ywPosAngle(pos, point));
  ywPosSubXY(pos, ywSizeW(s) / 2, ywSizeH(s) / 2) ;
}

static inline Entity *ywCanvasObjMod(Entity *obj)
{
  return yeGet(obj, "$mod");
}

/**
 * Create a texture from a Canvas Objet
 */
Entity *ywCanvasCreateYTexture(Entity *obj, Entity *father, const char *name);

/**
 * fufill @path_array, with a list of pos, that can be use as movement
 * to move @obj to @to_pos
 * speed is the maximum movement that can be made per iteration
 */
int ywCanvasDoPathfinding(Entity *canvas, Entity *obj, Entity *to_pos,
			  Entity *speed, Entity *path_array);

#endif
