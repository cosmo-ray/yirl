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

static void shooterSpamBullet(YWidgetState *wid, int x, int y)
{
  Entity *pos = ywMapGetPos(wid);
  int posX = yeGetInt(yeGet(pos, "x")) + x;
  int posY = yeGetInt(yeGet(pos, "y")) + y;
  Entity *bullet = NULL;

  static Entity *bulletSprite = NULL;
  static Entity *bulletManager = NULL;

  if (!bulletManager) {
    /* We add this inside wid->entity, like this when destroying wid->entity
     * bulletSprite and bulletManager will be destroy too :) */
    bulletSprite = yeCreateInt(2, wid->entity, "$bullet-sprite");
    bulletManager = yeCreateArray(wid->entity, "$bullet-manager");
  }
  bullet = yeCreateArray(bulletManager, NULL);
  ywMapCreatePos(posX, posY, bullet, "pos");
  yePushBack(bullet, bulletSprite, "id");

  ywMapPushElem(wid, yeGet(bullet, "id"), posX, posY, "bl");
}

int shooterAction(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  InputStatue ret = NOTHANDLE;

  if (eve->type != YKEY_DOWN) {
    return ret;
  }

  switch (eve->key) {

    /* move cases */
  case Y_DOWN_KEY:
    move(wid, 0, 1);
    goto end_switch;
  case Y_UP_KEY:
    move(wid, 0, -1);
    goto end_switch;
  case Y_RIGHT_KEY:
    move(wid, 1, 0);
    goto end_switch;
  case Y_LEFT_KEY:
    move(wid, -1, 0);
    goto end_switch;

    /* shoot cases */
  case 's':
    shooterSpamBullet(wid, 0, 1);
    goto end_switch;
  case 'w':
    shooterSpamBullet(wid, 0, -1);
    goto end_switch;
  case 'd':
    shooterSpamBullet(wid, 1, 0);
    goto end_switch;
  case 'a':
    shooterSpamBullet(wid, -1, 0);
    goto end_switch;

    /* exit */
  case 'q':
    ywidCallCallbackByStr("FinishGame", wid, eve, arg);
    goto end_switch;    
  end_switch:
    ret = ACTION;
  default:
    break;
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
