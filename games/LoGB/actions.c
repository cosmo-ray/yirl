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

static Entity *getLayer(Entity *contener, int idx)
{
  return ywCntGetEntry(ywCntGetEntry(contener, 0), idx);
}

static Entity *getTextScreen(Entity *contener)
{
  return ywCntGetEntry(contener, 2);
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
    yeAddStr(txtSurface, "na, I don't thins so, go f**k yourself\n");
    goto exit;
  }

  first = printFLeet(txtSurface, fleet);
  if (!yeLen(fleet)) {
    yeAddStr(txtSurface, "This is the end, ");
    yeAddStr(txtSurface, "my only friend, the end\n");
    goto exit;
  }
  if (!_fleet)
    yeCreateArray(cp, "_fleet");

  ywMapPushNbr(l1, 1, cursorPos, NULL);
  yeMoveByEntity(fleet, _fleet, first);
  printFLeet(txtSurface, fleet);
 exit:
  ywTextScreenPosAtEndOfText(getTextScreen(wid));
  ywContenerUpdate(wid, getTextScreen(wid));
  yeDestroy(gc);
}

void *nextState(Entity *wid)
{
  if (yeLen(yeGetByStr(wid, "current_player.fleet"))) {
    yeAddStr(getTextScreenSurface(wid),
	     "can't next because there is still fleet left to pos\n");
  } else {
    yeAddStr(getTextScreenSurface(wid),
	     "fine, it's not implemented, that was useless\n");
  }
  ywContenerUpdate(wid, getTextScreen(wid));
  ywTextScreenPosAtEndOfText(getTextScreen(wid));
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
    Entity *mousePos;
    Entity *cursorPos = yeGetByStr(wid, "_cursos pos");

    if (ywidEveType(eve) == YKEY_MOUSEDOWN) {
      Entity *cur_wid;
      if ((cur_wid = ywContenerGetWidgetAt(wid, eve->xMouse, eve->yMouse)) ==
	  ywCntGetEntry(wid, 1)) {
	return (void *)ywMenuCallActionOnByEntity(cur_wid, eve,
						  ywMenuPosFromPix(cur_wid,
								   eve->xMouse,
								   eve->yMouse),
						  wid);
      } else if (ywContenerGetWidgetAt(wid, eve->xMouse, eve->yMouse) !=
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
	  getTextScreen(wid)) {
	yeAddInt(yeGetByStr(getTextScreen(wid), "text-threshold"), eve->key);
	ywContenerUpdate(wid, getTextScreen(wid));
      }
    } else if (ywidEveType(eve) != YKEY_DOWN) {
      continue;
    }
    switch (ywidEveKey(eve)) {
    case 'q':
      ygVCall(NULL, "FinishGame");
      ret = (void *)ACTION;
      break;
    case 'n':
      nextState(wid);
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

#define ADD_ACTIONS_CALLBACK(callbackName)		\
  void *callbackName##Callback(int nbArgs, void **args)	\
  {							\
    Entity *wid = args[2];				\
    callbackName(wid);					\
    return (void *)ACTION;				\
  }

ADD_ACTIONS_CALLBACK(addShip)
ADD_ACTIONS_CALLBACK(nextState)

#undef ADD_ACTIONS_CALLBACK
