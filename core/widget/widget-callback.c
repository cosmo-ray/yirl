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

int ywidAddSignal(YWidgetState *wid, const char *name)
{
  YSignal *ret = g_new(YSignal, 1); 

  ret->name = g_strdup(name);
  if (!ret->name) {
    g_free(ret->name);
    return -1;
  }
  ret->callbackIdx = -1;
  wid->signals = g_array_append_val(wid->signals, ret);
  if (!callbacks)
    return -1;
  return wid->signals->len - 1;
}

static YSignal *getSinByStr(YWidgetState *wid, const char *str)
{
  YSignal *ret;

  for (unsigned int i = 0;
       (ret = g_array_index(wid->signals, YSignal *, i)) != NULL; ++i) {

    if (yuiStrEqual(ret->name, str))
      return ret;
  }
  return NULL;
}

static unsigned int getCallbackIdx(const char *str)
{
  unsigned int ret = yeArrayIdx(callbacks, str);
  return ret;
}

int ywidBindBySinIdx(YWidgetState *wid, int idx, const char *callback)
{
  YSignal *sin = g_array_index(wid->signals, YSignal *, idx);

  if (!sin || !callback)
    return -1;
  sin->callbackIdx = getCallbackIdx(callback);
  if (sin->callbackIdx < 0)
    return -1;
  return 0;
}

int ywidBind(YWidgetState *wid, const char *signal, const char *callback)
{
  YSignal *sin = getSinByStr(wid, signal);
  if (!sin || !callback)
    return -1;

  sin->callbackIdx = getCallbackIdx(callback);
  if (sin->callbackIdx < 0)
    return -1;
  return 0;
}

int ywidCallSignal(YWidgetState *wid, YEvent *eve, Entity *arg, int idx)
{
  YSignal *signal;

  if (idx < 0)
    return -1;
  signal = g_array_index(wid->signals, YSignal *, idx);
  if (!signal || signal->callbackIdx < 0)
    return -1;
  return ywidCallCallbackByIdx(wid, eve, arg, signal->callbackIdx);
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
  YSignal *ret;

  for (unsigned int i = 0;
       (ret = g_array_index(wid->signals, YSignal *, i)) != NULL; ++i)
    g_free(ret->name);
  g_array_free(wid->signals, 0);
}
