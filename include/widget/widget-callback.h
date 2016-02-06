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

typedef enum {
  YCALLBACK_NATIVE = 0,
  YCALLBACK_ENTITY,
  YCALLBACK_NONE
} YCallbackType;

typedef struct {
  int callbackIdx;
  char *name;
} YSignal;

typedef struct {
  char *name;
  /* Can be a entity script to call, or a native func from a widget */
  int type;
} YCallback;

typedef struct {
  YCallback base;
  int (*callack)(YWidgetState *wid, YEvent *eve, Entity *arg);
} YNativeCallback;

typedef struct {
  YCallback base;
  Entity *callback;
} YEntityCallback;

int ywidAddSignal(YWidgetState *wid, const char *name);
void ywidFinishSignal(YWidgetState *wid);

int ywidBind(YWidgetState *wid, const char *signal, const char *callback);
int ywidBindBySinIdx(YWidgetState *wid, int, const char *callback);

int ywinAddCallback(YCallback *callback);

YCallback *ywinCreateNativeCallback(const char *name,
				    int (*callack)(YWidgetState *wid,
						   YEvent *eve, Entity *arg));

/**
 * Create a callback from an entity.
 *
 * @callback: the functions entity use as a callback
 */
YCallback *ywinCreateEntityCallback(const char *name,
				    Entity *callback);

void ywidDdestroyCallback(int idx);

int ywidInitCallback(void);

void ywidFinishCallbacks(void);

YCallback * ywinGetCallbackByIdx(int idx);

YCallback *ywinGetCallbackByStr(const char *str);

int ywidCallSignal(YWidgetState *wid, YEvent *eve, Entity *arg, int idx);

int ywidCallCallback(YCallback *callback, YWidgetState *wid,
		     YEvent *eve, Entity *arg);

int ywidCallCallbackByIdx(YWidgetState *wid, YEvent *eve, Entity *arg, int idx);

static inline int ywidCallCallbackByStr(const char *str,
					YWidgetState *wid,
					YEvent *eve, Entity *arg)
{
  return ywidCallCallback(ywinGetCallbackByStr(str), wid, eve, arg);
}

#endif
