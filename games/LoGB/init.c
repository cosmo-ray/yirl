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
#include <map.h>
#include <game.h>
#include <contener.h>
#include <text-screen.h>

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

static Entity *getLayer(Entity *contener, int idx)
{
  return ywCntGetEntry(ywCntGetEntry(contener, 0), idx);
}

static Entity *getTextScreen(Entity *contener)
{
  return ywCntGetEntry(contener, 1);
}

static Entity *getTextScreenSurface(Entity *contener)
{
  return (yeGetByStr(getTextScreen(contener), "text"));
}

static Entity *getCursorPos(Entity *wid)
{
  yeGetByStr(wid, "_cursos pos");
}

static Entity *printFLeet(Entity *text, Entity *fleet)
{
  Entity *first = NULL;

  yeStringAdd(text, "fleet left:\n");
  YE_ARRAY_FOREACH_EXT(fleet, elem, it) {
    if (!first)
      first = elem;
    yeAddInt(text, it.pos);
    yeStringAdd(text, ": ");
    yeStringAdd(text, yeGetString(yeGetByIdx(elem, 0)));
    yeStringAdd(text, " - ");
    yeAddEnt(text, yeGetByIdx(elem, 1));
    yeStringAddNl(text, "");
  }
  if (!first)
    yeStringAddNl(text, "NONE");
  return first;
}

static Entity *getCurrentPlayer(Entity *wid)
{
  return yeGetByStr(wid, "current_player");
}

static void addShip(Entity *wid)
{
  Entity *gc = yeCreateArray(NULL, NULL);
  Entity *l1 = getLayer(wid, 1);
  Entity *cursorPos = getCursorPos(wid);
  Entity *cp = getCurrentPlayer(wid);
  Entity *_fleet = yeGetByStr(cp, "_fleet");
  Entity *fleet = yeGetByStr(cp, "fleet");
  Entity *txtSurface = getTextScreenSurface(wid);
  Entity *first;

  if (!ywMapGetNbrEntityAt(l1, cursorPos, 3)) {
    yeStringAdd(txtSurface, "na, I don't thins so, go f**k yourself\n");
    return;
  }

  first = printFLeet(txtSurface, fleet);
  if (!yeLen(fleet)) {
    yeAddStr(txtSurface, "This is the end, ");
    yeAddStr(txtSurface, "my only friend, the end\n");
    return;
  }
  if (!_fleet)
    yeCreateArray(cp, "_fleet");

  ywMapPushNbr(l1, 1, cursorPos, NULL);
  yeMoveByEntity(fleet, _fleet, first);
  printFLeet(txtSurface, fleet);
  ywTextScreenPosAtEndOfText(getTextScreen(wid));
  ywContenerUpdate(wid, getTextScreen(wid));
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

  /* ywContenerUpdate(wid, yeGetByIdx(yeGetByStrFast(wid, "entries"), 0)); */
  YEVE_FOREACH(eve, events) {
    Entity *mousePos;
    Entity *cursorPos = yeGetByStr(wid, "_cursos pos");

    if (ywidEveType(eve) == YKEY_MOUSEDOWN) {
      if (ywContenerGetWidgetAt(wid, eve->xMouse, eve->yMouse) !=
	  ywCntGetEntry(wid, 0)) {
	return ret;
      }
      mousePos = ywMapPosFromPixs(l1, eve->xMouse, eve->yMouse, NULL, NULL);
      ywMapMoveByEntity(l1, cursorPos, mousePos,
			yeGetByStrFast(wid, "cursor id"));
      ywPosSetEnt(cursorPos, mousePos, 0);
      YE_DESTROY(mousePos);
      ywContenerUpdate(wid, l1);
      ret = (void *)ACTION;
    } else if (ywidEveType(eve) == YKEY_MOUSEMOTION) {
      ywPosSetInts(globMousePos, eve->xMouse, eve->yMouse);
    } else if (ywidEveType(eve) == YKEY_MOUSEWHEEL) {
      if (ywContenerGetWidgetAt(wid, ywPosX(globMousePos), ywPosY(globMousePos)) ==
	  ywCntGetEntry(wid, 1)) {
	yeAddInt(yeGetByStr(getTextScreen(wid), "text-threshold"), eve->key);
	ywContenerUpdate(wid, getTextScreen(wid));
      }
    } else if (ywidEveType(eve) != YKEY_DOWN) {
      continue;
    }
    switch (ywidEveKey(eve)) {
    case 'q':
	    ygCall(NULL, "FinishGame", NULL);
      ret = (void *)ACTION;
      break;
    case 'n':
      if (yeLen(yeGetByStr(wid, "current_player.fleet"))) {
	    yeAddStr(getTextScreenSurface(wid),
		     "can't next because there is still fleet left to pos\n");
      } else {
	yeAddStr(getTextScreenSurface(wid),
		 "fine, it's not implemented, that was useless\n");
      }
      ywContenerUpdate(wid, getTextScreen(wid));
      ywTextScreenPosAtEndOfText(getTextScreen(wid));
      break;
    case '\n':
      addShip(wid);
      break;
    case Y_UP_KEY:
      ywMapAdvenceWithPos(l1, yeGetByStr(wid, "_cursos pos"),
			  0, -1, yeGetByStr(wid, "cursor id"));
      ywContenerUpdate(wid, l1);
      ret = (void *)ACTION;
      break;
    case Y_DOWN_KEY:
      ywMapAdvenceWithPos(l1, yeGetByStr(wid, "_cursos pos"),
			  0, 1, yeGetByStr(wid, "cursor id"));
      ywContenerUpdate(wid, l1);
      ret = (void *)ACTION;
      break;
    case Y_LEFT_KEY:
      ywMapAdvenceWithPos(l1, yeGetByStr(wid, "_cursos pos"),
			  -1, 0, yeGetByStr(wid, "cursor id"));
      ywContenerUpdate(wid, l1);
      ret = (void *)ACTION;
      break;
    case Y_RIGHT_KEY:
      ywMapAdvenceWithPos(l1, yeGetByStr(wid, "_cursos pos"),
			  1, 0, yeGetByStr(wid, "cursor id"));
      ywContenerUpdate(wid, l1);
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
  Entity *main = args[0];
  Entity *layers = yeCreateArray(main, "entries");
  Entity *cur_layer;
  Entity *resources = yeGetByStrFast(main, "resources");
  Entity *entity;
  Entity *textScreen;

  globMousePos = ywPosCreateInts(0, 0, NULL, NULL);
  ywidCreateFunction("battleAction", ygGetManager("tcc"), main, "action");
  Entity *pos = ywPosCreateInts(0, 0, main, "_cursos pos");

  if (!yeGetByStr(main, "player 1") || !yeGetByStr(main, "player 2")) {
    DPRINT_ERR("unable to get players fleets");
    return NULL;
  }
  /* create maps */
  entity = yeCreateArray(layers, NULL);
  textScreen = yeCreateArray(layers, NULL);

  yeCreateInt(80, entity, "size");
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
  printf("%d - %p - %p\n",
	 yeGetInt(yeGetByStr(main, "cursor id")),
	 yeGetByStr(main, "player 1"),
	 yeGetByStr(main, "player 2"));

  void *ret = ywidNewWidget(main, "contener");
  yeDestroy(gc);
  return ret;
}
