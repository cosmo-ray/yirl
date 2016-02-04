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

#ifndef _YIRL_LUA_BINDING_H_
#define _YIRL_LUA_BINDING_H_

#include "script.h"
#include "lua-convert.h"
#include "widget.h"
#include "keydef.h"
#include <lualib.h>

/* love lua */
int	luaYAnd(lua_State *L);
int	luaRand(lua_State *L);
int	luaRandInit(lua_State *L);

/* Array */
int	luaGet(lua_State *L);
int	luaLen(lua_State *L);
int	luaCreateArray(lua_State *L);
int	luaPopBack(lua_State *L);
int	luaPushBack(lua_State *L);
int	luaRemoveChild(lua_State *L);

/* Entity */
int	luaCopy(lua_State *L);
int	luaType(lua_State *L);

/* string */
int	luaGetString(lua_State *L);
int	luaCreateString(lua_State *L);

/* int */
int	luaGetInt(lua_State *L);
int	luaSetInt(lua_State *L);
int	luaCreateInt(lua_State *L);

/* float */
int	luaGetFloat(lua_State *L);
int	luaSetFloat(lua_State *L);
int	luaCreateFloat(lua_State *L);

/* Function */
int     luaCreateFunction(lua_State *L);
int	luaSetFunction(lua_State *L);
int	luaUnsetFunction(lua_State *L);
int	luaFunctionNumberArgs(lua_State *L);

/* widgets */
int	luaSetMainWid(lua_State *L);
int	luaNewWidget(lua_State *L);
int	luaWidBind(lua_State *L);
int	luaCreateCallback(lua_State *L);
int	luaWidAddCallback(lua_State *L);
int	luaCallCallbackByStr(lua_State *L);
int	luaWidEntity(lua_State *L);

/* event */
int	luaWidNextEve(lua_State *L);
int	luaWidEveIsEnd(lua_State *L);
int	luaEveType(lua_State *L);
int	luaEveKey(lua_State *L);

/* map */
int	luaMapCreatePos(lua_State *L);

#define YES_RET_IF_FAIL(OPERATION)		\
  if (OPERATION < 0) return -1;

static inline int	yesLuaRegister(void *sm)
{
  /* set gobales */
  lua_pushnumber(((YScriptLua *)sm)->l, YKEY_DOWN);
  lua_setglobal(((YScriptLua *)sm)->l, "YKEY_DOWN");
  lua_pushnumber(((YScriptLua *)sm)->l, YKEY_UP);
  lua_setglobal(((YScriptLua *)sm)->l, "YKEY_UP");
  lua_pushnumber(((YScriptLua *)sm)->l, YKEY_NONE);
  lua_setglobal(((YScriptLua *)sm)->l, "YKEY_NONE");

  lua_pushnumber(((YScriptLua *)sm)->l, 27);
  lua_setglobal(((YScriptLua *)sm)->l, "Y_ESC_KEY");
  lua_pushnumber(((YScriptLua *)sm)->l, Y_UP_KEY);
  lua_setglobal(((YScriptLua *)sm)->l, "Y_UP_KEY");
  lua_pushnumber(((YScriptLua *)sm)->l, Y_DOWN_KEY);
  lua_setglobal(((YScriptLua *)sm)->l, "Y_DOWN_KEY");
  lua_pushnumber(((YScriptLua *)sm)->l, Y_LEFT_KEY);
  lua_setglobal(((YScriptLua *)sm)->l, "Y_LEFT_KEY");
  lua_pushnumber(((YScriptLua *)sm)->l, Y_RIGHT_KEY);
  lua_setglobal(((YScriptLua *)sm)->l, "Y_RIGHT_KEY");
    
  /* I love lua */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yAnd", luaYAnd));

  /* Lua conceptor should love me */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yloveNbrToPtr", luaNbrToPtr));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ylovePtrToString", luaPtrToString));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yLovePtrToNumber", luaPtrToNumber));
  
  /*array*/
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeGet", luaGet));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeLen", luaLen));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeCreateArray", luaCreateArray));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yePushBack", luaPushBack));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yePopBack", luaPopBack));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeRemoveChild", luaRemoveChild));

  /* Entity */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeCopy", luaCopy));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeType", luaType));

  /* string */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeGetString", luaGetString));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeCreateString", luaCreateString));


  /* int */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeGetInt", luaGetInt));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeSetInt", luaSetInt));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeCreateInt", luaCreateInt));


  /* float */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeGetFloat", luaGetFloat));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeSetFloat", luaSetFloat));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeCreateFloat", luaCreateFloat));

  /* functions */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeCreateFunction", luaCreateFunction));

  /* widgets */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidNewWidget", luaNewWidget));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidSetMainWid", luaSetMainWid));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidBind", luaWidBind));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidCreateCallback", luaCreateCallback));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidAddCallback", luaWidAddCallback));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidCallCallbackByStr",
				 luaCallCallbackByStr));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidEntity", luaWidEntity));
  // TODO: Add get entity

  /* evenements */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidNextEve", luaWidNextEve));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidEveIsEnd", luaWidEveIsEnd));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidEveType", luaEveType));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidEveKey", luaEveKey));
  // Add ywidEveStat()
  // Add ywidEveMouseX()
  // Add ywidEveMouseY()
  /* map */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapCreatePos", luaMapCreatePos));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yuiRand", luaRand));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yuiRandInit", luaRandInit));
  
  return 0;
}

#undef YES_RET_IF_FAIL

#endif
