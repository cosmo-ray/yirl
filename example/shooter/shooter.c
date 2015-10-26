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

#define MAP_SIZE_W 40
#define MAP_SIZE_H 40

static void move(YWidgetState *wid, int x, int y)
{
  Entity *pos = ywMapGetPos(wid);
  Entity *cur = ywMapGetCurrentCase(wid);
  Entity *curHero;
  Entity *posX = yeGet(pos, "x");
  Entity *posY = yeGet(pos, "y");

  for (unsigned int i = 0; i < yeLen(cur); ++i) {
    curHero = yeGet(cur, i);
    if (yeGetInt(curHero) == 1) {
      /* You can get it Noww !!!! */
      yeOpsAddInt(posX, x);
      yeOpsAddInt(posY, y);
      ywMapPushElem(wid, curHero, yeGetInt(posX), yeGetInt(posY), "hr");
      yeRemoveChild(cur, curHero);
      break;
    }
  }
}

int shooterAction(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  InputStatue ret = NOTHANDLE;

  (void)wid;
  (void)arg;
  if (eve->type != YKEY_DOWN) {
    return ret;
  }
  if (eve->key == '\t') {
    ywidCallCallbackByStr("FinishGame", wid, eve, arg);
    ret = ACTION;
  }

  if (eve->key == '\n') {
    move(wid, 0, 1);
    ret = ACTION;
  }
  return ret;
}

int shooterInit(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  Entity *tmp;
  Entity *pos;

  (void)eve;
  yeCreateInt(MAP_SIZE_W, arg, "width");
  arg = yeCreateArray(arg, "map");
  for (int i = 0; i < MAP_SIZE_W * MAP_SIZE_H; ++i) {
    tmp = yeCreateArray(arg, NULL);
    yeCreateInt(0, tmp, NULL);
  }

  ((YMapState *)wid)->pos = yeCreateArray(NULL, NULL);
  pos = ywMapGetPos(wid);
  yeCreateInt(MAP_SIZE_W / 2, pos, "x");
  yeCreateInt(MAP_SIZE_H / 2, pos, "y");

  tmp = ywMapGetCase(wid, MAP_SIZE_W / 2, MAP_SIZE_H / 2);
  yeCreateInt(1, tmp, "hr");

  ywinAddCallback(ywinCreateNativeCallback("shooterAction", shooterAction));  
  ywidBind(wid, "action", "shooterAction");
  
  return NOTHANDLE;
}
