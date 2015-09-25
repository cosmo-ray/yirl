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

#include	"lua-binding.h"
#include	"debug.h"
#include	"entity.h"

int	luaYAnd(lua_State *L)
{
  lua_pushnumber(L, (int)lua_tonumber(L, 1) & (int)lua_tonumber(L, 2));
  return (1);
}

/* int	luaGetObject(lua_State *L) */
/* { */
/*   if (lua_gettop(L) != 2 || !lua_isstring (L, 1) || !lua_isstring (L, 2)) */
/*     { */
/*       DPRINT_ERR("function arguments are incorect\n" */
/* 	     "real prototyre is: getEntity(string objType, string objName)"); */
/*       return (-1); */
/*     } */
/*   lua_pushlightuserdata(L, getObject(lua_tostring (L, 1), lua_tostring (L, 2))); */
/*   return (1); */
/* } */

/* int	luaGetEntityByName(lua_State *L) */
/* { */
/*   if (lua_gettop(L) != 1 || !lua_isstring (L, 1)) */
/*     { */
/*       DPRINT_ERR("function arguments are incorect\n" */
/* 	     "real prototyre is: getEntityByName(string objName)"); */
/*       return -1; */
/*     } */
/*   lua_pushlightuserdata(L, getEntityByName(lua_tostring (L, 1))); */
/*   return 1; */
/* } */


int	luaGet(lua_State *L)
{
  if (lua_gettop(L) != 2 || !lua_islightuserdata(L, 1)) {
    goto error;
  }
  if (lua_isstring(L, 2)) {
    lua_pushlightuserdata(L, yeGetByStr(((Entity *)lua_topointer(L, 1)),
					lua_tostring (L, 2)));
    return 1;
  } else if (lua_isnumber(L, 2)) {
    lua_pushlightuserdata(L, yeGetByIdx(((Entity *)lua_topointer(L, 1)),
				       lua_tonumber (L, 2)));
    return 1;
  }
 error:
  DPRINT_ERR("function arguments for yeGet are incorect\n");
  return -1;
}

int	luaLen(lua_State *L)
{
  DPRINT_INFO("luaGetArrayMenber\n");
  if (lua_gettop(L) != 1 || !lua_islightuserdata(L, 1))
    {
      DPRINT_ERR("function arguments are incorect\n"
	     "real prototyre is: getLen(lightuserdata entity)\n");
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
      DPRINT_ERR("function arguments are incorect\n"
		 "real prototyre is: yeCopy(lightuserdata src, lightuserdata dest)\n");
      return -1;
    }
  lua_pushlightuserdata(L, yeCopy((Entity *)lua_topointer(L, 1),
				  (Entity *)lua_topointer(L, 2)));
  return 1;
}


int	luaCreateArray(lua_State *L)
{
  DPRINT_INFO("enter luaCreateArray\n");
  if (lua_gettop(L) != 2)
    {
      DPRINT_ERR("function arguments are incorect\n"
	     "real prototyre is: yeCreateArray(int type, lightuserdata father)\n");
      return -1;
    }
  lua_pushlightuserdata(L, yeCreateArray((Entity *)lua_topointer(L, 2),
					 lua_tostring(L, 1)));
  return 1;
}

int	luaPopBack(lua_State *L)
{
  DPRINT_INFO("enter luaArrayPopBack\n");
  if (lua_gettop(L) != 1 || !lua_islightuserdata(L, 1))
    {
      DPRINT_ERR("function arguments are incorect\n"
	     "real prototyre is: arrayPopBack(lightuserdata entity)\n");
      return -1;
    }

  DPRINT_INFO("before popBack\n");
  lua_pushlightuserdata(L, yePopBack(((Entity *)lua_topointer(L, 1))));
  return 1;
}

int	luaPushBack(lua_State *L)
{
  if (lua_gettop(L) < 2 || !lua_islightuserdata(L, 1) ||
      !lua_islightuserdata(L, 2))
    {
      DPRINT_ERR("function arguments are incorect\n"
	     "real prototyre is: yePushBack(lightuserdata entity, lightuserdata toPush, string name)\n");
      return -1;
    }
  if (!lua_isstring(L, 3))
    yePushBack(((Entity *)lua_topointer(L, 1)), (Entity *)lua_topointer(L, 2),
	       lua_tostring(L, 3));
  else
    yePushBack(((Entity *)lua_topointer(L, 1)), (Entity *)lua_topointer(L, 2),
	       NULL);
  return 0;
}


int	luaGetString(lua_State *L)
{
  if (lua_gettop(L) != 1 || !lua_islightuserdata(L, 1))
    {
      DPRINT_ERR("function arguments are incorect\n"
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
      DPRINT_ERR("function arguments are incorect\n"
	     "real prototyre is: getEntityIntue(lightuserdata entity)\n");
      return -1;
    }
  lua_pushnumber(L, (yeGetInt(((Entity *)lua_topointer(L, 1)))));
  return 1;
}

int	luaGetFloat(lua_State *L)
{
  if (lua_gettop(L) != 1 || !lua_islightuserdata(L, 1))
    {
      DPRINT_ERR("function arguments are incorect\n"
	     "real prototyre is: getEntityIntue(lightuserdata entity)\n");
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
      DPRINT_ERR("function arguments are incorect\n"
	     "real prototyre is: unsetFunction(lightuserdata entity, string functionName)\n");
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
     // DPRINT_ERR("function arguments are incorect\n""real prototyre is: setEntityIntue(...)\n");

      return -1;
    }
  yeSetInt(((Entity *)lua_topointer(L, 1)), lua_tonumber(L, 2));
  return 0;
}

int	luaSetFloat(lua_State *L)
{
  if (lua_gettop(L) != 2 || !lua_islightuserdata (L, 1) || !lua_isnumber (L, 2))
    {
      DPRINT_ERR("function arguments are incorect\n"
	     "real prototyre is: setEntityIntue(...)\n");
      return (-1);
    }
  yeSetFloat(((Entity *)lua_topointer(L, 1)), lua_tonumber(L, 2));
  return (0);
}

int	luaRemoveChild(lua_State *L)
{
  if (lua_gettop(L) != 2)
    {
      DPRINT_ERR("function arguments are incorect\n"
	     "real prototyre is: arrayRemove(lightuserdata array, lightuserdata toRemove, int deepSearch)\n");
      return (-1);
    }
  lua_pushlightuserdata(L, yeRemoveChild(YE_TO_ENTITY(lua_topointer(L, 1)),
					 YE_TO_ENTITY(lua_topointer(L, 2))));
  return (1);
}


/* int	luaAddEntity(lua_State *L) */
/* { */
/*   if (lua_gettop(L) != 2) */
/*     { */
/*       DPRINT_ERR("function arguments are incorect\n" */
/* 	     "real prototyre is: addEntity(string objType, string objName)\n"); */
/*       return (-1); */
/*     } */
/*   lua_pushlightuserdata(L, addEntity(lua_tostring(L, 1), lua_tostring(L, 2))); */
/*   return (1); */
/* } */

int	luaUnsetFunction(lua_State *L)
{
  DPRINT_INFO("enter luaUnsetFunction\n");
  if (lua_gettop(L) != 1)
    {
      DPRINT_ERR("function arguments are incorect\n"
	     "real prototyre is: unsetFunction(lightuserdata entity)\n");
      return (-1);
    }
  yeUnsetFunction(YE_TO_ENTITY(lua_topointer(L, 1)));
  return (0);
}

/* int	luaEndGame(lua_State *) */
/* { */
/*   endGame(true); */
/*   return (0);   */
/* } */

/* int	luaNameCall(lua_State *L) */
/* { */
/*   int	nArg = lua_gettop(L); */
/*   const char *name; */
/*   if (nArg < 1) */
/*     { */
/*       DPRINT_ERR("error in callByName\n" */
/* 		 "Prototype is callByName(name, ...)"); */
/*       return (-1); */
/*     } */

/*   name = lua_tostring(L, 1); */
/*   switch (nArg) */
/*     { */
/*     case 1: */
/*       DPRINT_INFO("callByName 0 args"); */
/*       lua_pushlightuserdata(L, callByName(name, 0)); */
/*       return (1); */
/*     case 2: */
/*       DPRINT_INFO("callByName 1 args"); */
/*       lua_pushlightuserdata(L, callByName(name, 1, */
/* 				    T_E(lua_topointer(L, 2)))); */
/*       return (1); */
/*     case 3: */
/*       DPRINT_INFO("callByName 2 args"); */
/*       lua_pushlightuserdata(L, callByName(name, 2, */
/* 				    T_E(lua_topointer(L, 2)), */
/* 				    T_E(lua_topointer(L, 3)))); */
/*       return (1); */
/*     default: */
/*       /\* a good api is always nice with his user(but hey it's lua :) ) *\/ */
/*       DPRINT_ERR("screw you"); */
/*     } */
/*   return (-1); */
/* } */

/* int	luaCall(lua_State *L) */
/* { */
/*   int	nArg = lua_gettop(L); */
/*   if (!nArg) */
/*     { */
/*       DPRINT_ERR("error in luaCall: missing function entity\n"); */
/*       return (-1); */
/*     }  */

/*   if (!lua_topointer(L, 1)) */
/*     return (-1); */
/*   int	entityNArg = getFunctionNumberArgs(T_CE(lua_topointer(L, 1))); */
/*   (void)entityNArg; */
/*   DPRINT_INFO("call with nArg %d %d\n", nArg, entityNArg); */

/*   // DPRINT_ERR("something went really wrong $d $d", nArg, ); */

/*   switch (nArg) */
/*     { */
/*     case 1: */
/*       DPRINT_INFO("call 0 args"); */
/*       lua_pushlightuserdata(L, call(T_F(lua_topointer(L, 1)))); */
/*       return (1); */
/*     case 2: */
/*       DPRINT_INFO("call 1 args"); */
/*       lua_pushlightuserdata(L, call(T_F(lua_topointer(L, 1)), */
/* 				    T_E(lua_topointer(L, 2)))); */
/*       return (1); */
/*     case 3: */
/*       DPRINT_INFO("call 2 args"); */
/*       lua_pushlightuserdata(L, call(T_F(lua_topointer(L, 1)), */
/* 				    T_E(lua_topointer(L, 2)), */
/* 				    T_E(lua_topointer(L, 3)))) ; */
/*       return (1); */
/*     default: */
/*       DPRINT_ERR("screw you"); */
/*     } */
/*   return (-1); */
/* } */

/* int    	luaPushChangeWidgetEvent(lua_State *L) */
/* { */
/*   if (lua_gettop(L) != 2 || !lua_islightuserdata (L, 1) || !lua_islightuserdata (L, 2)) */
/*     { */
/*       DPRINT_ERR("function arguments are incorect\n" */
/* 		 "real prototyre is: pushChangeWidgetEvent(lightuserdata entity, lightuserdata toSet)\n"); */
/*       return -1; */
/*     } */

/*   pushChangeWidgetEvent((StructEntity *)lua_topointer(L, 1), (StructEntity *)lua_topointer(L, 2)); */
/*   return 0; */
/* } */

int	luaFunctionNumberArgs(lua_State *L)
{
  int	nArg = lua_gettop(L);
  if (!nArg)
    {
      DPRINT_ERR("error in luaGetFunctionNumberArgs: missing function entity\n");
      return -1;
    }
  lua_pushnumber(L, yeFunctionNumberArgs(YE_TO_C_ENTITY(lua_topointer(L, 1))));
  return 1;
}

int	luaType(lua_State *L)
{
  if (lua_gettop(L) != 1 || !lua_islightuserdata (L, 1)) {
    DPRINT_ERR("error in getType: missing entity\n");
    return -1;
  }
  lua_pushnumber(L, yeType(YE_TO_ENTITY(lua_topointer(L, 1))));
  return 1;
}

/* int	luaSetTimeType(lua_State *L) */
/* { */
/*   DPRINT_INFO("luaSetInt\n");   */
/*   if (lua_gettop(L) != 1 || !lua_isnumber (L, 1)) */
/*     { */
/*       DPRINT_ERR("function arguments are incorect\n" */
/* 	     "real prototyre is: luaSetTimeType(int type)\n"); */
/*       return -1; */
/*     } */
/*   setTimeType(lua_tonumber(L, 1)); */
/*   return 0; */
/* } */

/* int	luaPlaySound(lua_State *L) */
/* { */
/*   if (lua_gettop(L) != 1 || !lua_isstring (L, 1)) */
/*     { */
/*       DPRINT_ERR("function arguments are incorect\n" */
/* 		 "real prototyre is: playSound(string PathFile)"); */
/*       return (-1); */
/*     } */
/*   static std::vector<MgrSound *> test; */
/*   test.push_back(new MgrSound()); */
/*   test[test.size()-1]->play(lua_tostring(L, 1)); */
/*   return (0);   */
/* } */
