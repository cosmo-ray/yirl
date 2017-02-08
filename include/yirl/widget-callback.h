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

#ifndef _YIRL_WIDGET_CALLBACK_H_
#define _YIRL_WIDGET_CALLBACK_H_

#include "widget.h"

#define ywidAddSignal(WID, VALUE)					\
  _Generic((WID),							\
	   YWidgetState *: ywidAddSignalByWid,				\
	   Entity *: ywidAddSignalByEntity)(WID, VALUE)

int ywidAddSignalByWid(YWidgetState *wid, const char *name);
int ywidAddSignalByEntity(Entity *wid, const char *name);

int ywidBind(YWidgetState *wid, const char *signal, Entity *callback);
int ywidBindBySinIdx(YWidgetState *wid, int, Entity *callback);

static inline Entity *ywidGetSignal(Entity *wid, int idx)
{
  return yeGetByIdx(yeGetByStrFast(wid, "signals"), idx);
}

/**
 * create function and bind it
 */
Entity *ywidCreateFunction(const char *name, void *manager,
			   Entity *wid, const char *sinName);

InputStatue ywidCallSignal(YWidgetState *wid, YEvent *eve,
			   Entity *arg, int idx);

InputStatue ywidCallSignalFromEntity(Entity *wid, YEvent *eve,
				     Entity *arg, int idx);

#endif
