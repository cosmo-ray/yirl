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

int ywidCallCallback(YWidgetState *wid, YEvent *eve, Entity *arg, unsigned idx)
{
  YCallback *callback = ywinGetCallbackByIdx(wid, idx);

  if (!callback)
    return -1;
  switch (callback->type) {
  case YCALLBACK_NATIVE:
    return ((YNativeCallback *)callback)->callack(wid, eve, arg);
    break;
  default:
    break;
  }
  return -1;
}

static inline void ywidDdestroyCallbackInt(YCallback *callback)
{
  if (callback) {
    g_free(callback->name);
    g_free(callback);
  }
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
  if (!ret->base.name) {
    g_free(ret->base.name);
    return NULL;
  }
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
  if (!wid->callbacks)
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

static unsigned int getCallbackIdx(YWidgetState *wid, const char *str)
{
  YCallback *ret;

  for (unsigned int i = 0;
       (ret = g_array_index(wid->callbacks, YCallback *, i)) != NULL; ++i) {

    if (yuiStrEqual(ret->name, str))
      return (int)i;
  }
  return -1;
}


int ywidBind(YWidgetState *wid, const char *signal, const char *callback)
{
  YSignal *sin = getSinByStr(wid, signal);
  if (!sin)
    return -1;

  sin->callbackIdx = getCallbackIdx(wid, callback);
  if (sin->callbackIdx < 0)
    return -1;
  return 0;
}

int ywidCallSignal(YWidgetState *wid, YEvent *eve, Entity *arg, unsigned idx)
{
  YSignal *signal = g_array_index(wid->signals, YSignal *, idx);

  if (!signal)
    return -1;
  return ywidCallCallback(wid, eve, arg, signal->callbackIdx);
}


int ywinAddCallback(YWidgetState *wid,  YCallback *callback)
{
  wid->callbacks = g_array_append_val(wid->callbacks, callback);
  return (wid->callbacks == NULL) * -1;
}

YCallback * ywinGetCallbackByIdx(YWidgetState *wid, int idx)
{
  return (g_array_index(wid->callbacks, YCallback *, idx));
}

YCallback *ywinGetCallbackByStr(YWidgetState *wid, const char *str)
{
  YCallback *ret;

  for (unsigned int i = 0;
       (ret = g_array_index(wid->callbacks, YCallback *, i)) != NULL; ++i) {

    if (yuiStrEqual(ret->name, str))
      return ret;
  }
  return NULL;
}

void ywidDdestroyCallback(YWidgetState *wid, int idx)
{
  ywidDdestroyCallbackInt(ywinGetCallbackByIdx(wid, idx));
  ywidDdestroyCallbackInt(ywinGetCallbackByIdx(wid, idx));
}

void ywidFinishSignal(YWidgetState *wid)
{
  YSignal *ret;

  for (unsigned int i = 0;
       (ret = g_array_index(wid->signals, YSignal *, i)) != NULL; ++i)
    g_free(ret->name);
  g_array_free(wid->signals, 0);
}

void ywidFinishCallbacks(YWidgetState *wid)
{
  YCallback *ret;

  for (unsigned int i = 0;
       (ret = g_array_index(wid->callbacks, YCallback *, i)) != NULL; ++i)
    ywidDdestroyCallbackInt(ret);
  g_array_free(wid->callbacks, 0);
}
