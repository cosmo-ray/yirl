/*
**Copyright (C) 2016 Matthias Gatto
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

#include <stdio.h>
#include <game.h>
#include <contener.h>

void *init(int nbArgs, void **args)
{
  Entity *mod = args[0];
  Entity *init;
  Entity *map = yeCreateArray(mod, "battle");

  yeCreateString("battle", map, "<type>");
  yePushBack(map, yeGetByStr(mod, "resources.battle-map"), "resources");
  
  yePushBack(map, yeGetByStr(mod, "player1 fleets"), "player 1");
  yePushBack(map, yeGetByStr(mod, "player2 fleets"), "player 2");
  yeCreateInt(2, map, "cursor id");
  init = yeCreateArray(NULL, NULL);
  yeCreateString("battle", init, "name");
  yeCreateFunction("battleInit", ygGetManager("tcc"), init, "callback");
  ywidAddSubType(init);
  return NULL;
}

Entity *getLayer(Entity *contener, int idx)
{
  return yeGetByIdx(yeGetByStrFast(contener, "entries"), idx);
}

Entity *getCursorPos(Entity *wid)
{
  yeGetByStr(wid, "_cursos pos");
}

void addShip(Entity *wid)
{
  Entity *gc = yeCreateArray(NULL, NULL);
  Entity *l1 = getLayer(wid, 1);
  Entity *cursorPos = getCursorPos(wid);

  if (ywMapGetNbrEntityAt(l1, cursorPos, 3))
    ywMapPushNbr(l1, 1, cursorPos, NULL);
  yeDestroy(gc);
}

void *battleAction(int nbArgs, void **args)
{
  YWidgetState *tmpwid = args[0];
  Entity *wid = tmpwid->entity;
  Entity *l1 = getLayer(wid, 1);
  void *ret = (void *)NOTHANDLE;
  YEvent *events = args[1];
  YEvent *eve = events;

  YEVE_FOREACH(eve, events) {
    if (ywidEveType(eve) == YKEY_MOUSEDOWN) {
      printf("mouse: key %d at (%d - %d)\n", eve->key,
	     eve->xMouse, eve->yMouse);
    } else if (ywidEveType(eve) != YKEY_DOWN) {
      continue;
    }
    switch (ywidEveKey(eve)) {
    case 'q':
      ygCall(NULL, "FinishGame");
      ret = (void *)ACTION;
      break;
    case '\n':
      addShip(wid);
      break;
    case Y_UP_KEY:
      ywMapAdvenceWithPos(l1, yeGetByStr(wid, "_cursos pos"),
			  0, -1, yeGetByStr(wid, "cursor id"));
      ywContenerUpdate(l1);
      ret = (void *)ACTION;
      break;
    case Y_DOWN_KEY:
      ywMapAdvenceWithPos(l1, yeGetByStr(wid, "_cursos pos"),
			  0, 1, yeGetByStr(wid, "cursor id"));
      ywContenerUpdate(l1);
      ret = (void *)ACTION;
      break;
    case Y_LEFT_KEY:
      ywMapAdvenceWithPos(l1, yeGetByStr(wid, "_cursos pos"),
			  -1, 0, yeGetByStr(wid, "cursor id"));
      ywContenerUpdate(l1);
      ret = (void *)ACTION;
      break;
    case Y_RIGHT_KEY:
      ywMapAdvenceWithPos(l1, yeGetByStr(wid, "_cursos pos"),
			  1, 0, yeGetByStr(wid, "cursor id"));
      ywContenerUpdate(l1);
      ret = (void *)ACTION;
      break;
    default:
      break;
    }
  }
  return ret;
}

void *battleInit(int nbArgs, void **args)
{
  Entity *gc = yeCreateArray(NULL, NULL);
  Entity *entity = args[0];
  Entity *layers = yeCreateArray(entity, "entries");
  Entity *cur_layer;
  Entity *resources = yeGetByStrFast(entity, "resources");

  /* create map */
  Entity *tmp = ywidCreateFunction("battleAction",
				   ygGetManager("tcc"), entity, "action");

  yeCreateString("stacking", entity, "cnt-type");

  cur_layer = ywMapCreateDefaultEntity(layers, NULL,
				       resources,
				       0, 25, 25);
  yeReCreateString("rgba: 255 255 255 255", cur_layer, "background");
  yeCreateString("map", cur_layer, "<type>");


  cur_layer = ywMapCreateDefaultEntity(layers, NULL, resources, -1, 25, 25);
  yeCreateString("map", cur_layer, "<type>");
  ywMapDrawRect(cur_layer, ywPosCreateInts(0, 0, gc, NULL),
		ywPosCreateInts(25, 5, gc, NULL), 3);

  /* battle specific fields */
  yeCreateInt(0, entity, "_state");

  Entity *pos = ywPosCreateInts(0, 0, entity, "_cursos pos");
  ywMapPushElem(getLayer(entity, 1), yeGetByStr(entity, "cursor id"), pos, NULL);

  printf("%d - %p - %p\n",
	 yeGetInt(yeGetByStr(entity, "cursor id")),
	 yeGetByStr(entity, "player 1"),
	 yeGetByStr(entity, "player 2"));

  void *ret = ywidNewWidget(entity, "contener");
  yeDestroy(gc);
  return ret;
}
