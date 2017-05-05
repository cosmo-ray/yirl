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

#ifndef	_YIRL_MENU_H_
#define	_YIRL_MENU_H_ 10

#include "yirl/widget.h"

int ywMenuInit(void);
int ywMenuEnd(void);
int ywMenuGetCurrent(YWidgetState *opac);
int ywMenuHasChange(YWidgetState *opac);
int ywMenuPosFromPix(Entity *wid, uint32_t x, uint32_t y);
int ywMenuReBind(Entity *entity);

InputStatue ywMenuCallActionOnByEntity(Entity *opac, YEvent *event, int idx,
				       void *arg);
InputStatue ywMenuCallActionOnByState(YWidgetState *opac, YEvent *event, int idx,
				      void *arg);

#ifndef Y_INSIDE_TCC

#define ywMenuCallActionOn(wid, eve, idx, arg)		\
  _Generic((wid),					\
	   Entity * : ywMenuCallActionOnByEntity,	\
	   YWidgetState * : ywMenuCallActionOnByState	\
	   )(wid, eve, idx, arg)

#else

#define ywMenuCallActionOn ywMenuCallActionOnByEntity

#endif

#endif
