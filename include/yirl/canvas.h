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
  YCanvasImg
} YCanvasObjType;

int ywCanvasInit(void);
int ywCanvasEnd(void);
int ysdl2RegistreCanvas(void);
/**
 * add @pos to object position at @objIdx
 */
int ywCanvasMoveObjByIdx(Entity *wid, int objIdx, Entity *pos);

Entity *ywCanvasObjSize(Entity *wid, Entity *obj);

Entity *ywCanvasObjPos(Entity *obj);

Entity *ywCanvasObjFromIdx(Entity *wid, int idx);
int ywCanvasIdxFromObj(Entity *wid, Entity *obj);

void ywCanvasObjSetPos(Entity *obj, int x, int y);

Entity *ywCanvasNewObj(Entity *wid, int x, int y, int id);
Entity *ywCanvasNewRect(Entity *wid, int x, int y, Entity *rect);
Entity *ywCanvasNewText(Entity *wid, int x, int y, Entity *string);
Entity *ywCanvasNewImgByPath(Entity *wid, int x, int y, const char *path);

void ywCanvasRemoveObj(Entity *wid, Entity *obj);

void ywCanvasStringSet(Entity *obj, Entity *newStr);
void ywCanvasObjSetResourceId(Entity *obj, int id);

YCanvasObjType ywCanvasObjType(Entity *obj);

Entity *ywCanvasNewCollisionsArray(Entity *wid, Entity *obj);
Entity *ywCanvasNewCollisionsArrayWithRectangle(Entity *wid, Entity *obj);

#endif
