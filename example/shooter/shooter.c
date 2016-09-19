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

#include "timer.h"
#include "shooter.h"

#define MAP_SIZE_W 40
#define MAP_SIZE_H 40

#define OUT_LEFT 10
#define OUT_RIGHT 11
#define OUT_TOP 20
#define OUT_BOTOM 21

static inline int isOut(YWidgetState *wid, Entity *pos)
{
  Entity *entX = yeGet(pos, "x");
  Entity *entY = yeGet(pos, "y");
  int posX = yeGetInt(yeGet(pos, "x"));
  int posY = yeGetInt(yeGet(pos, "y"));

  if (posX < 0) {
    yeSetInt(entX, 0);
    return OUT_LEFT;
  } else if (posX > ywMapW(wid->entity) - 1) {
    yeSetInt(entX, ywMapW(wid->entity) - 1);
    return OUT_RIGHT;
  }

  if (posY < 0) {
    yeSetInt(entY, 0);
    return OUT_TOP;
  } else if (posY > ywMapH(wid->entity) - 1) {
    yeSetInt(entY, ywMapH(wid->entity) - 1);
    return OUT_BOTOM;
  }
  return 0;
}

static int removeBullet(YWidgetState *wid, Entity *obj)
{
  Entity *bulletManager = yeGet(wid->entity, "$bullet-manager");
  Entity *id = yeGet(obj, "id");
  Entity *pos = yeGet(obj, "pos");

  ywMapRemove(wid->entity, pos, id);
  yeRemoveChild(bulletManager, obj);
  return 0;
}

static int move(YWidgetState *wid, Entity *obj, Entity *dir)
{
  Entity *id = yeGet(obj, "id");
  Entity *pos = yeGet(obj, "pos");
  Entity *cur = ywMapGetCase(wid->entity, pos);
  Entity *posX = yeGet(pos, "x");
  Entity *posY = yeGet(pos, "y");
  int ret = isOut(wid, pos);

  if (!!ret)
    return removeBullet(wid, obj);

  yeRemoveChild(cur, id);
  yeAddEnt(posX, yeGet(dir, "x"));
  yeAddEnt(posY, yeGet(dir, "y"));
  ywMapPushElem(wid->entity, id, pos, "bl");
  return 0;
}

static int shooterHandleBullets(YWidgetState *wid)
{
  Entity *bulletManager = yeGet(wid->entity, "$bullet-manager");

  if (!bulletManager)
    return 0;

  YE_ARRAY_FOREACH(bulletManager, bullet) {
    if (!bullet)
      continue;
    Entity *speedAndDir = yeGet(bullet, "speedAndDir");

    move(wid, bullet, speedAndDir);
  }
  return ACTION;
}

static void moveMainCaracter(YWidgetState *wid, int x, int y)
{
  Entity *pos = yeGet(wid->entity, "pos");
  Entity *cur = ywMapGetCase(wid->entity, pos);
  Entity *posX = yeGet(pos, "x");
  Entity *posY = yeGet(pos, "y");


  for (unsigned int i = 0; i < yeLen(cur); ++i) {
      Entity *curHero = yeGet(cur, i);
    if (yeGetInt(curHero) == 1) {
      /* You can get it Noww !!!! */
      yeAddInt(posX, x);
      yeAddInt(posY, y);

      if (yeGetInt(posX) < 0)
	yeSetInt(posX, 0);
      else if (yeGetInt(posX) >= ywMapW(wid->entity) - 1)
	yeSetInt(posX, ywMapW(wid->entity) - 1);
      if (yeGetInt(posY) < 0)
	yeSetInt(posY, 0);
      else if (yeGetInt(posY) >= ywMapH(wid->entity) - 1)
	yeSetInt(posY, ywMapH(wid->entity) - 1);

      ywMapPushElem(wid->entity, curHero, pos, "hr");
      yeRemoveChild(cur, curHero);
      break;
    }
  }
}

static void shooterSpamBullet(YWidgetState *wid, int x, int y)
{
  Entity *pos = yeGet(wid->entity, "pos");
  int posX = yeGetInt(yeGet(pos, "x"));
  int posY = yeGetInt(yeGet(pos, "y"));
  Entity *bullet = NULL;

  Entity *bulletSprite = yeGet(wid->entity, "$bullet-sprite");
  Entity *bulletManager = yeGet(wid->entity, "$bullet-manager");
  static YTimer *bulletTimeout = NULL;

  if (!bulletTimeout)
    bulletTimeout = YTimerCreate();

  if (YTimerGet(bulletTimeout) < 100000)
    return;
  YTimerReset(bulletTimeout);
  if (!yeGet(wid->entity, "$bullet-manager")) {
    /* We add this inside wid->entity, because we want to destroy
     * bulletSprite and bulletManager at the same time than wid->entity */
    bulletSprite = yeCreateInt(2, wid->entity, "$bullet-sprite");
    bulletManager = yeCreateArray(wid->entity, "$bullet-manager");
  }
  bullet = yeCreateArray(bulletManager, NULL);
  ywPosCreate(posX, posY, bullet, "pos");
  ywPosCreate(x, y, bullet, "speedAndDir");
  yePushBack(bullet, bulletSprite, "id");

  ywMapPushElem(wid->entity, bulletSprite, pos, "bl");
}

static int shooterActionInt(YWidgetState *wid, YEvent *eve)
{
  InputStatue ret = NOTHANDLE;

  if (eve->type != YKEY_DOWN) {
    return ret;
  }

  switch (eve->key) {
    /* move cases */
  case Y_DOWN_KEY:
    moveMainCaracter(wid, 0, 1);
    goto end_switch;
  case Y_UP_KEY:
    moveMainCaracter(wid, 0, -1);
    goto end_switch;
  case Y_RIGHT_KEY:
    moveMainCaracter(wid, 1, 0);
    goto end_switch;
  case Y_LEFT_KEY:
    moveMainCaracter(wid, -1, 0);
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
    sound_stop("42");
    ygCall(NULL, "FinishGame");
    goto end_switch;
  end_switch:
    ret = ACTION;
  default:
    break;
  }
  return ret;
}

void *shooterAction(va_list ap)
{
  //YWidgetState *wid, YEvent *eve, Entity *arg
  YWidgetState *wid = va_arg(ap, YWidgetState *);
  YEvent *eve = va_arg(ap, YEvent *);

  shooterHandleBullets(wid);
  YEvent *curEve;

  YEVE_FOREACH(curEve, eve) {
    shooterActionInt(wid, curEve);
  }
  return (void *)ACTION;
}

void *shooterInit(va_list ap)
{
  YWidgetState *wid = va_arg(ap, YWidgetState *);
  Entity *arg;
  Entity *tmp;
  Entity *pos;

  va_arg(ap, YEvent *);
  arg = va_arg(ap, Entity *);
  sound_play_loop("42", "BlablablaMrFreeman.mp3");

  yeCreateInt(MAP_SIZE_W, arg, "width");
  if (!(pos = yeGet(arg, "pos")))
    pos = ywPosCreate(0, 0, arg, "pos");
  arg = yeCreateArray(arg, "map");
  for (int i = 0; i < MAP_SIZE_W * MAP_SIZE_H; ++i) {
    tmp = yeCreateArray(arg, NULL);
    yeCreateInt(0, tmp, NULL);
  }

  yeSetInt(yeGet(pos, "x"), MAP_SIZE_W / 2);
  yeSetInt(yeGet(pos, "y"), MAP_SIZE_H / 2);

  tmp = ywMapGetCase(wid->entity, pos);
  yeCreateInt(1, tmp, "hr");

  ysRegistreFunc(ysNativeManager(), "shooterAction", shooterAction);
  ygBind(wid, "action", "shooterAction");

  return (void *)NOTHANDLE;
}
