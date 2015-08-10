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

YCallback *ywinCreateNativeCallback(const char *name,
				    int (*callack)(YWidgetState *wid, void *arg))
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

int ywinAddCallback(YWidgetState *wid,  YCallback *callback)
{
  wid->callbacks = g_array_append_val(wid->callbacks, callback);
  return (wid->callbacks != NULL);
}

YCallback * ywinGetCallbackByIdx(YWidgetState *wid, int idx)
{
  return (g_array_index(wid->callbacks, YCallback *, idx));
}

YCallback *ywinGetCallbackByStr(YWidgetState *wid, const char *str)
{
  for (unsigned int i = 0; i < wid->callbacks->len; ++i) {
    YCallback *ret = g_array_index(wid->callbacks, YCallback *, i);

    if (yuiStrEqual(ret->name, str))
      return ret;
  }
  return NULL;
}
