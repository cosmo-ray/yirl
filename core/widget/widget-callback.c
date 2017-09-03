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

int ywidAddSignal(Entity *wid, const char *name)
{
  int64_t idx;
  Entity *sins = yeGet(wid, "signals");
  Entity *sin;

  if (!sins) {
    sins = yeCreateArray(wid, "signals");
    if (!sins)
      return -1;
  }
  sin = yeGetByStrExt(sins, name, &idx);
  if (sin)
    return idx;
  yeReCreateInt(-1, sins, name);
  yeGetByStrExt(sins, name, &idx);
  return idx;
}

int ywidBindBySinIdx(YWidgetState *wid, int idx, Entity *callback)
{
  if (unlikely(!callback || !wid))
    return -1;
  return yeReplace(wid->signals, yeGet(wid->signals, idx), callback);
}

int ywidBind(YWidgetState *wid, const char *signal, Entity *callback)
{
  if (unlikely(!callback || !wid))
    return -1;
  return yeReplace(wid->signals, yeGet(wid->signals, signal), callback);
}

Entity *ywidCreateFunction(const char *name, void *manager,
			   Entity *wid, const char *sinName)
{
  Entity *sins = yeGet(wid, "signals");
  Entity *tmp = yeCreateFunction(name, manager, NULL, NULL);

  if (!sins) {
    sins = yeCreateArray(wid, "signals");
    yePushBack(sins, tmp, sinName);
  } else {
    yeReplace(sins, yeGet(sins, ywidAddSignal(wid, sinName)), tmp);
  }
  yeDestroy(tmp);
  return tmp;
}

InputStatue ywidCallSignalFromEntity(Entity *wid, Entity *eve, Entity *arg,
				     int idx)
{
  return (InputStatue)yesCall(ywidGetSignal(wid, idx), wid, eve, arg);
}

InputStatue ywidCallSignal(YWidgetState *wid, Entity *eve, Entity *arg, int idx)
{
  return (InputStatue)yesCall(yeGet(wid->signals, idx), wid->entity, eve, arg);
}
