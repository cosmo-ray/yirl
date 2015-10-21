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

#include "shooter.h"

#define MAP_SIZE_W 5
#define MAP_SIZE_H 5

int shooterAction(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  InputStatue ret = NOTHANDLE;

  (void)wid;
  (void)arg;
  if (eve->key == '\t') {
    ywidCallCallbackByStr("FinishGame", wid, eve, arg);
    ret = ACTION;
  }

  return ret;
}

int shooterInit(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  Entity *tmp;

  (void)wid;
  (void)eve;
  yeCreateInt(MAP_SIZE_W, arg, "width");
  arg = yeCreateArray(arg, "map");
  for (int i = 0; i < MAP_SIZE_W * MAP_SIZE_H; ++i) {
    tmp = yeCreateArray(arg, NULL);
    yeCreateInt(0, tmp, NULL);
  }
  ywinAddCallback(ywinCreateNativeCallback("shooterAction", shooterAction));  
  ywidBind(wid, "action", "shooterAction");
  
  return NOTHANDLE;
}
