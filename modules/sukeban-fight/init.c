/*
**Copyright (C) 2017 Matthias Gatto
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

#include <yirl/entity-script.h>
#include <yirl/game.h>
#include <yirl/container.h>
#include <yirl/menu.h>
#include <yirl/canvas.h>
#include <yirl/texture.h>
#include <yirl/rect.h>
#include <yirl/timer.h>

enum state {
  NORMAL_STATE,
  INSIDE_ANIMATION
};

static Entity *getCanvasWid(Entity *mainWid)
{
  return ywCntGetEntry(mainWid, 0);
}

static void tryLoadTexture(Entity *pose)
{
  if (!yeGet(pose, "_texture"))
    ywTextureNewImg(yeGetStringAt(pose, "img"), NULL, pose, "_texture");
}

static void setAnimationPose(Entity *guy, Entity *pose)
{
  tryLoadTexture(pose);
  if (yeGet(pose, "pos"))
    yeReCreateInt(0, pose, "_cur");
  yeReplaceBack(guy, pose, "_cp");
}

static void createMapObjs(Entity *ent)
{
  Entity *map = yeGet(ent, "_map");
  Entity *canvas = getCanvasWid(ent);
  Entity *size = yeGet(canvas, "wid-pix");
  int widthQuarter = ywRectW(size) / 4;
  int hightThier = ywRectH(size) / 3;

  for (int x = 0; x < 4; ++x) {
    for (int y = 0; y < 3; ++y) {
      Entity *guy = yeGet(yeGet(map, x), y);
      Entity *objSize;
      Entity *obj;
      Entity *pose;

      if (!guy)
	continue;
      pose = yeGetByStr(guy, "poses.base");
      /* use pose, can be refacto later */
      if (yeGet(pose, "id")) {
	obj = ywCanvasNewObj(canvas, 0, 0,
			     yeGetIntAt(pose, "id"));
      } else if (yeGet(pose, "img")) {
	Entity *srcPos = yeGet(pose, "pos");
	Entity *srcRect = NULL;

	if (srcPos) {
	  srcRect = ywRectCreatePosSize(srcPos, yeGet(pose, "size"), NULL, NULL);
	}
	setAnimationPose(guy, pose);
	obj = ywCanvasNewImgFromTexture(canvas, 0, 0, yeGet(pose, "_texture"),
					srcRect);
	yeDestroy(srcRect);
      }
      objSize = ywCanvasObjSize(map, obj);
      yePushBack(guy, obj, "_canvas");
      ywCanvasObjSetPos(obj,
			widthQuarter * x + widthQuarter / 2 - ywPosX(objSize) / 2,
			hightThier * y  + hightThier / 2 - ywPosY(objSize) / 2);
    }
  }
}

static Entity *getMenuCnt(Entity *wid)
{
  return ywCntGetEntry(wid, 1);
}

static Entity *getLoader(Entity *widget)
{
  Entity *menu_cnt = getMenuCnt(widget);
  return yeGet(menu_cnt, "loader");
}

static Entity *getLoaderAt(Entity *main, int pos)
{
  ywMenuGetEntry(getLoader(main), pos);
}

static Entity *getGuyAt(Entity *wid, int at)
{
  return yeGet(getLoaderAt(wid, at), "guy");
}

static Entity *getCurrentGuy(Entity *main)
{
  getGuyAt(main, yeGetIntAt(main, "pcDoingAction"));
}

static Entity *getActionMenu(Entity *widget, int actionIdx)
{
  Entity *guy = getGuyAt(widget, actionIdx);

  return yeGet(guy, "action_menu");
}

static void swapLoadingAction(Entity *ent, int actionIdx)
{
  ywCntPopLastEntry(getMenuCnt(ent));
  ywPushNewWidget(getMenuCnt(ent), getActionMenu(ent, actionIdx), 0);
}

static void swapActionLoading(Entity *ent)
{
  ywCntPopLastEntry(getMenuCnt(ent));
  ywPushNewWidget(getMenuCnt(ent), getLoader(ent), 0);
}

void *sukeFightEndTurn(Entity *main)
{
  Entity *pcDoingActionIdx = yeGet(main, "pcDoingAction");
  Entity *loader;

  if (yeGetInt(pcDoingActionIdx) < 0)
    return (void *)NOTHANDLE;

  loader = getLoaderAt(main, yeGetInt(pcDoingActionIdx));
  ywMenuSetLoaderPercent(loader, 0);
  yeSetInt(pcDoingActionIdx, -1);
  YTimerReset(yeGetDataAt(main, "timer"));
  uint64_t t = YTimerGet(yeGetDataAt(main, "timer")) / 100000;
  swapActionLoading(main);
  return (void *)NOTHANDLE;
}

void *sukeFightPostAction(int nbArg, void **args)
{
  Entity *ent = args[1];
  /* move into the pose */
  Entity *canvas = getCanvasWid(ent);
  Entity *map = yeGet(ent, "_map");

  YE_ARRAY_FOREACH(map, row) {
    YE_ARRAY_FOREACH(row, guy) {
      Entity *pose = yeGet(guy, "_cp");
      Entity *curEnt = yeGet(pose, "_cur");
      int cur = yeGetInt(curEnt) + 1;
      Entity *srcPos = yeGet(pose, "pos");
      Entity *srcRect = NULL;
      Entity *obj;

      if (!curEnt)
	continue;

      if (cur > yeGetIntAt(pose, "len"))
	cur = 0;
      obj = yeGet(guy, "_canvas");
      ywCanvasRemoveObj(canvas, obj);
      srcRect = ywRectCreatePosSize(srcPos, yeGet(pose, "size"), NULL, NULL);
      srcPos = ywCanvasObjPos(obj);
      ywRectSetX(srcRect,
		 ywRectX(srcRect) +
		 (ywRectW(srcRect) + yeGetIntAt(pose, "margin")) * cur);
      obj = ywCanvasNewImgFromTexture(canvas, ywPosX(srcPos), ywPosY(srcPos),
				      yeGet(pose, "_texture"), srcRect);
      yeDestroy(srcRect);
      yeReCreateInt(cur, pose, "_cur");
      yeReplaceBack(guy, obj, "_canvas");
    }
  }
  ywContainerUpdate(ent, canvas);
  if (args[0] == NOTHANDLE)
    return (void *)ACTION;
}

void *sukeFightAction(int nbArg, void **args)
{
  Entity *ent = args[0];
  Entity *eves = args[1];
  Entity *eve;
  Entity *pcDoingAction = yeGet(ent, "pcDoingAction");

  YEVE_FOREACH(eve, eves) {
    // for debug, to be remove
    if (ywidEveType(eve) == YKEY_DOWN && ywidEveKey(eve) == Y_ESC_KEY) {
      yFinishGame();
      return (void *)ACTION;
    }
  }

  if (yeGetIntAt(ent, "state") == INSIDE_ANIMATION) {
    yDoAnimation(ent, "action_anim");
    return (void *)ACTION;
  }

  /* let menu handle actions */
  if (yeGetInt(pcDoingAction) >= 0) {
    return (void *)NOTHANDLE;
  }


  void *timer = yeGetDataAt(ent, "timer");
  uint64_t t = YTimerGet(timer) / 100000;
  Entity *loader = getLoader(ent);
  ywContainerUpdate(ent, loader);


  for (int j = 0; j < 6; ++j) {
    Entity *curLoadingBar = ywMenuGetEntry(loader, j);
    Entity *guy = yeGet(curLoadingBar, "guy");

    if (!guy)
      break;
    Entity *curTime = yeGet(guy, "current-reload-time");

    yeSetInt(curTime, t + yeGetIntAt(guy, "start-reload-time"));
    Entity *cur_bar = ywCntGetEntry(loader, j);
    Entity *percent = yeGet(cur_bar, "loading-bar-%");

    yeSetInt(percent, yeGetInt(curTime) * 100 /
	     yeGetIntAt(guy, "reload-time"));
    if (yeGetInt(percent) >= 100) {
      YTimerReset(timer);
      yeSetInt(pcDoingAction, j);
      yeReCreateInt(0, guy, "start-reload-time");

      for (int i = 0; i < 6; ++i) {
	if (i == j)
	  continue;
	Entity *curLoadingBar = ywMenuGetEntry(loader, i);
	Entity *guy = yeGet(curLoadingBar, "guy");
	Entity *curTime = yeGet(guy, "current-reload-time");

	yeReCreateInt(yeGetInt(curTime), guy, "start-reload-time");
      }
      swapLoadingAction(ent, j);
      return (void *)NOTHANDLE;
    }
  }

  return (void *)NOTHANDLE;
}

static void tryPushGuyToMenu(Entity *menu_cnt, Entity *guy)
{
  Entity *name = yeGet(guy, "name");
  Entity *loader = yeGet(menu_cnt, "loader");
  Entity *loader_entries = yeGet(loader, "entries");
  Entity *menu = ywCntGetEntry(menu_cnt, 0);
  Entity *menu_entries = yeGet(menu, "entries");

  if (name) {
    yeReCreateInt(0, guy, "current-reload-time");
    Entity *menu_entry = yeCreateArray(menu_entries, NULL);
    yeCreateString(yeGetString(name), menu_entry, "text");
    Entity *loader_entry = yeCreateArray(loader_entries, NULL);
    yeCreateString(NULL, loader_entry, "text");
    yeCreateString("loading-bar", loader_entry, "type");
    yeCreateInt(0, loader_entry, "loading-bar-%");
    yePushBack(loader_entry, guy, "guy");
  }
}

void *sukeFightClean(int nbArgs, void **args)
{
  Entity *ent = args[0];

  for (int i = 0; i < 6; ++i) {
    Entity *g = getGuyAt(ent, i);

    if (g)
      yeRemoveChildByStr(g, "main");
  }
  return NULL;
}

void *menuActionAnimation(int nbArg, void **args)
{
  Entity *main = args[0];
  Entity *animation = args[1];

  if (yeGetIntAt(animation, "animation_frame") == 5) {
    yeSetAt(main, "state", NORMAL_STATE);
    yEndAnimation(main, "action_anim");
    sukeFightEndTurn(main);
    return 0;
  }
  return 1;
}

void *menuAction(int nbArg, void **args)
{
  Entity *mn = args[0];
  Entity *cur = ywMenuGetCurrentEntry(mn);
  Entity *main = yeGet(mn, "main");
  Entity *action = yeGet(cur, "_action");
  Entity *animPose = yeGet(cur, "_anim");
  void *ret;

  if (yeType(animPose) == YSTRING) {
    Entity *animeFunc = yeCreateFunction("menuActionAnimation",
					 ygGetTccManager(), NULL, NULL);
    Entity *guy = getCurrentGuy(main);
    Entity *anim = yeCreateArray(NULL, NULL);
    Entity *pose;

    printf("act %p 0: %s 1:  %s\n", yeGet(yeGet(guy, "poses"),
					  yeGetString(animPose)),
	   yeTypeToString(yeType(action)),
	   yeGetString(animPose));
    yePushBack(anim, action, "action");
    yePushBack(anim, yeGet(guy, "_cp"), "old_pose");
    pose = yeGet(yeGet(guy, "poses"), yeGetString(animPose));
    setAnimationPose(guy, pose);
    yInitAnimation(main, anim, animeFunc, "action_anim");
    yeDestroy(animeFunc);
    yeDestroy(anim);
    yeSetAt(main, "state", INSIDE_ANIMATION);
  } else if (yeType(action) == YSTRING) {
    ret = yesCall(ygGet(yeGetString(action)), mn);
  }
  printf("oy\n");
  if (yeGetIntAt(main, "state") != INSIDE_ANIMATION)
    sukeFightEndTurn(main);
  return ret;
}

void makeActionMenu(Entity *guy, Entity *ent)
{
  if (!guy)
    return;
  Entity *actions = yeGet(guy, "actions");

  printf("%d %p\n", yeLen(actions), actions);

  Entity *menu = yeCreateArray(guy, "action_menu");
  yeCreateString("menu", menu, "<type>");
  yePushBack(menu, ent, "main");
  Entity *menu_entries = yeCreateArray(menu, "entries");
  YE_ARRAY_FOREACH(actions, action) {
    Entity *menu_entry = yeCreateArray(menu_entries, NULL);
    Entity *actionInfo = yeGet(action, 1);

    printf("%s %s\n", yeGetStringAt(action, 0), yeGetStringAt(action, 1));
    yePushBack(menu_entry, yeGet(action, 0), "text");
    yeCreateFunction("menuAction", ygGetTccManager(), menu_entry, "action");
    if (yeType(actionInfo) == YSTRING) {
      yePushBack(menu_entry, actionInfo, "_action");
    } else if (yeType(actionInfo) == YARRAY) {
      yePushBack(menu_entry, yeGet(actionInfo, 0), "_anim");
      yePushBack(menu_entry, yeGet(actionInfo, 1), "_action");
    }

    /* menu_entry = yeCreateArray(menu_entries, NULL); */
    /* yeCreateString("run away", menu_entry, "text"); */
    /* yeCreateString("FinishGame", menu_entry, "action"); */
  }
}

void *sukeFightInit(int nbArg, void **args)
{
  Entity *ent = args[0];
  Entity *entries = yeCreateArray(ent, "entries");
  Entity *bad_guys = yeGet(ent, "bad-guys");
  Entity *good_guys = yeGet(ent, "good-guys");
  Entity *canvas;
  Entity *menu_cnt;
  Entity *menu_cnt_entries;
  Entity *menu_caracters_list;
  Entity *menu;
  Entity *menu_entries;
  Entity *menu_entry;
  Entity *map = yeReCreateArray(ent, "_map", NULL);
  Entity *rows[4];
  void *ret;
  Entity *loader;
  Entity *loader_entries;

  yeReCreateInt(NORMAL_STATE, ent, "state");
  yeReCreateInt(1, ent, "current");
  yeReCreateInt(-1, ent, "pcDoingAction");
  yeRemoveChild(ent, "destroy");
  yeRemoveChild(ent, "action");
  yeRemoveChild(ent, "post-action");
  yeCreateFunction("sukeFightClean", ygGetManager("tcc"), ent, "destroy");
  yeCreateFunction("sukeFightAction", ygGetManager("tcc"), ent, "action");
  yeCreateFunction("sukeFightPostAction", ygGetTccManager(),
		   ent, "post-action");

  YTimerReset(yeGetData(yeCreateDataExt(NULL, ent, "timer",
					YE_DATA_USE_OWN_METADATA)));
  for (int i = 0; i < 4; ++i)
    rows[i] = yeCreateArray(map, NULL);
  canvas = yeCreateArray(entries, NULL);
  yeCreateString("canvas", canvas, "<type>");
  yeReplaceBack(canvas, yeGet(ent, "resources"), "resources");
  Entity *good_front_row = yeGet(good_guys, 0);
  Entity *good_back_row = yeGet(good_guys, 1);
  Entity *bad_front_row = yeGet(bad_guys, 0);
  Entity *bad_back_row = yeGet(bad_guys, 1);

  for (int i = 0; i < 3; ++i) {
    Entity *guy = yeGet(good_front_row, i);
    if (guy) {
      yePushAt(rows[2], guy, i);
    }
    guy = yeGet(good_back_row, i);
    if (guy) {
      yePushAt(rows[3], guy, i);
    }
    guy = yeGet(bad_front_row, i);
    if (guy) {
      yePushAt(rows[1], guy, i);
    }
    guy = yeGet(bad_back_row, i);
    if (guy) {
      yePushAt(rows[0], guy, i);
    }
  }
  yeCreateInt(70, canvas, "size");
  menu_cnt = yeCreateArray(entries, NULL);
  yeCreateString("container", menu_cnt, "<type>");
  menu_cnt_entries = yeCreateArray(menu_cnt, "entries");
  yeCreateString("vertical", menu_cnt, "cnt-type");
  yeReCreateInt(2, menu_cnt, "current");
  yeCreateString("rgba: 123 123 255 255", menu_cnt, "background");

  menu_caracters_list = yeCreateArray(menu_cnt_entries, NULL);
  yeCreateString("menu", menu_caracters_list, "<type>");
  yeReCreateInt(-1, menu_caracters_list, "current");
  yeCreateInt(20, menu_caracters_list, "size");
  menu_entries = yeCreateArray(menu_caracters_list, "entries");

  loader = yeCreateArray(menu_cnt, "loader");

  yePushBack(menu_cnt_entries, loader, NULL);
  yeCreateString("menu", loader, "<type>");
  yeCreateArray(loader, "entries");
  yeReCreateInt(-1, loader, "current");

  for (int i = 0; i < 3; ++i) {
    Entity *guy = yeGet(good_front_row, i);

    makeActionMenu(guy, ent);
    if (guy)
      tryPushGuyToMenu(menu_cnt, guy);
    guy = yeGet(good_back_row, i);
    makeActionMenu(guy, ent);
    if (guy)
      tryPushGuyToMenu(menu_cnt, guy);
  }

  ret = ywidNewWidget(ent, "container");
  createMapObjs(ent);
  return ret;
}

void *init_sukeban_fight(int nbArg, void **args)
{
  Entity *mod = args[0];
  Entity *init = yeCreateArray(NULL, NULL);

  yeCreateString("sukeban-fight", init, "name");
  yeCreateFunction("sukeFightInit", ygGetManager("tcc"), init, "callback");
  ywidAddSubType(init);

  return NULL;
}
