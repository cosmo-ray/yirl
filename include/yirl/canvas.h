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

/**
 * TODO !!!!!!!!
 * CanvasObj is a super ugly name, so I should rename that to 'canel' (for canvas element)
 * we need to do that in a retrocompatible way, like this V:
 * ywCanvasObjAngle -> ywCanelAngle
 * #define ywCanvasObjAngle ywCanelAngle
 * TODO !!!!!!!!
 */

/**
 * canvas widget:
 * a canvas is composed of multiple objects that are rendable.
 * you have 2 way to use it:
 * you can store canvas object inside the widget
 * (using ywCanvasNewObj/Image/Rect....)
 * Or printing directly inside it using ywCanvasMergeTexture/Rectangle...
 * if you use objects the widget will keep in memory each object,
 * will enable you to set weight to them
 * need to be enable using ywCanvasEnableWeight after widget creation,
 * or with have_weight int entity inside widget
 * move them on screen directly...
 * If you just want to print and forget something,
 * you should go with the merge function
 * (note that this need to be enable at widget
 *  creation using "mergable" atribute in the widget entity)
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
	YCanvasBicolorImg,
	YCanvasHeadacheImg,
	YCnvasEndType
} YCanvasObjType;

typedef enum  {
	YCanvasForceSize,
	YCanvasRotate,
	YCanvasVFlip,
	YCanvasHFlip,
} YCanvasModifier;

enum {
	YC_HAS_WEIGHT = 1,
	YC_MERGE = 1 << 1,
	YC_MERGE_NO_MERGE = 1 << 2,
};

#define YCANVAS_TYPE_IDX 0
#define YCANVAS_POS_IDX 1
#define YCANVAS_DATA_IDX 2
#define YCANVAS_IMG_IDX 6
#define YCANVAS_SIZE_IDX 7
#define YCANVAS_SURFACE_IDX 8
#define YCANVAS_UDATA_IDX 10

typedef union {
	uint32_t i;
	uint8_t rgba[4];
} YCanvasPixiel;

typedef struct {
	YWidgetState sate;
	Entity *merge_texture;
	int flag;
} YCanvasState;

/* VVVVVV YIRL Internal VVVVVVV */
int ywCanvasInit(void);
int ywCanvasEnd(void);
int ysdl2RegistreCanvas(void);
/* ^^^^^^ YIRL Internal ^^^^^^^ */

/**
 * write an obj in a mergable widget
 * the obj is then automaticaly remove from the widget
 */
int ywCanvasMergeObj(Entity *wid, Entity *obj);

/**
 * directly write rect into a mergable wid
 * @param color the color, format: "rgba: XXX,XXX,XXX,XXX" of "0xffffffff"
 */
int ywCanvasMergeRectangle(Entity *wid, int x, int y,
			   int w, int h, const char *color);

/**
 * directly write yTexture into a mergable wid
 */
int ywCanvasMergeTexture(Entity *wid, Entity *yTexture,
			 Entity *srcRect, Entity *dstRect);

/**
 * directly write text into a mergable wid
 * w h are curently unused
 */
int ywCanvasMergeText(Entity *wid, int x, int y, int w, int h,
		      const char * txt);

/**
 * Clear all object of the canvas
 */
void ywCanvasClear(Entity *canvas);

/**
 * @brief helper to create a canvas widget
 */
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

static inline int ywCanvasForceSizeXY(Entity *obj, int x, int y)
{
	yeAutoFree Entity *img_size = ywSizeCreate(x, y, NULL, NULL);

	return ywCanvasForceSize(obj, img_size);
}

static inline int ywCanvasMultiplySize(Entity *obj, double multiplier)
{
	Entity *sz = ywCanvasObjSize(NULL, obj);
	yeAutoFree Entity *img_size = ywSizeCreate(ywSizeW(sz) * multiplier,
						   ywSizeH(sz) * multiplier,
						   NULL, NULL);

	return ywCanvasForceSize(obj, img_size);
}

/**
 * reduce @obj of @percent
 */
static inline int ywCanvasPercentReduce(Entity *obj, int percent)
{
	Entity *s = ywCanvasObjSize(NULL, obj);
	yeAutoFree Entity *ns = ywSizeCreate(percent * ywSizeW(s) / 100,
					     percent * ywSizeH(s) / 100,
					     NULL, NULL);

	return ywCanvasForceSize(obj, ns);
}

/**
 * rotate a canvas object
 */
int ywCanvasRotate(Entity *obj, double angle);

int ywCanvasObjUnsetHFlip(Entity *obj);
int ywCanvasObjUnsetVFlip(Entity *obj);

/**
 * @brief flip an object vertically or horizontally
 */
int ywCanvasVFlip(Entity *obj);
int ywCanvasHFlip(Entity *obj);

int ywCanvasAdvenceObj(Entity *obj, int speed, double direction);
int ywCanvasMoveObj(Entity *obj, Entity *pos);
int ywCanvasMoveObjXY(Entity *obj, int x, int y);

Entity *ywCanvasObjFromIdx(Entity *wid, int idx);
int ywCanvasIdxFromObj(Entity *wid, Entity *obj);

/**
 * ywCanvasObjReplacePos replace the object pos to a reference to p, which might my useful,
 * if for example you want your cam and pos to be the same thing
 */
void ywCanvasObjReplacePos(Entity *obj, Entity *p);

void ywCanvasObjSetPos(Entity *obj, int x, int y);
void ywCanvasObjSetPosByEntity(Entity *obj, Entity *p);

/**
 * @brief return 1 if object out of screen
 */
int ywCanvasObjIsOut(Entity *wid, Entity *obj);

/**
 * enable/disable weight on canvas object
 * weight give you better control on which object if above other objects
 * but can slow down the widgets when they is too much objects
 */
void ywCanvasDisableWeight(Entity *w);
void ywCanvasEnableWeight(Entity *w);

static inline YWidgetState *ywCanvasDisableWeight2(Entity *w)
{
	ywCanvasDisableWeight(w);
	return ywidGetState(w);
}

static inline YWidgetState *ywCanvasEnableWeight2(Entity *w)
{
	ywCanvasEnableWeight(w);
	return ywidGetState(w);
}

/**
 * Set card weight if weight is activated
 * the higter the weight is, the more on top the object will be
 */
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

static inline  Entity *ywCanvasNewRectangleByRect(Entity *wid, Entity *orect, const char *str)
{
	return ywCanvasNewRectangle(wid, ywRectX(orect), ywRectY(orect), ywRectW(orect), ywRectH(orect), str);
}

/**
 * Create horizontal segments
 */
static inline Entity *ywCanvasNewHSegment(Entity *wid, int x, int y, int w, const char *color_str)
{
	return ywCanvasNewRectangle(wid, x, y, w, 1, color_str);
}

/**
 * Create vertical segments
 */
static inline Entity *ywCanvasNewVSegment(Entity *wid, int x, int y, int h, const char *color_str)
{
	return ywCanvasNewRectangle(wid, x, y, 1, h, color_str);
}

Entity *ywCanvasNewTextByStr(Entity *wid, int x, int y, const char *str);
Entity *ywCanvasNewText(Entity *wid, int x, int y, Entity *string);
Entity *ywCanvasNewTextExt(Entity *wid, int x, int y, Entity *string,
			   const char *color);

/**
 * create a image make of 2 colors describe in map, and info.
 * info mustbe like:
 * info[0] = size in pixiels (examples ywSizeCreate(4, 2, info, NULL))
 * info[1] = hex_backgroud_color_number (examples yeCreateInt(0xffffff, info, NULL))
 * info[2] = hex_forground_color_number (examples yeCreateInt(0x000000, info, NULL))
 */
Entity *ywCanvasNewBicolorImg(Entity *wid, int x, int y, uint8_t *map,
			      Entity *info);
int ywCanvasCacheBicolorImg(Entity *obj, const uint8_t *map, Entity *info);

int ywCanvasCacheHeadacheImg(Entity *obj, Entity *map, Entity *info);
Entity *ywCanvasNewHeadacheImg(Entity *c, int x, int y, Entity *map, Entity *info);

Entity *ywCanvasNewImgByPath(Entity *wid, int x, int y, const char *path);
Entity *ywCanvasNewImg(Entity *wid, int x, int y, const char *path,
		       Entity *src_rect);

Entity *ywCanvasNewImgFromTexture(Entity *wid, int x, int y, Entity *yTexture,
				  Entity *img_src_rect);
Entity *ywCanvasNewImgFromTexture2(Entity *wid, int x, int y, Entity *yTexture,
				   Entity *img_src_rect, Entity *img_dst_rect);

static inline int ywCanvasReplace(Entity *wid, Entity *toReplace,
				  Entity *toPush)
{
	return yeReplace(yeGet(wid, "objs"), toReplace, toPush);
}

void ywCanvasRemoveObj(Entity *wid, Entity *obj);

static inline void ywCanvasClearArray(Entity *wid, Entity *array_to_clear)
{
	YE_FOREACH(array_to_clear, el) {
		ywCanvasRemoveObj(wid, el);
	}
	yeClearArray(array_to_clear);
}

static inline void ywCanvasPopObj(Entity *wid)
{
	yePopBack(yeGet(wid, "objs"));
}

static inline Entity *ywCanvasLastObj(Entity *wid)
{
	return yeLast(yeGet(wid, "objs"));
}

/**
 * @brief Set color of a string object
 */
static inline void ywCanvasSetStrColor(Entity *obj, const char *str)
{
	yeCreateStringAt(str, obj, "color", 3);
}

/**
 * @brief get the color of a string obj
 */
static inline const char *ywCanvasStrColor(Entity *obj)
{
	return yeGetStringAt(obj, 3);
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
 * it should be pixiel perfect
 */
_Bool ywCanvasObjectsCheckColisions(Entity *obj0, Entity *obj1);

/**
 * @return 1 if r0 is in colision with obj1
 * it should be pixiel perfect
 */
_Bool ywCanvasCheckColisionsRectObj(Entity *r0, Entity *obj1);

/**
 * @return 1 if @obj colide with any object appart itself in @wid
 * it not very precise, if you want more presision, you can then use
 * ywCanvasObjectsCheckColisions or ywCanvasCheckColisionsRectObj
 */
_Bool ywCanvasCheckCollisions(Entity *wid, Entity *obj, Entity *colisionFunc,
			      Entity *colisionFuncArg);

/**
 * ATTENTION !!!!!! USING THIS IN SCRIPT,
 * YOU WILL HAVE TO CALL yeDestroy MANUALLY
 * @return array containing all object that colide with obj, this need to be free
 */
Entity *ywCanvasNewCollisionsArray(Entity *wid, Entity *obj);

/**
 * @return all objects intersected between pos0 and pos1
 */
Entity *ywCanvasNewIntersectArray(Entity *wid, Entity *pos0, Entity *pos1);

/**
 * @return array containing all object that colide with obj, this need to be free
 */
Entity *ywCanvasNewCollisionsArrayExt(Entity *wid, Entity *obj,
				      Entity *colisionFunc,
				      Entity *colisionFuncArg);

/**
 * @return array containing all object that colide with rectangle, this need to be free
 */
Entity *ywCanvasNewCollisionsArrayWithRectangle(Entity *wid, Entity *rectangle);

/**
 * WARNING: this function is untester and very experimental
 * @return an array of array that represent all collitions with all objRects
 */
Entity *ywCanvasNewCollisionsArrayWithRectangles(Entity *wid, Entity *objRects,
						 Entity *colisionFunc,
						 Entity *colisionFuncArg);

/**
 * @return colision array of obj where add_pos have been add to obj pos
 */
Entity *ywCanvasProjectedColisionArray(Entity *wid, Entity *ojb, Entity *add_pos);

Entity *ywCanvasProjectedArColisionArray(Entity *wid, Entity *rect,
					 Entity *add_pos);

/**
 * turn obj so the top of the sprite point in the direction of point
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

static inline int ywCanvasObjDistanceXY(Entity *obj, int x, int y)
{
	Entity *p0 = ywCanvasObjPos(obj);
	uint32_t w = ywPosX(p0) - x;
	uint32_t h = ywPosY(p0) - y;

	return sqrt(w * w + h * h);
}


/**
 * small function to ease objects manipulation
 */
static inline Entity *ycoRepushObj(Entity *parent, const char *key, Entity *obj)
{
	ywCanvasRemoveObj(parent, yeGet(parent, key));
	yeReplaceBack(parent, obj, key);
	return obj;
}

static inline void ycoRmObj(Entity *parent, const char *key)
{
	ywCanvasRemoveObj(parent, yeGet(parent, key));
	yeRemoveChildByStr(parent, key);
}

#endif
