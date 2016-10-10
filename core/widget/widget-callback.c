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
#include <glib.h>

int ywidAddSignalByEntity(Entity *wid, const char *name)
{
  int64_t idx;
  Entity *sin = yeGet(wid, "signals");

  if (!sin) {
    sin = yeCreateArray(wid, "signals");
  }
  yeReCreateInt(-1, sin, name);
  yeGetByStrExt(sin, name, &idx);
  return idx;
}

int ywidAddSignalByWid(YWidgetState *wid, const char *name)
{
  int64_t idx;

  yeReCreateInt(-1, wid->signals, name);
  yeGetByStrExt(wid->signals, name, &idx);
  return idx;
}

int ywidBindBySinIdx(YWidgetState *wid, int idx, Entity *callback)
{ 
  if (callback)
    return yeReplace(wid->signals, yeGet(wid->signals, idx), callback);
  return -1;
}

int ywidBind(YWidgetState *wid, const char *signal, Entity *callback)
{
  if (callback) {
    int ret = yeReplace(wid->signals, yeGet(wid->signals, signal), callback);
    return ret;
  }
  return -1;
}

InputStatue ywidCallSignal(YWidgetState *wid,
			   YEvent *eve,
			   Entity *arg,
			   int idx)
{
  return (InputStatue)yesCall(yeGet(wid->signals, idx), wid, eve, arg);
}
