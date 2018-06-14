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
  YCanvasImg
} YCanvasObjType;

typedef enum  {
  YCanvasForceSize,
  YCanvasRotate,
} YCanvasModifier;

#define YCANVAS_IMG_IDX 5

typedef union {
  uint32_t i;
  uint8_t rgba[4];
} YCanvasPixiel;

int ywCanvasInit(void);
int ywCanvasEnd(void);
int ysdl2RegistreCanvas(void);
/**
 * add @pos to object position at @objIdx
 */
int ywCanvasMoveObjByIdx(Entity *wid, int objIdx, Entity *pos);

Entity *ywCanvasObjSize(Entity *wid, Entity *obj);

Entity *ywCanvasObjPos(Entity *obj);

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

Entity *ywCanvasNewObj(Entity *wid, int x, int y, int id);
Entity *ywCanvasNewRect(Entity *wid, int x, int y, Entity *rect);
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

int ywCanvasSwapObj(Entity *wid, Entity *obj0, Entity *obj1);

YCanvasObjType ywCanvasObjType(Entity *obj);

/**
 * @return 1 if @obj0 is in colision with @obj1
 */
int ywCanvasObjectsCheckColisions(Entity *obj0, Entity *obj1);

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

#endif
