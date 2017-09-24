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

#include <yirl/game.h>
#include <yirl/container.h>
#include <yirl/canvas.h>
#include <yirl/rect.h>

void rendMap(Entity *ent)
{
  Entity *map = yeGet(ent, "_map");
  Entity *canvas = ywCntGetEntry(ent, 0);
  Entity *objs = yeGet(canvas, "objs");
  Entity *size = yeGet(canvas, "wid-pix");
  int widthQuarter = ywRectW(size) / 4;
  int hightThier = ywRectH(size) / 3;

  printf("%d %d\n", widthQuarter, hightThier);
  for (int x = 0; x < 4; ++x) {
    for (int y = 0; y < 3; ++y) {
      Entity *obj = yeGet(yeGet(map, x), y);
      Entity *objSize;

      if (!obj)
	continue;
      objSize = ywCanvasObjSize(ent, obj);
      yeRemoveChildByStr(obj, "pos");
      ywPosCreate(widthQuarter * x + widthQuarter / 2 - ywPosX(objSize) / 2,
		  hightThier * y  + hightThier / 2 - ywPosY(objSize) / 2,
		  obj, "pos");
      yePushBack(objs, obj, NULL);
      printf("%p - %s\n", obj, ywPosToString(objSize));
    }
  }
}

void *sukeFightInit(int nbArg, void **args)
{
  Entity *ent = args[0];
  Entity *entries = yeCreateArray(ent, "entries");
  Entity *bad_guys = yeGet(ent, "bad-guys");
  Entity *good_guys = yeGet(ent, "good-guys");
  Entity *canvas;
  Entity *menu;
  Entity *menu_entries;
  Entity *menu_entry;
  Entity *map = yeReCreateArray(ent, "_map", NULL);
  Entity *rows[4];
   void *ret;

  for (int i = 0; i < 4; ++i)
    rows[i] = yeCreateArray(map, NULL);
  canvas = yeCreateArray(entries, NULL);
  yeCreateString("canvas", canvas, "<type>");
  yeCreateArray(canvas, "objs");
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
  menu = yeCreateArray(entries, NULL);
  yeCreateString("menu", menu, "<type>");
  menu_entries = yeCreateArray(menu, "entries");
  menu_entry = yeCreateArray(menu_entries, NULL);
  yeCreateString("attack", menu_entry, "text");
  ret = ywidNewWidget(ent, "container");
  rendMap(ent);
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
