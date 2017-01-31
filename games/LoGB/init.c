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
#include <yirl/game.h>
#include <yirl/menu.h>
#include <yirl/map.h>
#include <yirl/contener.h>
#include <yirl/text-screen.h>

static Entity *globMousePos;

void *init(int nbArgs, void **args)
{
  Entity *mod = args[0];
  Entity *init;
  Entity *map = yeCreateArray(mod, "battle");
  Entity *cp;

  yeCreateString("battle", map, "<type>");
  yePushBack(map, yeGetByStr(mod, "resources.battle-map"), "resources");
  cp = yeCreateArray(map, "player 1");
  yePushBack(cp, yeGetByStr(mod, "player1 fleets"), "fleet");
  cp = yeCreateArray(map, "player 2");
  yePushBack(map, yeGetByStr(mod, "player2 fleets"), "fleet");
  yeCreateInt(2, map, "cursor id");
  init = yeCreateArray(NULL, NULL);
  yeCreateString("battle", init, "name");
  yeCreateFunction("battleInit", ygGetManager("tcc"), init, "callback");
  ywidAddSubType(init);
  return NULL;
}

#include "actions.c"

void *battleInit(int nbArgs, void **args)
{
  Entity *gc = yeCreateArray(NULL, NULL);
  Entity *main = args[0];
  Entity *layers = yeCreateArray(main, "entries");
  Entity *cur_layer;
  Entity *resources = yeGetByStrFast(main, "resources");
  Entity *entity;
  Entity *textScreen;
  Entity *panel;

  globMousePos = ywPosCreateInts(0, 0, NULL, NULL);
  ywidCreateFunction("battleAction", ygGetManager("tcc"), main, "action");
  Entity *pos = ywPosCreateInts(0, 0, main, "_cursos pos");

  if (!yeGetByStr(main, "player 1") || !yeGetByStr(main, "player 2")) {
    DPRINT_ERR("unable to get players fleets");
    return NULL;
  }
  /* create maps */
  entity = yeCreateArray(layers, NULL);
  panel = yeCreateArray(layers, NULL);
  textScreen = yeCreateArray(layers, NULL);

  yeCreateInt(80, entity, "size");
  yeCreateInt(5, panel, "size");
  layers = yeCreateArray(entity, "entries");
  yeCreateString("contener", entity, "<type>");
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
  yePushBack(main, yeGetByStr(main, "player 1"), "current_player");

  ywMapPushElem(getLayer(main, 1), yeGetByStr(main, "cursor id"), pos, NULL);

  yeCreateString("text-screen", textScreen, "<type>");
  printFLeet(yeCreateString("", textScreen, "text"),
	     yeGetByStr(yeGetByStr(main, "current_player"), "fleet"));
  yeCreateString("menu", panel, "<type>");
  yeCreateString("panel", panel, "mn-type");
  layers = yeCreateArray(panel, "entries");
  cur_layer = yeCreateArray(layers, NULL);
  yeCreateString("add ship", cur_layer, "text");  
  cur_layer = yeCreateArray(layers, NULL);
  yeCreateString("remove ship", cur_layer, "text");
  cur_layer = yeCreateArray(layers, NULL);
  yeCreateString("next ship", cur_layer, "text");
  cur_layer = yeCreateArray(layers, NULL);
  yeCreateString("end positioning", cur_layer, "text");
  void *ret = ywidNewWidget(main, "contener");
  printf("io\n");
  yeDestroy(gc);
  return ret;
}
