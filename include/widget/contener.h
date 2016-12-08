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

#ifndef	_YIRL_CONTENER_H_
#define	_YIRL_CONTENER_H_

#include "widget.h"

typedef enum {
  CNT_VERTICAL,
  CNT_HORIZONTAL,
  CNT_STACK,
  CNT_NONE
} CntType;

typedef struct {
  YWidgetState sate;
  int curent;
  /* Default type is vertical */
  int type;
} YContenerState;

int ywContenerInit(void);
int ywContenerEnd(void);

int ywContenerUpdate(Entity *contener, Entity *widEnt);

#define ywCntType(opac) (((YContenerState *)opac)->type)

#endif
