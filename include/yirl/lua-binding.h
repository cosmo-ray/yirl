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
#include "game.h"
#include "container.h"
#include <lualib.h>
#include <lauxlib.h>

/* Entity objets */
int	luaentity_tocentity(lua_State *L);
int	luaentity_destroy(lua_State *L);
int	luaentity_tostring(lua_State *L);
int	luaentity_newfunc(lua_State *L);
int	luaentity_newint(lua_State *L);
int	luaentity_newstring(lua_State *L);
int	luaentity_newarray(lua_State *L);
int	luaentity_index(lua_State *L);
int	luaentity_newindex(lua_State *L);
int	luaentity_call(lua_State *L);
int	luaentity_mul(lua_State *L);
int	luaentity_div(lua_State *L);
int	luaentity_add(lua_State *L);
int	luaentity_sub(lua_State *L);
int	luaentity_lt(lua_State *L);
int	luaentity_eq(lua_State *L);
int	luaentity_wrapp(lua_State *L);
int	luaentity__wrapp_(lua_State *L);
int	luaentity_len(lua_State *L);
int	luaentity_pushback(lua_State *L);
int	luaentity_tofloat(lua_State *L);
int	luaentity_setfloat(lua_State *L);
int	luaentity_remove(lua_State *L);
int	luaentity_toint(lua_State *L);

/* love lua */
int	luayIsLightUserData(lua_State *L);
int	luaYAnd(lua_State *L);
int	luayOr(lua_State *L);
int     luaStringToPtr(lua_State *l);
int     luaToPtr(lua_State *l);

/* util */
int	luaRand(lua_State *L);
int	luaRandInit(lua_State *L);
int	luaYuiAbs(lua_State *L);
int	luayuiMkdir(lua_State *L);

/* Array */
int	luaGet(lua_State *L);
int	luaLen(lua_State *L);
int	luaCreateArray(lua_State *L);
int	luaPopBack(lua_State *L);
int	luaPushBack(lua_State *L);
int	luayePushAt(lua_State *L);
int	luayeInsertAt(lua_State *L);
int	luayeGetKeyAt(lua_State *L);
int	luaRemoveChild(lua_State *L);
int	luaDestroy(lua_State *L);
int	luaSetAt(lua_State *L);
int	luaYeReplace(lua_State *L);
int	luaYeReplaceBack(lua_State *L);
int	luaYeReplaceAtIdx(lua_State *L);
int	luaYeSwapElems(lua_State *L);
int	luayeRenameIdxStr(lua_State *L);
int	luayeGetPush(lua_State *L);

/* Entity */
int	luaCopy(lua_State *L);
int	luaType(lua_State *L);
int	luaYeToLuaString(lua_State *L);
int	luaYeIncrRef(lua_State *L);
int	luayeEntitiesArraySize(lua_State *L);
int	luayeFreeEntitiesInStack(lua_State *L);
int	luayeEntitiesUsed(lua_State *L);
int	luayeRefCount(lua_State *L);
int	luayeConvert(lua_State *L);
int	luayePatchCreate(lua_State *L);
int	luayePatchAply(lua_State *L);

/* string */
int	luaGetString(lua_State *L);
int	luaCreateString(lua_State *L);
int	luaSetString(lua_State *L);
int	luayeCreateYirlFmtString(lua_State *L);
int	luayeStrCaseCmp(lua_State *L);
int	luayeToLower(lua_State *L);

/* int */
int	luaGetInt(lua_State *L);
int	luaSetInt(lua_State *L);
int	luaCreateInt(lua_State *L);
int	luayeGetIntAt(lua_State *L);

/* float */
int	luaGetFloat(lua_State *L);
int	luaSetFloat(lua_State *L);
int	luaCreateFloat(lua_State *L);

/* Function */
int     luaCreateFunction(lua_State *L);
int	luaSetFunction(lua_State *L);
int	luaUnsetFunction(lua_State *L);
int	luaFunctionNumberArgs(lua_State *L);
int	luaYesCall(lua_State *L);

/* widgets */
int	luaSetMainWid(lua_State *L);
int	luaNewWidget(lua_State *L);
int	luaWidNext(lua_State *L);
int	luaAddSubType(lua_State *L);
int	luaywidAction(lua_State *L);

/* event */
int	luaWidNextEve(lua_State *L);
int	luaWidEveIsEnd(lua_State *L);
int	luaEveType(lua_State *L);
int	luaEveKey(lua_State *L);
int	luaywidEveMousePos(lua_State *L);

/* rect */
int	luaYwRectCreate(lua_State *L);
int	luaywRectCollision(lua_State *L);
int	luaywRectW(lua_State *L);
int	luaywRectH(lua_State *L);

/* pos */
int	luaywPosCreate(lua_State *L);
int	luaywPosSet(lua_State *L);
int	luaywPosIsSameX(lua_State *L);
int	luaywPosIsSameY(lua_State *L);
int	luaywPosIsSame(lua_State *L);
int	luaywPosAdd(lua_State *L);
int	luaywPosPrint(lua_State *L);
int	luaywPosToString(lua_State *L);
int	luaywPosX(lua_State *L);
int	luaywPosY(lua_State *L);
int	luaywPosAngle(lua_State *L);
int	luaywPosMoveToward2(lua_State *L);

/* map */
int	luaYwMapPosFromInt(lua_State *L);
int	luaYwMapIntFromPos(lua_State *L);
int	luaYwMapMove(lua_State *L);
int	luaYwMapSmootMove(lua_State *L);
int	luaYwMapRemove(lua_State *L);
int	luaYwMapPushElem(lua_State *L);
int	luaYwMapPushNbr(lua_State *L);
int	luaYwMapGetCase(lua_State *L);
int	luaYwMapW(lua_State *L);
int	luaYwMapH(lua_State *L);
int	luaYwMapSetSmootMovement(lua_State *L);
int	luaMvTablePush(lua_State *L);
int	luaYwMapGetEntityById(lua_State *L);
int	luaYMapAdvence(lua_State *L);
int	luaYwMapGetResource(lua_State *L);
int	luaYwMapGetResourceId(lua_State *L);
int	luaYwMapGetIdByElem(lua_State *L);

/* container */
int	luaYwCntGetEntry(lua_State *L);
int	luaYwPushNewWidget(lua_State *L);
int	luaYwCntPopLastEntry(lua_State *L);
int	luaYwReplaceEntry(lua_State *L);
int	luaywCntWidgetFather(lua_State *L);
int	luaywCntConstructChilds(lua_State *L);

/* canvas */
int	luaYwCanvasRemoveObj(lua_State *L);
int	luaYwCanvasMoveObjByIdx(lua_State *L);
int	luaYwCanvasNewObj(lua_State *L);
int	luaYwCanvasObjPos(lua_State *L);
int	luaYwCanvasObjSize(lua_State *L);
int	luaywCanvasObjAngle(lua_State *L);
int	luaYwCanvasObjFromIdx(lua_State *L);
int	luaYwCanvasIdxFromObj(lua_State *L);
int	luaYwCanvasObjSetPos(lua_State *L);
int	luaYwCanvasNewCollisionsArray(lua_State *L);
int	luaYwCanvasNewCollisionsArrayWithRectangle(lua_State *L);
int	luaYwCanvasNewText(lua_State *L);
int	luaywCanvasNewTextExt(lua_State *L);
int	luaYwCanvasObjSetResourceId(lua_State *L);
int	luaywCanvasObjClearCache(lua_State *L);
int	luaywCanvasNewRect(lua_State *L);
int	luaywCanvasCheckCollisions(lua_State *L);
int	luaywCanvasNewCollisionsArrayExt(lua_State *L);
int	luaywCanvasNewImg(lua_State *L);
int	luaywCanvasMoveObj(lua_State *L);
int	luaywCanvasAdvenceObj(lua_State *L);
int	luaywCanvasSwapObj(lua_State *L);
int	luaywCanvasForceSize(lua_State *L);
int	luaywCanvasRotate(lua_State *L);
int	luaywCanvasObjPointTopTo(lua_State *L);
int	luaywCanvasObjPointRightTo(lua_State *L);
int	luaywCanvasObjIsOut(lua_State *L);
int	luaywCanvasObjectsCheckColisions(lua_State *L);
int	luaywCanvasPopObj(lua_State *L);
int	luaywCanvasCreateYTexture(lua_State *L);
int	luaywCanvasNewImgFromTexture(lua_State *L);
int	luaywCanvasSetWeight(lua_State *L);
int	luaywCanvasDoPathfinding(lua_State *L);

/* texture */
int	luaywTextureNewImg(lua_State *L);
int	luaywTextureMerge(lua_State *L);
int	luaywTextureNormalize(lua_State *L);

/* Menu */
int	luaywMenuCallActionOn(lua_State *lua);
int	luaywMenuGetCurrent(lua_State *lua);
int	luaywMenuPushEntry(lua_State *lua);
int	luaywMenuGetCurrentEntry(lua_State *lua);
int	luaywMenuMove(lua_State *lua);

/* Game and Module */
int	luaGetMod(lua_State *L);
int	luaGCall(lua_State *L);
int	luaYgRegistreFunc(lua_State *L);
int	luaYgFileToEnt(lua_State *L);
int	luaYgEntToFile(lua_State *L);
int	luaYGet(lua_State *L);
int	luaygSetInt(lua_State *L);

/* Audio */
int	luaySoundLoad(lua_State *L);
int	luaySoundPlay(lua_State *L);
int	luaySoundPlayLoop(lua_State *L);
int	luaySoundStop(lua_State *L);

/* condition */
int	luayeCheckCondition(lua_State *L);

#define YES_RET_IF_FAIL(OPERATION)		\
  if (OPERATION < 0) return -1;

#define YES_LUA_REGISTRE_CALL(manager, name)	\
  YES_RET_IF_FAIL(ysRegistreFunc(manager,#name, lua##name))


#define LUA_SET_INT_GLOBAL(manager, value) do {				\
    lua_pushnumber(((YScriptLua *)manager)->l, value);			\
    lua_setglobal(((YScriptLua *)manager)->l, #value);			\
  } while (0)

#define LUA_SET_INT_GLOBAL_VAL(manager, key, val) do {			\
    lua_pushnumber(((YScriptLua *)manager)->l, val);			\
    lua_setglobal(((YScriptLua *)manager)->l, #key);			\
  } while (0)

static inline int	yesLuaRegister(void *sm)
{
  lua_State *L = ((YScriptLua *)sm)->l;

  lua_pushlightuserdata(L, (void *)NOTHANDLE);
  lua_setglobal(L, "YEVE_NOTHANDLE");
  lua_pushlightuserdata(L, (void *)NOACTION);
  lua_setglobal(L, "YEVE_NOACTION");
  lua_pushlightuserdata(L, (void *)ACTION);
  lua_setglobal(L, "YEVE_ACTION");
  lua_pushlightuserdata(L, (void *)YJSON);
  lua_setglobal(L, "YJSON");

  lua_pushlightuserdata(L, (void *)1);
  lua_setglobal(L, "Y_TRUE");
  lua_pushlightuserdata(L, (void *)0);
  lua_setglobal(L, "Y_FALSE");


  /* set gobales */
  LUA_SET_INT_GLOBAL_VAL(sm, Y_A_KEY, 'a');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_B_KEY, 'b');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_C_KEY, 'c');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_D_KEY, 'd');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_E_KEY, 'e');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_F_KEY, 'f');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_G_KEY, 'g');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_H_KEY, 'h');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_I_KEY, 'i');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_J_KEY, 'j');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_K_KEY, 'k');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_L_KEY, 'l');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_M_KEY, 'm');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_N_KEY, 'n');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_O_KEY, 'o');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_P_KEY, 'p');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_Q_KEY, 'q');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_R_KEY, 'r');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_S_KEY, 's');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_T_KEY, 't');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_U_KEY, 'u');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_V_KEY, 'v');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_W_KEY, 'w');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_X_KEY, 'x');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_Y_KEY, 'y');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_Z_KEY, 'z');
  /* Spaceeeeee ! */
  LUA_SET_INT_GLOBAL_VAL(sm, Y_SPACE_KEY, ' ');
  LUA_SET_INT_GLOBAL_VAL(sm, Y_ENTER_KEY, '\n');

  LUA_SET_INT_GLOBAL_VAL(sm, Y_CNT_VERTICAL, CNT_VERTICAL);
  LUA_SET_INT_GLOBAL_VAL(sm, Y_CNT_HORIZONTAL, CNT_HORIZONTAL);

  LUA_SET_INT_GLOBAL_VAL(sm, Y_CNT_STACK, CNT_STACK);
  LUA_SET_INT_GLOBAL_VAL(sm, Y_CNT_NONE, CNT_NONE);

  lua_pushnumber(L, YKEY_DOWN);
  lua_setglobal(L, "YKEY_DOWN");
  lua_pushnumber(L, YKEY_UP);
  lua_setglobal(L, "YKEY_UP");
  lua_pushnumber(L, YKEY_NONE);
  lua_setglobal(L, "YKEY_NONE");
  LUA_SET_INT_GLOBAL(sm, YKEY_MOUSEDOWN);
  LUA_SET_INT_GLOBAL(sm, YKEY_MOUSEWHEEL);
  LUA_SET_INT_GLOBAL(sm, YKEY_MOUSEMOTION);

  lua_pushstring(L, YIRL_MODULES_PATH);
  lua_setglobal(L, "YIRL_MODULES_PATH");

  lua_pushnumber(L, 27);
  lua_setglobal(L, "Y_ESC_KEY");
  lua_pushnumber(L, Y_UP_KEY);
  lua_setglobal(L, "Y_UP_KEY");
  lua_pushnumber(L, Y_DOWN_KEY);
  lua_setglobal(L, "Y_DOWN_KEY");
  lua_pushnumber(L, Y_LEFT_KEY);
  lua_setglobal(L, "Y_LEFT_KEY");
  lua_pushnumber(L, Y_RIGHT_KEY);
  lua_setglobal(L, "Y_RIGHT_KEY");

  static const struct luaL_Reg luaentity_methods[] = {
    {"__gc", luaentity_destroy},
    {"__tostring", luaentity_tostring},
    {"__index", luaentity_index},
    {"__newindex", luaentity_newindex},
    {"__call", luaentity_call},
    {"__mul", luaentity_mul},
    {"__add", luaentity_add},
    {"__div", luaentity_div},
    {"__sub", luaentity_sub},
    {"__lt", luaentity_lt},
    {"__eq", luaentity_eq},
    {"cent", luaentity_tocentity},
    {"len", luaentity_len},
    {"push_back", luaentity_pushback},
    {"to_float", luaentity_tofloat},
    {"to_string", luaentity_tostring},
    {"to_int", luaentity_toint},
    {"set_float", luaentity_setfloat},
    {"remove", luaentity_remove},
    {NULL, NULL},
  };
  static const struct luaL_Reg luaentity_functions[] = {
    { "new_int", luaentity_newint},
    { "new_string", luaentity_newstring},
    { "new_func", luaentity_newfunc},
    { "new_array", luaentity_newarray},
    {"wrapp", luaentity_wrapp},
    {"_wrapp_", luaentity__wrapp_},
    {NULL, NULL},
  };

  luaL_newmetatable(L, "Entity");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_setfuncs(L, luaentity_methods, 0);
  luaL_newlib(L, luaentity_functions);
  lua_setglobal(L, "Entity");

  /* set entities type global */
  LUA_SET_INT_GLOBAL(sm, YINT);
  LUA_SET_INT_GLOBAL(sm, YSTRING);
  LUA_SET_INT_GLOBAL(sm, YFLOAT);
  LUA_SET_INT_GLOBAL(sm, YARRAY);
  LUA_SET_INT_GLOBAL(sm, YFUNCTION);
  LUA_SET_INT_GLOBAL(sm, YDATA);


  /* I love lua */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yAnd", luaYAnd));
  YES_LUA_REGISTRE_CALL(sm, yOr);
  YES_LUA_REGISTRE_CALL(sm, yIsLightUserData);

  /* utils */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yuiRand", luaRand));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yuiRandInit", luaRandInit));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yuiAbs", luaYuiAbs));
  YES_LUA_REGISTRE_CALL(sm, yuiMkdir);

  /* Lua conceptor should love me */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yloveNbrToPtr", luaNbrToPtr));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ylovePtrToString", luaPtrToString));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yLovePtrToNumber", luaPtrToNumber));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yLovePtrToInt32", luaPtrToInt32));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yloveStringToPtr", luaStringToPtr));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yLoveToPtr", luaToPtr));


  /*array*/
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeGet", luaGet));
  YES_LUA_REGISTRE_CALL(sm, yeGetKeyAt);
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeLen", luaLen));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeCreateArray", luaCreateArray));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yePushBack", luaPushBack));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yePopBack", luaPopBack));
  YES_LUA_REGISTRE_CALL(sm, yePushAt);
  YES_LUA_REGISTRE_CALL(sm, yeInsertAt);
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeRemoveChild", luaRemoveChild));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeDestroy", luaDestroy));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeReplace", luaYeReplace));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeReplaceAtIdx", luaYeReplaceAtIdx));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeReplaceBack", luaYeReplaceBack));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeSwapElems", luaYeSwapElems));
  YES_LUA_REGISTRE_CALL(sm, yeRenameIdxStr);
  YES_LUA_REGISTRE_CALL(sm, yeGetPush);

  /* Entity */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeCopy", luaCopy));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeType", luaType));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeSetAt", luaSetAt));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeToLuaString", luaYeToLuaString));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeIncrRef", luaYeIncrRef));
  YES_LUA_REGISTRE_CALL(sm, yeEntitiesArraySize);
  YES_LUA_REGISTRE_CALL(sm, yeEntitiesUsed);
  YES_LUA_REGISTRE_CALL(sm, yeFreeEntitiesInStack);
  YES_LUA_REGISTRE_CALL(sm, yeRefCount);
  YES_LUA_REGISTRE_CALL(sm, yeConvert);
  YES_LUA_REGISTRE_CALL(sm, yePatchCreate);
  YES_LUA_REGISTRE_CALL(sm, yePatchAply);

  /* string */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeGetString", luaGetString));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeSetString", luaSetString));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeCreateString", luaCreateString));
  YES_LUA_REGISTRE_CALL(sm, yeCreateYirlFmtString);
  YES_LUA_REGISTRE_CALL(sm, yeStrCaseCmp);
  YES_LUA_REGISTRE_CALL(sm, yeToLower);


  /* int */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeGetInt", luaGetInt));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeSetInt", luaSetInt));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeCreateInt", luaCreateInt));
  YES_LUA_REGISTRE_CALL(sm, yeGetIntAt);

  /* float */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeGetFloat", luaGetFloat));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeSetFloat", luaSetFloat));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeCreateFloat", luaCreateFloat));

  /* functions */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yeCreateFunction", luaCreateFunction));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "yesCall", luaYesCall));

  /* widgets */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidNewWidget", luaNewWidget));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidSetMainWid", luaSetMainWid));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidNext", luaWidNext));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidAddSubType", luaAddSubType));
  YES_LUA_REGISTRE_CALL(sm, ywidAction);

  /* evenements */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidNextEve", luaWidNextEve));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidEveIsEnd", luaWidEveIsEnd));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidEveType", luaEveType));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywidEveKey", luaEveKey));
  YES_LUA_REGISTRE_CALL(sm, ywidEveMousePos);
  // Add ywidEveStat()
  // Add ywidEveMouseX()
  // Add ywidEveMouseY()

  /* map */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapPosFromInt", luaYwMapPosFromInt));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapIntFromPos", luaYwMapIntFromPos));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapGetCase", luaYwMapGetCase));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapMove", luaYwMapMove));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapSmootMove", luaYwMapSmootMove));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapRemove", luaYwMapRemove));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapPushElem", luaYwMapPushElem));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapPushNbr", luaYwMapPushNbr));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapW", luaYwMapW));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapH", luaYwMapH));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapSetSmootMovement",
				 luaYwMapSetSmootMovement));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMvTablePush", luaMvTablePush));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapGetEntityById",
				 luaYwMapGetEntityById));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapAdvence", luaYMapAdvence));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapGetResource", luaYwMapGetResource));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapGetResourceId",
				 luaYwMapGetResourceId));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywMapGetIdByElem", luaYwMapGetIdByElem));

  /* menu */
  YES_LUA_REGISTRE_CALL(sm, ywMenuCallActionOn);
  YES_LUA_REGISTRE_CALL(sm, ywMenuGetCurrent);
  YES_LUA_REGISTRE_CALL(sm, ywMenuGetCurrentEntry);
  YES_LUA_REGISTRE_CALL(sm, ywMenuPushEntry);
  YES_LUA_REGISTRE_CALL(sm, ywMenuMove);

  /* container */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywReplaceEntry", luaYwReplaceEntry));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCntGetEntry", luaYwCntGetEntry));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywPushNewWidget", luaYwPushNewWidget));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCntPopLastEntry", luaYwCntPopLastEntry));
  YES_LUA_REGISTRE_CALL(sm, ywCntWidgetFather);
  YES_LUA_REGISTRE_CALL(sm, ywCntConstructChilds);

  /* rect */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywRectCreate", luaYwRectCreate));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywRectX", luaywPosX));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywRectY", luaywPosY));
  YES_LUA_REGISTRE_CALL(sm, ywRectW);
  YES_LUA_REGISTRE_CALL(sm, ywRectH);
  YES_LUA_REGISTRE_CALL(sm, ywRectCollision);

  /* pos */
  YES_LUA_REGISTRE_CALL(sm, ywPosX);
  YES_LUA_REGISTRE_CALL(sm, ywPosY);
  YES_LUA_REGISTRE_CALL(sm, ywPosCreate);
  YES_LUA_REGISTRE_CALL(sm, ywPosSet);
  YES_LUA_REGISTRE_CALL(sm, ywPosIsSame);
  YES_LUA_REGISTRE_CALL(sm, ywPosIsSameX);
  YES_LUA_REGISTRE_CALL(sm, ywPosIsSameY);
  YES_LUA_REGISTRE_CALL(sm, ywPosAdd);
  YES_LUA_REGISTRE_CALL(sm, ywPosPrint);
  YES_LUA_REGISTRE_CALL(sm, ywPosToString);
  YES_LUA_REGISTRE_CALL(sm, ywPosAngle);
  YES_LUA_REGISTRE_CALL(sm, ywPosMoveToward2);

  /* Size, they're pos with diferent names */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywSizeW", luaywPosX));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywSizeH", luaywPosY));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywSizeCreate", luaywPosCreate));

  /* canvas */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCanvasRemoveObj", luaYwCanvasRemoveObj));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCanvasMoveObjByIdx",
				 luaYwCanvasMoveObjByIdx));
  YES_LUA_REGISTRE_CALL(sm, ywCanvasAdvenceObj);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasObjAngle);
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCanvasNewObj", luaYwCanvasNewObj));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCanvasNewText", luaYwCanvasNewText));
  YES_LUA_REGISTRE_CALL(sm, ywCanvasNewTextExt);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasNewRect);
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCanvasObjPos", luaYwCanvasObjPos));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCanvasObjSize", luaYwCanvasObjSize));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCanvasObjFromIdx", luaYwCanvasObjFromIdx));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCanvasIdxFromObj", luaYwCanvasIdxFromObj));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCanvasObjSetPos", luaYwCanvasObjSetPos));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCanvasNewCollisionsArray",
				 luaYwCanvasNewCollisionsArray));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCanvasNewCollisionsArrayWithRectangle",
				 luaYwCanvasNewCollisionsArrayWithRectangle));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ywCanvasObjSetResourceId",
				 luaYwCanvasObjSetResourceId));
  YES_LUA_REGISTRE_CALL(sm, ywCanvasObjClearCache);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasNewCollisionsArrayExt);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasCheckCollisions);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasNewImg);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasMoveObj);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasSwapObj);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasForceSize);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasRotate);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasObjPointTopTo);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasObjPointRightTo);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasObjIsOut);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasObjectsCheckColisions);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasPopObj);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasCreateYTexture);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasNewImgFromTexture);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasSetWeight);
  YES_LUA_REGISTRE_CALL(sm, ywCanvasDoPathfinding);

  /* texture */
  YES_LUA_REGISTRE_CALL(sm, ywTextureNewImg);
  YES_LUA_REGISTRE_CALL(sm, ywTextureMerge);
  YES_LUA_REGISTRE_CALL(sm, ywTextureNormalize);

  /* Game and Modules */
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ygGetMod", luaGetMod));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ygCall", luaGCall));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ygRegistreFunc", luaYgRegistreFunc));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ygFileToEnt", luaYgFileToEnt));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ygEntToFile", luaYgEntToFile));
  YES_RET_IF_FAIL(ysRegistreFunc(sm, "ygGet", luaYGet));
  YES_LUA_REGISTRE_CALL(sm, ygSetInt);

  /* Audio */
  YES_LUA_REGISTRE_CALL(sm, ySoundLoad);
  YES_LUA_REGISTRE_CALL(sm, ySoundPlay);
  YES_LUA_REGISTRE_CALL(sm, ySoundPlayLoop);
  YES_LUA_REGISTRE_CALL(sm, ySoundStop);

  /* Condition */
  YES_LUA_REGISTRE_CALL(sm, yeCheckCondition);

  return 0;
}

#undef LUA_SET_INT_GLOBAL
#undef YES_RET_IF_FAIL
#undef YES_LUA_REGISTRE_CALL
#undef LUA_SET_INT_GLOBAL_VAL
#endif
