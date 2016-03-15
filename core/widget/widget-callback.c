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

#include "widget-callback.h"
#include "utils.h"
#include "entity-script.h"

static Entity *callbacks = NULL;

static inline void ywidDdestroyCallbackInt(YCallback *callback)
{
  if (callback) {
    g_free(callback->name);
    g_free(callback);
  }
}

static void callbackDestroy(void *callback)
{
  return ywidDdestroyCallbackInt(callback);
}

int ywidInitCallback(void)
{
  if (callbacks)
    return 0;
  callbacks = yeCreateArray(NULL, NULL);
  return (callbacks == NULL) * -1;
}

void ywidFinishCallbacks(void)
{
  YE_DESTROY(callbacks);
  callbacks = NULL;
}

int ywidCallCallback(YCallback *callback, YWidgetState *wid,
		     YEvent *eve, Entity *arg)
{
  if (!callback)
    return -1;
  switch (callback->type) {
  case YCALLBACK_NATIVE:
    return ((YNativeCallback *)callback)->callack(wid, eve, arg);
    break;
  case YCALLBACK_ENTITY:
    yesCall(((YEntityCallback *)callback)->callback, wid, eve, arg);
    break;
  default:
    break;
  }
  return -1;
}  


int ywidCallCallbackByIdx(YWidgetState *wid, YEvent *eve, Entity *arg, int idx)
{
  return ywidCallCallback(ywinGetCallbackByIdx(idx), wid, eve, arg);
}


YCallback *ywinCreateEntityCallback(const char *name,
				    Entity *callback)
{
  YEntityCallback *ret = g_new(YEntityCallback, 1);

  if (!ret)
    return NULL;
  ret->base.type = YCALLBACK_ENTITY;
  ret->base.name = g_strdup(name);
  if (!ret->base.name)
    return NULL;
  ret ->callback = callback;
  return (YCallback *)ret;
}

YCallback *ywinCreateNativeCallback(const char *name,
				    int (*callack)(YWidgetState *wid,
						   YEvent *eve, Entity *arg))
{
  YNativeCallback *ret = g_new(YNativeCallback, 1);

  if (!ret)
    return NULL;
  ret->base.type = YCALLBACK_NATIVE;
  ret->base.name = g_strdup(name);
  if (!ret->base.name)
    return NULL;
  ret ->callack = callack;
  return (YCallback *)ret;
}

int ywidAddSignalByEntity(Entity *wid, const char *name)
{
  int64_t idx;
  Entity *sin = yeGet(wid, "signals");

  if (!sin) {
    sin = yeCreateArray(wid, "signals");
  }
  yeCreateInt(-1, sin, name);
  yeGetByStrExt(sin, name, &idx);
  return idx;
}

int ywidAddSignalByWid(YWidgetState *wid, const char *name)
{
  int64_t idx;

  yeCreateInt(-1, wid->signals, name);
  yeGetByStrExt(wid->signals, name, &idx);
  return idx;
}

static unsigned int getCallbackIdx(const char *str)
{
  unsigned int ret = yeArrayIdx(callbacks, str);
  return ret;
}

int ywidBindBySinIdx(YWidgetState *wid, int idx, const char *callback)
{
  Entity *sin = yeGet(wid->signals, idx);
  int ret;

  if (!sin || !callback)
    return -1;
  ret = getCallbackIdx(callback);
  yeSetInt(sin, ret);
  if (ret < 0)
    return -1;
  return 0;
}

int ywidBind(YWidgetState *wid, const char *signal, const char *callback)
{
  Entity *sin = yeGet(wid->signals, signal);

  if (!sin || !callback)
    return -1;
  yeSetInt(sin, getCallbackIdx(callback));
  return 0;
}

int ywidCallSignal(YWidgetState *wid, YEvent *eve, Entity *arg, int idx)
{
  Entity *signal;

  if (idx < 0 || !wid)
    return -1;

  signal = yeGet(wid->signals, idx);
  if (!signal || yeGetInt(signal) < 0)
    return -1;
  return ywidCallCallbackByIdx(wid, eve, arg, yeGetInt(signal));
}


int ywinAddCallback(YCallback *callback)
{
  Entity *tmp = yeCreateData(callback, callbacks, callback->name);
  yeSetDestroy(tmp, callbackDestroy);
  return 0;
}

YCallback * ywinGetCallbackByIdx(int idx)
{
  return yeGetData(yeGet(callbacks, idx));
}

YCallback *ywinGetCallbackByStr(const char *str)
{
  return yeGetData(yeGet(callbacks, str));
}

void ywidDdestroyCallback(int idx)
{
  yeDestroy(yeGet(callbacks, idx));
}

void ywidFinishSignal(YWidgetState *wid)
{
  yeRemoveChild(wid->entity, wid->signals);
  wid->signals = NULL;
}
