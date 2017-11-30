/*
**Copyright (C) 2013 Matthias Gatto
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

#include	<lauxlib.h>
#include	<stdlib.h>
#include	"lua-binding.h"
#include	"debug.h"
#include	"entity.h"
#include	"entity-script.h"
#include	"map.h"
#include	"container.h"
#include	"canvas.h"
#include	"game.h"

int	luaYAnd(lua_State *L)
{
  lua_pushnumber(L, (int)lua_tonumber(L, 1) & (int)lua_tonumber(L, 2));
  return (1);
}

int	luaGet(lua_State *L)
{
  if (lua_gettop(L) != 2 || !lua_isuserdata(L, 1)) {
    goto error;
  } else if (lua_isnumber(L, 2)) {
    lua_pushlightuserdata(L, yeGetByIdx(lua_touserdata(L, 1),
				       lua_tonumber (L, 2)));
    return 1;
  } else if (lua_isstring(L, 2)) {
    lua_pushlightuserdata(L, yeGetByStrFast(lua_touserdata(L, 1),
					lua_tostring (L, 2)));
    return 1;
  }
 error:
  luaL_error(L, "function arguments for yeGet are incorect");
  return -1;
}

int	luaLen(lua_State *L)
{
  DPRINT_INFO("luaGetArrayMenber\n");
  if (lua_gettop(L) != 1 || !lua_islightuserdata(L, 1))
    {
      luaL_error(L, "argument are incorrect\n");
      return -1;
    }
  lua_pushnumber(L, yeLen((Entity *)lua_topointer(L, 1)));
  return 1;
}

int	luaCopy(lua_State *L)
{
  DPRINT_INFO("enter luaCopyEntity\n");
  if (lua_gettop(L) != 2 || !lua_islightuserdata(L, 1) ||
      !lua_islightuserdata(L, 2))
    {
      luaL_error(L, "function arguments are incorect\n"
		 "real prototyre is: yeCopy(lightuserdata src, lightuserdata dest)\n");
      return -1;
    }
  lua_pushlightuserdata(L, yeCopy((Entity *)lua_topointer(L, 1),
				  (Entity *)lua_topointer(L, 2)));
  return 1;
}

int	luaYwCanvasMoveObjByIdx(lua_State *L)
{
  lua_pushnumber(L, ywCanvasMoveObjByIdx(lua_touserdata(L, 1),
					 lua_tonumber(L, 2),
					 lua_touserdata(L, 3)));
  return 1;
}

int	luaYwCanvasObjSetResourceId(lua_State *L)
{
  ywCanvasObjSetResourceId(lua_touserdata(L, 1), lua_tonumber(L, 2));
  return 0;
}

int	luaYwCanvasNewObj(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasNewObj(lua_touserdata(L, 1),
					  lua_tonumber(L, 2),
					  lua_tonumber(L, 3),
					  lua_tonumber(L, 4)));
  return 1;
}

int	luaYwCanvasObjSize(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasObjSize(lua_touserdata(L, 1),
					   lua_touserdata(L, 2)));
  return 1;
}

int	luaYwCanvasObjPos(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasObjPos(lua_touserdata(L, 1)));
  return 1;
}

int luaYwCanvasObjFromIdx(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasObjFromIdx(lua_touserdata(L, 1),
					      lua_tonumber(L, 2)));
  return 1;
}

int luaYwCanvasIdxFromObj(lua_State *L)
{
  lua_pushnumber(L, ywCanvasIdxFromObj(lua_touserdata(L, 1),
				       lua_touserdata(L, 2)));
  return 1;
}

int luaYwCanvasObjSetPos(lua_State *L)
{
  ywCanvasObjSetPos(lua_touserdata(L, 1), lua_tonumber(L, 2),
		    lua_tonumber(L, 3));
  return 0;
}

int	luaYwCanvasRemoveObj(lua_State *L)
{
  ywCanvasRemoveObj(lua_touserdata(L, 1), lua_touserdata(L, 2));
  return 0;
}

int luaYwCanvasNewColisionsArray(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasNewColisionsArray(lua_touserdata(L, 1),
						     lua_touserdata(L, 2)));
  return 1;
}

int luaYwCanvasNewColisionsArrayWithRectangle(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasNewColisionsArray(lua_touserdata(L, 1),
						     lua_touserdata(L, 2)));
  return 1;
}

int luaYwCanvasNewText(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasNewText(lua_touserdata(L, 1), lua_tonumber(L, 2),
					   lua_tonumber(L, 3), lua_touserdata(L, 4)));
  return 1;
}

int	luaYeIncrRef(lua_State *L)
{
  yeIncrRef(YE_TO_ENTITY(lua_touserdata(L, 1)));
  return 0;
}

int	luaYeToLuaString(lua_State *L)
{
  char *str = yeToCStr(lua_touserdata(L, 1), -1, 0);

  lua_pushstring(L, str);
  free(str);
  return 1;
}

int	luaSetMainWid(lua_State *L)
{
  ywidSetMainWid(lua_touserdata(L, 1));
  return 0;
}

int	luaAddSubType(lua_State *L)
{
  ywidAddSubType(lua_touserdata(L, 1));
  return 0;
}


int	luaNewWidget(lua_State *L)
{
  lua_pushlightuserdata(L, ywidNewWidget((Entity *)lua_touserdata(L, 1),
					 lua_tostring(L, 2)));
  return 1;
}

int	luaCreateArray(lua_State *L)
{
  lua_pushlightuserdata(L, yeCreateArray((Entity *)lua_topointer(L, 1),
					 lua_tostring(L, 2)));
  return 1;
}

int	luaCreateString(lua_State *L)
{
  if (!lua_isstring(L, 1)) {
    luaL_error(L, "missing string");
    return -1;
  }
  Entity *ret = yeCreateString(lua_tostring(L, 1),
			       (Entity *)lua_topointer(L, 2),
			       lua_tostring(L, 3));
  lua_pushlightuserdata(L, ret);
  return 1;
}

int	luaCreateInt(lua_State *L)
{
  if (!lua_isnumber(L, 1)) {
    luaL_error(L, "missing string");
    return -1;
  }
  lua_pushlightuserdata(L, yeCreateInt((int)lua_tonumber(L, 1),
				       (Entity *)lua_topointer(L, 2),
				       lua_tostring(L, 3)));
  return 1;
}

int	luaCreateFloat(lua_State *L)
{
  if (!lua_isnumber(L, 1)) {
    luaL_error(L, "missing string");
    return -1;
  }
  lua_pushlightuserdata(L, yeCreateFloat(lua_tonumber(L, 1),
					 (Entity *)lua_topointer(L, 2),
					 lua_tostring(L, 3)));
  return 1;
}

int	luaySoundLoad(lua_State *L)
{
  lua_pushnumber(L, sound_load(lua_tostring(L, 1)));
  return 1;
}

int	luaySoundPlayLoop(lua_State *L)
{
  lua_pushnumber(L, sound_play_loop(lua_tonumber(L, 1)));
  return 1;
}

int	luaySoundPlay(lua_State *L)
{
  lua_pushnumber(L, sound_play(lua_tonumber(L, 1)));
  return 1;
}

int	luaySoundStop(lua_State *L)
{
  lua_pushnumber(L, sound_stop(lua_tonumber(L, 1)));
  return 1;
}

/**
 * This is not a strict binding of the original yeCreateFunction,
 * because the original has to handle managers,
 * In this functions we use the lua manager inside game.c by calling
 * ygGetLuaManager()
 */
int     luaCreateFunction(lua_State *L)
{
  void *ret;

  if (!lua_tostring(L, 3)) {
    ret = yeCreateFunctionSimple(lua_tostring(L, 1),
				 ygGetLuaManager(), lua_touserdata(L, 2));
  } else {
    ret = yeCreateFunction(lua_tostring(L, 1),
			   ygGetLuaManager(), lua_touserdata(L, 2),
			   lua_tostring(L, 3));
  }

  lua_pushlightuserdata(L, ret);
  return 1;
}

int	luaWidNextEve(lua_State *L)
{
  lua_pushlightuserdata(L, ywidNextEve(lua_touserdata(L, 1)));
  return 1;
}

int	luaWidNext(lua_State *L)
{
  lua_pushnumber(L, ywidNext(lua_touserdata(L, 1)));
  return 1;
}

int	luaWidEveIsEnd(lua_State *L)
{
  lua_pushboolean(L, lua_touserdata(L, 1) == NULL);
  return 1;
}

int	luaEveType(lua_State *L)
{
  lua_pushnumber(L, ywidEveType(lua_touserdata(L, 1)));
  return 1;
}

int	luaEveKey(lua_State *L)
{
  lua_pushnumber(L, ywidEveKey(lua_touserdata(L, 1)));
  return 1;
}

/* TODO: Add luaEveMouseX() */
/* TODO: Add luaEveMouseY() */

int	luaPopBack(lua_State *L)
{
  DPRINT_INFO("enter luaArrayPopBack\n");
  if (lua_gettop(L) != 1 || !lua_islightuserdata(L, 1))
    {
      luaL_error(L, "function arguments are incorect\n"
	     "real prototyre is: arrayPopBack(lightuserdata entity)\n");
      return -1;
    }

  lua_pushlightuserdata(L, yePopBack(((Entity *)lua_topointer(L, 1))));
  return 1;
}

int	luaPushBack(lua_State *L)
{
  if (lua_gettop(L) < 2)
    {
      luaL_error(L, "function arguments are incorect\n"
	     "real prototyre is: yePushBack(lightuserdata entity, lightuserdata toPush, string name)\n");
      return -1;
    }
  if (lua_isstring(L, 3)) {
    yePushBack(((Entity *)lua_topointer(L, 1)), (Entity *)lua_topointer(L, 2),
	       lua_tostring(L, 3));
  } else {
    yePushBack(((Entity *)lua_topointer(L, 1)), (Entity *)lua_topointer(L, 2),
	       NULL);
  }
  return 0;
}

int	luaSetString(lua_State *L)
{
  yeSetString(lua_touserdata(L, 1), lua_tostring(L, 2));
  return 0;
}

int	luaGetString(lua_State *L)
{
  if (lua_gettop(L) != 1 || !lua_islightuserdata(L, 1))
    {
      luaL_error(L, "function arguments are incorect\n"
		 "real prototyre is: yeGetString(lightuserdata entity)\n");
      return -1;
    }
  lua_pushstring(L, yeGetString(YE_TO_ENTITY(lua_topointer(L, 1))));
  return 1;
}

int	luaGetInt(lua_State *L)
{
  if (lua_gettop(L) != 1 || !lua_islightuserdata(L, 1))
    {
      luaL_error(L, "function arguments are incorect\n"
	     "real prototyre is: yeGetInt(lightuserdata entity)\n");
      return -1;
    }
  lua_pushnumber(L, (yeGetInt(((Entity *)lua_topointer(L, 1)))));
  return 1;
}

int	luaGetFloat(lua_State *L)
{
  if (lua_gettop(L) != 1 || !lua_islightuserdata(L, 1))
    {
      luaL_error(L, "function arguments are incorect\n"
	     "real prototyre is: yeGetFloat(lightuserdata entity)\n");
      return -1;
    }
  lua_pushnumber(L, (yeGetFloat(((Entity *)lua_topointer(L, 1)))));
  return 1;
}

int	luaSetFunction(lua_State *L)
{
  DPRINT_INFO("enter luaSetFunction\n");
  if (lua_gettop(L) != 2)
    {
      luaL_error(L, "function arguments are incorect\n"
	     "real prototyre is: yeSetFunction(lightuserdata entity, string functionName)\n");
      return -1;
    }
  yeSetFunction(YE_TO_ENTITY(lua_topointer(L, 1)), lua_tostring(L, 2));
  return 0;
}

int	luaSetInt(lua_State *L)
{
  DPRINT_INFO("luaSetInt\n");
  if (lua_gettop(L) != 2 || !lua_islightuserdata (L, 1) || !lua_isnumber (L, 2))
    {
     // luaL_error(L, "function arguments are incorect\n""real prototyre is: setEntityIntue(...)\n");

      return -1;
    }
  yeSetInt(((Entity *)lua_topointer(L, 1)), lua_tonumber(L, 2));
  return 0;
}

int	luaSetFloat(lua_State *L)
{
  if (lua_gettop(L) != 2 || !lua_islightuserdata (L, 1) || !lua_isnumber (L, 2))
    {
      luaL_error(L, "function arguments are incorect\n"
	     "real prototyre is: setEntityIntue(...)\n");
      return (-1);
    }
  yeSetFloat(((Entity *)lua_topointer(L, 1)), lua_tonumber(L, 2));
  return (0);
}

int	luaDestroy(lua_State *L)
{
  yeDestroy(lua_touserdata(L, 1));
  return 0;
}

int	luaRemoveChild(lua_State *L)
{
  if (lua_gettop(L) != 2)
    {
      luaL_error(L, "function arguments are incorect\n"
	     "real prototyre is: arrayRemove(lightuserdata array, lightuserdata toRemove, int deepSearch)\n");
      return (-1);
    }
  lua_pushlightuserdata(L, yeRemoveChild(YE_TO_ENTITY(lua_topointer(L, 1)),
					 YE_TO_ENTITY(lua_topointer(L, 2))));
  return (1);
}


int	luaUnsetFunction(lua_State *L)
{
  DPRINT_INFO("enter luaUnsetFunction\n");
  if (lua_gettop(L) != 1)
    {
      luaL_error(L, "function arguments are incorect\n"
	     "real prototyre is: unsetFunction(lightuserdata entity)\n");
      return (-1);
    }
  yeUnsetFunction(YE_TO_ENTITY(lua_topointer(L, 1)));
  return (0);
}

int	luaType(lua_State *L)
{
  if (lua_gettop(L) != 1 || !lua_islightuserdata (L, 1)) {
    luaL_error(L, "error in yeType: missing entity\n");
    return -1;
  }
  lua_pushnumber(L, yeType(YE_TO_ENTITY(lua_topointer(L, 1))));
  return 1;
}

int	luaYwMapGetIdByElem(lua_State *L)
{
  lua_pushnumber(L, ywMapGetIdByElem(lua_touserdata(L, 1)));
  return 1;
}

int	luaYwMapGetResourceId(lua_State *L)
{
  lua_pushnumber(L, ywMapGetResourceId(lua_touserdata(L, 1),
				       lua_touserdata(L, 2)));
  return 1;
}

int	luaYwMapGetResource(lua_State *L)
{
  lua_pushlightuserdata(L, ywMapGetResource(lua_touserdata(L, 1),
					    lua_touserdata(L, 2)));
  return 1;
}

int	luaYwMapGetCase(lua_State *L)
{
  lua_pushlightuserdata(L, ywMapGetCase(lua_touserdata(L, 1),
					lua_touserdata(L, 2)));
  return 1;
}

int	luaYwMapGetEntityById(lua_State *L)
{
  lua_pushlightuserdata(L, ywMapGetEntityById(lua_touserdata(L, 1),
					      lua_touserdata(L, 2),
					      lua_tonumber(L, 3)));
  return 1;
}

int	luaYMapAdvence(lua_State *L)
{
  if (lua_isnumber(L, 3)) {
    lua_pushnumber(L, ywMapAdvenceWithPos(lua_touserdata(L, 1),
					  lua_touserdata(L, 2),
					  lua_tonumber(L, 3),
					  lua_tonumber(L, 4),
					  lua_touserdata(L, 5)));
  } else {
    lua_pushnumber(L, ywMapAdvenceWithEPos(lua_touserdata(L, 1),
					   lua_touserdata(L, 2),
					   lua_touserdata(L, 3),
					   lua_touserdata(L, 4)));
  }
  return 1;
}

int	luaYwReplaceEntry(lua_State *L)
{
  lua_pushnumber(L, ywReplaceEntry(lua_touserdata(L, 1),
				   lua_tonumber(L, 2),
				   lua_touserdata(L, 3))
		 );
  return 1;
}

int	luaYeSwapElems(lua_State *L)
{
  lua_pushnumber(L, yeSwapElems(lua_touserdata(L, 1),
				lua_touserdata(L, 2),
				lua_touserdata(L, 3))
		 );
  return 1;
}

int	luaYwCntGetEntry(lua_State *L)
{
  lua_pushlightuserdata(L, ywCntGetEntry(lua_touserdata(L, 1),
					 lua_tonumber(L, 2)));
  return 1;
}


int	luaYwCntPopLastEntry(lua_State *L)
{
  lua_pushlightuserdata(L, ywCntPopLastEntry(lua_touserdata(L, 1)));
  return 1;
}

int	luaYwPushNewWidget(lua_State *L)
{
  lua_pushnumber(L, ywPushNewWidget(lua_touserdata(L, 1),
				    lua_touserdata(L, 2),
				    lua_tonumber(L, 3)));
  return 1;
}

int	luaYuiAbs(lua_State *L)
{
  lua_pushnumber(L, yuiAbs(lua_tonumber(L, 1)));
  return 1;
}

int	luaywPosX(lua_State *L)
{
  lua_pushnumber(L, ywPosX(lua_touserdata(L, 1)));
  return 1;
}

int	luaywPosY(lua_State *L)
{
  lua_pushnumber(L, ywPosY(lua_touserdata(L, 1)));
  return 1;
}

int	luaYwRectCreate(lua_State *L)
{
  if (lua_isnumber(L, 1)) {
    lua_pushlightuserdata(L, ywRectCreateInts(lua_tonumber(L, 1),
					      lua_tonumber(L, 2),
					      lua_tonumber(L, 3),
					      lua_tonumber(L, 4),
					      lua_touserdata(L, 5),
					      lua_tostring(L, 6)));
  } else if (lua_gettop(L) == 3) {
    lua_pushlightuserdata(L, ywRectCreateEnt(lua_touserdata(L, 1),
					     lua_touserdata(L, 2),
					     lua_tostring(L, 3)));
  } else {
    lua_pushlightuserdata(L, ywRectCreatePosSize(lua_touserdata(L, 1),
						 lua_touserdata(L, 2),
						 lua_touserdata(L, 3),
						 lua_tostring(L, 4)));
  }
  return 1;
}

int	luaywPosCreate(lua_State *L)
{
  if (lua_isnumber(L, 1)) {
    lua_pushlightuserdata(L, ywPosCreate(lua_tonumber(L, 1),
					 lua_tonumber(L, 2),
					 lua_touserdata(L, 3),
					 lua_tostring(L, 4)));
  } else {
    lua_pushlightuserdata(L, ywPosCreate(lua_touserdata(L, 1),
					 0, lua_touserdata(L, 2),
					 lua_tostring(L, 3)));
  }
  return 1;
}

int	luaywPosPrint(lua_State *L)
{
  ywPosPrint(lua_touserdata(L, 1));
  return 0;
}

int	luaywPosToString(lua_State *L)
{
  lua_pushstring(L, ywPosToString(lua_touserdata(L, 1)));
  return 1;
}

int	luaywPosSet(lua_State *L)
{
  if (lua_isnumber(L, 2)) {
    lua_pushlightuserdata(L, ywPosSet(lua_touserdata(L, 1), lua_tonumber(L, 2),
				      lua_tonumber(L, 3)));
  } else {
    lua_pushlightuserdata(L, ywPosSet(lua_touserdata(L, 1),
				      lua_touserdata(L, 2), 0));
  }
  return 1;
}

int	luaywPosIsSame(lua_State *L)
{
  if (lua_isnumber(L, 2))
    lua_pushboolean(L, ywPosIsSame(lua_touserdata(L, 1),
				      lua_tonumber(L, 2), lua_tonumber(L, 3)));
  else
    lua_pushboolean(L, ywPosIsSame(lua_touserdata(L, 1),
				      lua_touserdata(L, 2), 0));
  return 1;
}

int	luaywPosIsSameX(lua_State *L)
{
  if (lua_isnumber(L, 2))
    lua_pushboolean(L, ywPosIsSameX(lua_touserdata(L, 1),
				       lua_tonumber(L, 2)));
  else
    lua_pushboolean(L, ywPosIsSameX(lua_touserdata(L, 1),
				       lua_touserdata(L, 2)));
  return 1;
}

int	luaywPosIsSameY(lua_State *L)
{
  if (lua_isnumber(L, 2))
    lua_pushboolean(L, ywPosIsSameY(lua_touserdata(L, 1),
				       lua_tonumber(L, 2)));
  else
    lua_pushboolean(L, ywPosIsSameY(lua_touserdata(L, 1),
				       lua_touserdata(L, 2)));
  return 1;
}

int	luaywPosAdd(lua_State *L)
{
  if (lua_isnumber(L, 2)) {
    lua_pushlightuserdata(L, ywPosAddXY(lua_touserdata(L, 1),
					lua_tonumber(L, 2),
					lua_tonumber(L, 3)));
  } else {
    lua_pushlightuserdata(L, ywPosAdd(lua_touserdata(L, 1),
				      lua_touserdata(L, 2)));
  }
  return 1;
}

int	luaYwMapMove(lua_State *L)
{
  if (lua_isstring(L, 4))
    lua_pushnumber(L, ywMapMoveByStr(lua_touserdata(L, 1),
				     lua_touserdata(L, 2),
				     lua_touserdata(L, 3),
				     lua_tostring(L, 4)));
  else
    lua_pushnumber(L, ywMapMoveByEntity(lua_touserdata(L, 1),
					lua_touserdata(L, 2),
					lua_touserdata(L, 3),
					lua_touserdata(L, 4)));
  return 1;
}

int	luaYwMapSmootMove(lua_State *L)
{
  lua_pushnumber(L, ywMapSmootMove(lua_touserdata(L, 1),
				   lua_touserdata(L, 2),
				   lua_touserdata(L, 3),
				   lua_touserdata(L, 4)));
  return 1;
}

int	luaYwMapIntFromPos(lua_State *L)
{
  lua_pushnumber(L, ywMapIntFromPos(lua_touserdata(L, 1),
				    lua_touserdata(L, 2)));
  return 1;
}

int	luaYwMapPosFromInt(lua_State *L)
{
  lua_pushlightuserdata(L, ywMapPosFromInt(lua_touserdata(L, 1),
					   lua_tonumber(L, 2),
					   lua_touserdata(L, 3),
					   lua_tostring(L, 4)));
  return 1;
}

int	luaYwMapRemove(lua_State *L)
{
  if (lua_isstring(L, 3))
    ywMapRemove(lua_touserdata(L, 1), lua_touserdata(L, 2), lua_tostring(L, 3));
  else
    ywMapRemove(lua_touserdata(L, 1), lua_touserdata(L, 2),
		YE_TO_ENTITY(lua_touserdata(L, 3)));
  return 0;
}

int	luaYwMapW(lua_State *L)
{
  lua_pushnumber(L, ywMapW(lua_touserdata(L, 1)));
  return 1;
}

int	luaYwMapH(lua_State *L)
{
  lua_pushnumber(L, ywMapH(lua_touserdata(L, 1)));
  return 1;
}

int	luaYwMapSetSmootMovement(lua_State *L)
{
  ywMapSetSmootMovement(lua_touserdata(L, 1), lua_tonumber(L, 2));
  return 0;
}

int	luaMvTablePush(lua_State *L)
{
  lua_pushlightuserdata(L, ywMapMvTablePush(lua_touserdata(L, 1),
					    lua_touserdata(L, 2),
					    lua_touserdata(L, 3),
					    lua_touserdata(L, 4),
					    lua_touserdata(L, 5)));
  return 1;
}

int	luaYwMapPushElem(lua_State *L)
{
  lua_pushlightuserdata(L,
			ywMapPushElem(lua_touserdata(L, 1), lua_touserdata(L, 2),
				      lua_touserdata(L, 3), lua_tostring(L, 4))
			);
  return 1;
}

int	luaYwMapPushNbr(lua_State *L)
{
  lua_pushlightuserdata(L,
			ywMapPushNbr(lua_touserdata(L, 1), lua_tonumber(L, 2),
				     lua_touserdata(L, 3), lua_tostring(L, 4))
			);
  return 1;
}

int	luaRand(lua_State *L)
{
  lua_pushnumber(L, yuiRand());
  return 1;
}

int	luaRandInit(lua_State *L)
{
  (void)L;
  yuiRandInit();
  return 0;
}

int	luaGetMod(lua_State *L)
{
  lua_pushlightuserdata(L, ygGetMod(lua_tostring(L, 1)));
  return 1;
}

#define LUA_YG_CALL(args...)					\
  lua_pushlightuserdata(L, ygCall(lua_tostring(L, 1),		\
				  lua_tostring(L, 2), args))

int	luaGCall(lua_State *L)
{

  switch (lua_gettop(L)) {
  case 2:
    lua_pushlightuserdata(L, ygCall(lua_tostring(L, 1), lua_tostring(L, 2)));
    return 1;
  case 3:
    LUA_YG_CALL(lua_touserdata(L, 3));
    return 1;
  case 4:
    LUA_YG_CALL(lua_touserdata(L, 3), lua_touserdata(L, 4));
    return 1;
  case 5:
    LUA_YG_CALL(lua_touserdata(L, 3),
		lua_touserdata(L, 4), lua_touserdata(L, 5));
    return 1;
  case 6:
    LUA_YG_CALL(lua_touserdata(L, 3), lua_touserdata(L, 4),
		lua_touserdata(L, 5), lua_touserdata(L, 6));
    return 1;
  default:
    return -1;
  }

  return -1;
}

#define LUA_YES_CALL(args...)					\
  lua_pushlightuserdata(L, yesCall(lua_touserdata(L, 1), args))

int	luaYesCall(lua_State *L)
{

  switch (lua_gettop(L)) {
  case 1:
    lua_pushlightuserdata(L, yesCall(lua_touserdata(L, 1)));
    return 1;
  case 2:
    LUA_YES_CALL(lua_touserdata(L, 2));
    return 1;
  case 3:
    LUA_YES_CALL(lua_touserdata(L, 2), lua_touserdata(L, 3));
    return 1;
  case 4:
    LUA_YES_CALL(lua_touserdata(L, 2),
		 lua_touserdata(L, 3), lua_touserdata(L, 4));
    return 1;
  case 5:
    LUA_YES_CALL(lua_touserdata(L, 2), lua_touserdata(L, 3),
		 lua_touserdata(L, 4), lua_touserdata(L, 5));
    return 1;
  case 6:
    LUA_YES_CALL(lua_touserdata(L, 2), lua_touserdata(L, 3),
		 lua_touserdata(L, 4), lua_touserdata(L, 5),
		 lua_touserdata(L, 6));
    return 1;
  case 7:
    LUA_YES_CALL(lua_touserdata(L, 2), lua_touserdata(L, 3),
		 lua_touserdata(L, 4), lua_touserdata(L, 5),
		 lua_touserdata(L, 6), lua_touserdata(L, 7));
    return 1;
  default:
    DPRINT_ERR("internal error: too much argument");
    return -1;
  }

  return -1;
}

#undef LUA_YES_CALL
#undef LUA_YG_CALL

int	luaYGet(lua_State *L)
{
  if (lua_isstring(L, 1))
    lua_pushlightuserdata(L, ygGet(lua_tostring(L, 1)));
  else if (lua_islightuserdata(L, 1))
    lua_pushlightuserdata(L, ygGet(yeGetString(lua_touserdata(L, 1))));
  return 1;
}

int	luaYgRegistreFunc(lua_State *L)
{
  return ygRegistreFuncInternal(ygGetLuaManager(), lua_tonumber(L, 1),
				lua_tostring(L, 2), lua_tostring(L, 3));
}

int	luaYgFileToEnt(lua_State *L)
{
  lua_pushlightuserdata(L, ygFileToEnt(lua_tonumber(L, 1),
				       lua_tostring(L, 2),
				       lua_touserdata(L, 3)));
  return 1;
}

int	luaYgEntToFile(lua_State *L)
{
  lua_pushnumber(L, ygEntToFile(lua_tonumber(L, 1),
				lua_tostring(L, 2),
				lua_touserdata(L, 3)));
  return 1;
}

int	luaSetAt(lua_State *L)
{
  Entity *ent = NULL;
  if (lua_gettop(L) != 3)
    return -1;

  if (lua_isnumber(L, 2)) {
    ent = yeGet(lua_touserdata(L, 1), (int64_t)lua_tonumber(L, 2));
  } else if (lua_isstring(L, 2)) {
    ent = yeGet(lua_touserdata(L, 1), lua_tostring(L, 2));
  } else {
    return -1;
  }

  if (lua_isnumber(L, 3)) {
    yeSetInt(ent, lua_tonumber(L, 3));
  } else if (lua_isstring(L, 3)) {
    yeSetString(ent, lua_tostring(L, 3));
  }
  return 0;
}

int	luaYeReplace(lua_State *L)
{
  if (lua_gettop(L) != 3)
    return -1;
  lua_pushnumber(L, yeReplace(lua_touserdata(L, 1),
			      lua_touserdata(L, 2),
			      lua_touserdata(L, 3)));
  return 1;
}


int	luaYeReplaceAtIdx(lua_State *L)
{
  lua_pushlightuserdata(L, yeReplaceAtIdx(lua_touserdata(L, 1),
					  lua_touserdata(L, 2),
					  lua_tonumber(L, 3)));
  return 1;
}

int	luaYeReplaceBack(lua_State *L)
{
  lua_pushlightuserdata(L, yeReplaceBack(lua_touserdata(L, 1),
					 lua_touserdata(L, 2),
					 lua_tostring(L, 3)));
  return 1;
}
