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
#include	<unistd.h>
#include	"lua-binding.h"
#include	"debug.h"
#include	"entity.h"
#include	"entity-script.h"
#include	"map.h"
#include	"container.h"
#include	"canvas.h"
#include	"texture.h"
#include	"menu.h"
#include	"game.h"
#include	"timer.h"
#include	"lua-convert.h"
#include	"condition.h"

Entity *YLUA_NO_DESTROY_ORPHAN = ((void *)0x1);

struct entityWrapper {
  Entity *e;
  int needDestroy;
};

Entity *luaEntityAt(lua_State *L, int pos)
{
  if (lua_islightuserdata(L, pos)) {
      void  *udata = lua_touserdata(L, pos);
      if (yeIsPtrAnEntity(udata))
	return udata;
      return NULL;
    }

  if (!lua_isuserdata(L, pos))
    return NULL;
  struct entityWrapper *ew = luaL_checkudata(L, pos, "Entity");
  return ew->e;
}

static struct entityWrapper *createEntityWrapper(lua_State *L, int fatherPos,
						 Entity **father)
{
  struct entityWrapper *ew;
  int needDestroy = !lua_isuserdata(L, fatherPos);

  if (*father == YLUA_NO_DESTROY_ORPHAN) {
    needDestroy = 0;
    goto finish_init;
  }
  if (needDestroy)
    *father = NULL;
  else
    *father = luaEntityAt(L, fatherPos);

 finish_init:
  ew = lua_newuserdata(L, sizeof(struct entityWrapper));
  ew->needDestroy = needDestroy;
  luaL_getmetatable(L, "Entity");
  lua_setmetatable(L, -2);
  return ew;
}

int luaOrEntType(lua_State *L, int i)
{
	int t;
	if (lua_islightuserdata(L, i)) {
		void  *udata = lua_touserdata(L, i);

		if (yeIsPtrAnEntity(udata)) {
			t = yeType(udata);
			goto entity_type;
		}
		return YLUA_DONT_KNOW;
	} else if (lua_isuserdata(L, i)) {
		struct entityWrapper *ew = luaL_checkudata(L, i, "Entity");
		t = yeType(ew->e);

	entity_type:
		if (t == YINT)
			return YLUA_INT;
		else if (t == YSTRING)
			return YLUA_STR;
		else if (t == YFLOAT)
			return YLUA_FLOAT;
		return YLUA_ENTIY;
	} else if (lua_isnumber(L, i)) {
		return YLUA_INT;
	} else if (lua_isstring(L, i)) {
		return YLUA_STR;
	}
	return YLUA_DONT_KNOW;
}

intptr_t luaNumberAt(lua_State *L, int i)
{
  if (lua_islightuserdata(L, i)) {
    void  *udata = lua_touserdata(L, i);

    if (yeIsPtrAnEntity(udata))
      return yeGetInt(udata);
    return (intptr_t)udata;
  } else if (lua_isuserdata(L, i)) {
    struct entityWrapper *ew = luaL_checkudata(L, i, "Entity");

    if (unlikely(yeType(ew->e) == YFLOAT))
	    return yeGetFloat(ew->e);
    return yeGetInt(ew->e);
  } else if (lua_isnumber(L, i)) {
    return lua_tonumber(L, i);
  }
  return 0;
}

int	luaentity_lt(lua_State *L)
{
  double i0 = luaNumberAt(L, 1);
  double i1 = luaNumberAt(L, 2);

  lua_pushboolean(L, i0 < i1);
  return 1;
}

int	luaentity_eq(lua_State *L)
{
  double i0 = luaNumberAt(L, 1);
  double i1 = luaNumberAt(L, 2);

  lua_pushboolean(L, i0 == i1);
  return 1;
}

int	luaentity_sub(lua_State *L)
{
  double i0 = luaNumberAt(L, 1);
  double i1 = luaNumberAt(L, 2);

  lua_pushnumber(L, i0 - i1);
  return 1;
}

int	luaentity_add(lua_State *L)
{
  double i0 = luaNumberAt(L, 1);
  double i1 = luaNumberAt(L, 2);

  lua_pushnumber(L, i0 + i1);
  return 1;
}

int	luaentity_div(lua_State *L)
{
  double i0 = luaNumberAt(L, 1);
  double i1 = luaNumberAt(L, 2);

  lua_pushnumber(L, i0 / i1);
  return 1;
}

int	luaentity_mul(lua_State *L)
{
  double i0 = luaNumberAt(L, 1);
  double i1 = luaNumberAt(L, 2);

  lua_pushnumber(L, i0 * i1);
  return 1;
}

int	luaentity_call(lua_State *L)
{
  struct entityWrapper *ew = luaL_checkudata(L, 1, "Entity");

  lua_pushlightuserdata(L, ew->e);
  lua_replace(L, 1);
  return luaYesCall(L);
}

int	luaentity_remove(lua_State *L)
{
  Entity *ae = luaEntityAt(L, 1);
  Entity *e = luaEntityAt(L, 2);

  lua_pushlightuserdata(L, yeRemoveChild(ae, e));
  return 1;
}

int	luaentity_newindex(lua_State *L)
{
	struct entityWrapper *ew = luaL_checkudata(L, 1, "Entity");
	Entity *toPush;

	if (lua_isboolean(L, 3)) {
		toPush = yeCreateInt(lua_toboolean(L, 3), NULL, NULL);
	} else if (lua_isnumber(L, 3)) {
		toPush = yeCreateInt(lua_tonumber(L, 3), NULL, NULL);
	} else if (lua_isstring(L, 3)) {
		toPush = yeCreateString(lua_tostring(L, 3), NULL, NULL);
		/* this would be sure a cool feature */
		/* } else if (lua_isfunction(L, 3)) { */
		/*   abort(); */
	} else if (lua_istable(L, 3)) {
		toPush = yeCreateArray(NULL, NULL);

		/* In lua array start by default at 1 ! */
		/* It's retarded */
		for (int i = 1 ;; ++i) {
			lua_geti(L, 3, i);
			if (lua_isnoneornil(L, 4)) {
				lua_pop(L, 1);
				break;
			} else if (lua_isnumber(L, 4)) {
				yeCreateInt(lua_tointeger(L, 4), toPush, NULL);
			} else if (lua_isstring(L, 4)) {
				yeCreateString(lua_tostring(L, 4), toPush,
					       NULL);
			}
			lua_pop(L, 1);
		}
	} else if (lua_islightuserdata(L, 3) || lua_isuserdata(L, 3)) {
		toPush = luaEntityAt(L, 3);

		if (toPush) {
			if (yeType(toPush) == YINT)
				toPush = yeCreateInt(yeGetIntDirect(toPush),
						     NULL, NULL);
			else if (yeType(toPush) == YFLOAT)
				toPush = yeCreateFloat(yeGetFloatDirect(toPush),
						       NULL, NULL);
			else
				yeIncrRef(toPush);
		} else {
			toPush = yeCreateInt((int_ptr_t)lua_touserdata(L, 3),
					     NULL, NULL);
		}
	} else if (lua_isnil(L, 3)) {
		if (lua_isnumber(L, 2))
			yeRemoveChild(ew->e, lua_tonumber(L, 2));
		else
			yeRemoveChild(ew->e, lua_tostring(L, 2));
		return 0;
	} else {
		return luaL_error(L, "type %d not handle", lua_type(L, 3));
	}

	if (lua_isnumber(L, 2)) {
		yePushAt(ew->e, toPush, lua_tonumber(L, 2));
	} else if (lua_isstring(L, 2)) {
		yeReplaceBack(ew->e, toPush, lua_tostring(L, 2));
	}
	yeDestroy(toPush);
	return 0;
}

int	luaentity_index(lua_State *L)
{
	struct entityWrapper *ew = luaL_checkudata(L, 1, "Entity");
	int isNumber = lua_isnumber(L, 2);
	int isString = lua_isstring(L, 2);
	Entity *ret;
	struct entityWrapper *ew_ret;
	int isbaseMethode;

	lua_getmetatable(L, 1);
	lua_pushvalue(L, 2);
	lua_rawget(L, 3);
	isbaseMethode = !!lua_type(L, lua_gettop(L));
	if (isbaseMethode) {
		return 1;
	}

	if (isNumber) {
		ret = yeGetByIdx(ew->e, lua_tonumber(L, 2));
	} else if (isString) {
		ret = yeGet(ew->e, lua_tostring(L, 2));
	} else if (lua_islightuserdata(L, 2) || lua_isuserdata(L, 2)) {
		Entity *ek = luaEntityAt(L, 2);

		if (yeType(ek) == YSTRING)
			ret = yeGet(ew->e, yeGetString(ek));
		else if (yeType(ek) == YINT)
			ret = yeGet(ew->e, yeGetInt(ek));
		else
			return 0;
	} else {
		return 0;
	}
	if (!ret)
		return 0;
	ew_ret = createEntityWrapper(L, 0, &YLUA_NO_DESTROY_ORPHAN);
	ew_ret->e = ret;
	return 1;
}

int	luaentity__wrapp_(lua_State *L)
{
  struct entityWrapper *ret;
  Entity *e = luaEntityAt(L, 1);
  int needDestroy = lua_toboolean(L, 2);
  Entity *father = needDestroy ? NULL : YLUA_NO_DESTROY_ORPHAN;

  ret = createEntityWrapper(L, 2, &father);
  ret->e = e;
  return 1;
}

int	luaentity_wrapp(lua_State *L)
{
  struct entityWrapper *ret;
  Entity *e = luaEntityAt(L, 1);

  ret = createEntityWrapper(L, 0, &YLUA_NO_DESTROY_ORPHAN);
  ret->e = e;
  return 1;
}

int	luaentity_tostring(lua_State *L)
{
  struct entityWrapper *ew = luaL_checkudata(L, 1, "Entity");

  if (yeType(ew->e) == YSTRING) {
    lua_pushstring(L, yeGetString(ew->e));
    return 1;
  }
  char *str = yeToCStr(ew->e, 4, YE_FORMAT_PRETTY);
  lua_pushstring(L, str);
  free(str);
  return 1;
}

int	luaentity_tocentity(lua_State *L)
{
  struct entityWrapper *ew = luaL_checkudata(L, 1, "Entity");

  lua_pushlightuserdata(L, ew->e);
  return 1;
}

int	luaentity_len(lua_State *L)
{
  lua_pushnumber(L, yeLen(luaEntityAt(L, 1)));
  return 1;
}

int	luaentity_tofloat(lua_State *L)
{
  lua_pushnumber(L, yeGetFloat(luaEntityAt(L, 1)));
  return 1;
}

int	luaentity_setfloat(lua_State *L)
{
  yeSetFloat(luaEntityAt(L, 1), lua_tonumber(L, 2));
  return 0;
}

int	luaentity_toint(lua_State *L)
{
  lua_pushnumber(L, yeGetInt(luaEntityAt(L, 1)));
  return 1;
}

int	luaentity_pushback(lua_State *L)
{
  lua_pushnumber(L, yePushBack(luaEntityAt(L, 1), luaEntityAt(L, 2),
			       lua_tostring(L, 3)));
  return 1;
}

int	luaentity_destroy(lua_State *L)
{
  struct entityWrapper *ew = luaL_checkudata(L, 1, "Entity");

  if (ew->needDestroy) {
    yeDestroy(ew->e);
    ew->needDestroy = 0;
    ew->e = NULL;
  }
  return 0;
}

int	luaentity_newarray(lua_State *L)
{
  const char *name = lua_tostring(L, 2);
  Entity *father = NULL;
  struct entityWrapper *ew = createEntityWrapper(L, 1, &father);

  ew->e = yeCreateArray(father, name);
  return 1;
}


int	luaentity_newcopy(lua_State *L)
{
	Entity *val = luaEntityAt(L, 1);
	const char *name = lua_tostring(L, 3);
	Entity *father = NULL;
	struct entityWrapper *ew = createEntityWrapper(L, 2, &father);

	ew->e = yeCreateCopy(val, father, name);
	return 1;
}

int	luaentity_newstring(lua_State *L)
{
	const char *val = luaL_checkstring(L, 1);
	const char *name = lua_tostring(L, 3);
	Entity *father = NULL;
	struct entityWrapper *ew = createEntityWrapper(L, 2, &father);

	ew->e = yeCreateString(val, father, name);
	return 1;
}

int	luaentity_newfunc(lua_State *L)
{
	const char *name = lua_tostring(L, 3);
	Entity *father = NULL;
	int is_func = lua_isfunction(L, 1);
	struct entityWrapper *ew = is_func ? NULL :
		createEntityWrapper(L, 2, &father);
	const char *val = is_func ? NULL : luaL_checkstring(L, 1);
	if (is_func) {
		int f_ref;
		father = luaEntityAt(L, 2);

		lua_settop(L, 1);
		f_ref = luaL_ref(L, LUA_REGISTRYINDEX);
		ew = lua_newuserdata(L, sizeof(struct entityWrapper));
		ew->needDestroy = father == NULL ? 1 : 0;
		luaL_getmetatable(L, "Entity");
		lua_setmetatable(L, -2);
		ew->e = yeCreateFunctionExt(NULL,
					    ygGetLuaManager(),
					    father, name,
					    YE_FUNC_NO_FASTPATH_INIT);
		YE_TO_FUNC(ew->e)->idata = f_ref;
	} else if (!name) {
		ew->e = yeCreateFunctionSimple(val, ygGetLuaManager(), father);
	} else {
		ew->e = yeCreateFunction(val, ygGetLuaManager(), father, name);
	}
	return 1;
}

int	luaentity_newint(lua_State *L)
{
  int val = luaL_checknumber(L, 1);
  const char *name = lua_tostring(L, 3);
  Entity *father = NULL;
  struct entityWrapper *ew = createEntityWrapper(L, 2, &father);

  ew->e = yeCreateInt(val, father, name);
  return 1;
}

int	luaentity_newfloat(lua_State *L)
{
  int val = luaL_checknumber(L, 1);
  const char *name = lua_tostring(L, 3);
  Entity *father = NULL;
  struct entityWrapper *ew = createEntityWrapper(L, 2, &father);

  ew->e = yeCreateFloat(val, father, name);
  return 1;
}

int	luayIsLightUserData(lua_State *L)
{
  lua_pushboolean(L, lua_islightuserdata(L, 1));
  return 1;
}

int	luayOr(lua_State *L)
{
  lua_pushnumber(L,
		 (int_ptr_t)luaNumberAt(L, 1) | (int_ptr_t)luaNumberAt(L, 2));
  return (1);
}

int	luaYAnd(lua_State *L)
{
  lua_pushnumber(L,
		 (int_ptr_t)luaNumberAt(L, 1) & (int_ptr_t)luaNumberAt(L, 2));
  return (1);
}

#define luaIsNil_()				\
	if (lua_islightuserdata(l, 1))		\
		isNil = !lua_touserdata(l, 1);	\
	else if (lua_isnumber(l, 1))		\
		isNil = !lua_tonumber(l, 1);	\
	else					\
		isNil = lua_isnil(l, 1);	\


int	luayIsNNil(lua_State *l)
{
	int isNil;

	luaIsNil_();
	lua_pushboolean(l, !isNil);
	return 1;
}

int	luayIsNil(lua_State *l)
{
	int isNil;

	luaIsNil_();
	lua_pushboolean(l, isNil);
	return 1;
}

int	luayeGet(lua_State *L)
{
	if (lua_isnumber(L, 2)) {
		lua_pushlightuserdata(L, yeGetByIdx(luaEntityAt(L, 1),
						    lua_tonumber (L, 2)));
		return 1;
	} else if (lua_isstring(L, 2)) {
		lua_pushlightuserdata(L, yeGetByStrFast(luaEntityAt(L, 1),
							lua_tostring (L, 2)));
		return 1;
	}
	return 0;
}

int	luaywCanvasRotate(lua_State *L)
{
  lua_pushnumber(L, ywCanvasRotate(luaEntityAt(L, 1),
				   lua_tonumber(L, 2)));
  return 1;
}

int	luaywCanvasSwapObj(lua_State *L)
{
  lua_pushnumber(L, ywCanvasSwapObj(luaEntityAt(L, 1),
				    luaEntityAt(L, 2),
				    luaEntityAt(L, 3)));
  return 1;
}

int	luaywCanvasObjIsOut(lua_State *L)
{
  lua_pushnumber(L, ywCanvasObjIsOut(luaEntityAt(L, 1),
				     luaEntityAt(L, 2)));
  return 1;
}

int	luaywCanvasObjPointTopTo(lua_State *L)
{
  ywCanvasObjPointTopTo(luaEntityAt(L, 1), luaEntityAt(L, 2));
  return 0;
}

int	luaywCanvasObjPointRightTo(lua_State *L)
{
  ywCanvasObjPointRightTo(luaEntityAt(L, 1), luaEntityAt(L, 2));
  return 0;
}

int	luaYwCanvasMoveObjByIdx(lua_State *L)
{
  lua_pushnumber(L, ywCanvasMoveObjByIdx(luaEntityAt(L, 1),
					 lua_tonumber(L, 2),
					 luaEntityAt(L, 3)));
  return 1;
}

int	luaywCanvasObjAngle(lua_State *L)
{
  lua_pushnumber(L, ywCanvasObjAngle(luaEntityAt(L, 1)));
  return 1;
}

int	luaywCanvasAdvenceObj(lua_State *L)
{
  lua_pushnumber(L, ywCanvasAdvenceObj(luaEntityAt(L, 1),
				       lua_tonumber(L, 2),
				       lua_tonumber(L, 3)));
  return 1;
}

int	luaywCanvasMoveObj(lua_State *L)
{
  lua_pushnumber(L, ywCanvasMoveObj(luaEntityAt(L, 1),
				    luaEntityAt(L, 2)));
  return 1;
}

int	luaYwCanvasObjSetResourceId(lua_State *L)
{
  ywCanvasObjSetResourceId(luaEntityAt(L, 1), lua_tonumber(L, 2));
  return 0;
}

int	luaywCanvasObjClearCache(lua_State *L)
{
  ywCanvasObjClearCache(luaEntityAt(L, 1));
  return 0;
}

int	luaywCanvasNewImg(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasNewImg(luaEntityAt(L, 1),
					  lua_tonumber(L, 2),
					  lua_tonumber(L, 3),
					  lua_tostring(L, 4),
					  luaEntityAt(L, 5)));
  return 1;
}

int	luaywCanvasNewImgFromTexture(lua_State *L);

int	luaYwCanvasNewObj(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasNewObj(luaEntityAt(L, 1),
					  lua_tonumber(L, 2),
					  lua_tonumber(L, 3),
					  lua_tonumber(L, 4)));
  return 1;
}

int	luaYwCanvasObjSize(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasObjSize(luaEntityAt(L, 1),
					   luaEntityAt(L, 2)));
  return 1;
}

int	luaYwCanvasObjPos(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasObjPos(luaEntityAt(L, 1)));
  return 1;
}

int luaYwCanvasObjFromIdx(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasObjFromIdx(luaEntityAt(L, 1),
					      lua_tonumber(L, 2)));
  return 1;
}

int luaYwCanvasIdxFromObj(lua_State *L)
{
  lua_pushnumber(L, ywCanvasIdxFromObj(luaEntityAt(L, 1),
				       luaEntityAt(L, 2)));
  return 1;
}

int luaYwCanvasObjSetPos(lua_State *L)
{
  if (lua_isuserdata(L, 2)) {
    ywCanvasObjSetPosByEntity(luaEntityAt(L, 1), lua_touserdata(L, 2));
  } else {
    ywCanvasObjSetPos(luaEntityAt(L, 1), lua_tonumber(L, 2),
		      lua_tonumber(L, 3));
  }
  return 0;
}

int	luaYwCanvasRemoveObj(lua_State *L)
{
  ywCanvasRemoveObj(luaEntityAt(L, 1), luaEntityAt(L, 2));
  return 0;
}

int	luaywCanvasObjectsCheckColisions(lua_State *L)
{
  lua_pushboolean(L, ywCanvasObjectsCheckColisions(luaEntityAt(L, 1),
						   luaEntityAt(L, 2)));
  return 1;
}

int	luaywCanvasNewCollisionsArrayExt(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasNewCollisionsArrayExt(luaEntityAt(L, 1),
							 luaEntityAt(L, 2),
							 luaEntityAt(L, 3),
							 luaEntityAt(L, 4)));
  return 1;

}

int luaYwCanvasNewCollisionsArray(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasNewCollisionsArray(luaEntityAt(L, 1),
						     luaEntityAt(L, 2)));
  return 1;
}

int luaYwCanvasNewCollisionsArrayWithRectangle(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasNewCollisionsArrayWithRectangle(luaEntityAt(L, 1),
						     luaEntityAt(L, 2)));
  return 1;
}

int	luaywCanvasNewTextExt(lua_State *L)
{
  lua_pushlightuserdata(L, ywCanvasNewTextExt(luaEntityAt(L, 1),
					      lua_tonumber(L, 2),
					      lua_tonumber(L, 3),
					      luaEntityAt(L, 4),
					      lua_tostring(L, 5)));
  return 1;
}


int luaYwCanvasNewText(lua_State *L)
{
	if (lua_isstring(L, 4)) {
		lua_pushlightuserdata(
			L, ywCanvasNewTextByStr(luaEntityAt(L, 1),
						lua_tonumber(L, 2),
						lua_tonumber(L, 3),
						lua_tostring(L, 4)));

	} else {
		lua_pushlightuserdata(
			L, ywCanvasNewText(luaEntityAt(L, 1),
					   lua_tonumber(L, 2),
					   lua_tonumber(L, 3),
					   luaEntityAt(L, 4)));
	}
	return 1;
}

int	luaywCanvasPopObj(lua_State *L)
{
	ywCanvasPopObj(luaEntityAt(L, 1));
	return 0;
}

int	luaywTextureNormalize(lua_State *L)
{
	lua_pushnumber(L, ywTextureNormalize(luaEntityAt(L, 1)));
	return 1;
}

int	luaywCanvasSetWeight(lua_State *L)
{
	lua_pushnumber(L, ywCanvasSetWeight(luaEntityAt(L, 1),
					    luaEntityAt(L, 2),
					    lua_tonumber(L, 3)));
	return 1;
}

int	luaywTextureMerge(lua_State *L)
{
	lua_pushnumber(L, ywTextureMerge(luaEntityAt(L, 1),
					 luaEntityAt(L, 2),
					 luaEntityAt(L, 3),
					 luaEntityAt(L, 4)));
	return 1;
}

int	luaYeIncrRef(lua_State *L)
{
	yeIncrRef(luaEntityAt(L, 1));
	return 0;
}

int	luaYeToLuaString(lua_State *L)
{
	char *str = yeToCStr(luaEntityAt(L, 1), 4, 0);

	lua_pushstring(L, str);
	free(str);
	return 1;
}

int	luaywidTurnTimer(lua_State *L)
{
	lua_pushnumber(L, ywidTurnTimer);
	return 1;
}

int	luaSetMainWid(lua_State *L)
{
	ywidSetMainWid(lua_touserdata(L, 1));
	return 0;
}

int	luaAddSubType(lua_State *L)
{
	ywidAddSubType(luaEntityAt(L, 1));
	return 0;
}


int	luaNewWidget(lua_State *L)
{
	lua_pushlightuserdata(L, ywidNewWidget(luaEntityAt(L, 1),
					       lua_tostring(L, 2)));
	return 1;
}

int	luaySoundPlayLoop(lua_State *L)
{
	lua_pushnumber(L, ySoundPlayLoop(luaNumberAt(L, 1)));
	return 1;
}

int	luaywMenuCallActionOn(lua_State *L)
{
	lua_pushnumber(L, ywMenuCallActionOn(luaEntityAt(L, 1),
					     luaEntityAt(L, 2),
					     lua_tonumber(L, 3)));
	return 1;
}

int	luaywMenuGetCurrent(lua_State *L)
{
	lua_pushnumber(L, ywMenuGetCurrentByEntity(luaEntityAt(L, 1)));
	return 1;
}

int	luaywMenuGetCurrentEntry(lua_State *L)
{
	lua_pushlightuserdata(L, ywMenuGetCurrentEntry(luaEntityAt(L, 1)));
	return 1;
}

int	luaywMenuMove(lua_State *L)
{
	lua_pushlightuserdata(L, ywMenuMove(luaEntityAt(L, 1),
					    lua_tonumber(L, 2)));
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
	int is_func = lua_isfunction(L, 1);

	if (is_func) {
		Entity *f = luaEntityAt(L, 2);
		const char *name = lua_tostring(L, 3);
		int f_ref;

		lua_settop(L, 1);
		/*lua array starting at 1, for the first time of my whole life*/
		/* I see an adventage of having array starting at one */
		f_ref = luaL_ref(L, LUA_REGISTRYINDEX);
		ret = yeCreateFunctionExt(NULL,
					  ygGetLuaManager(),
					  f, name,
					  YE_FUNC_NO_FASTPATH_INIT);
		YE_TO_FUNC(ret)->idata = f_ref;
	} else if (!lua_tostring(L, 3)) {
		ret = yeCreateFunctionSimple(lua_tostring(L, 1),
					     ygGetLuaManager(),
					     luaEntityAt(L, 2));
	} else {
		ret = yeCreateFunction(lua_tostring(L, 1),
				       ygGetLuaManager(), luaEntityAt(L, 2),
				       lua_tostring(L, 3));
	}

	lua_pushlightuserdata(L, ret);
	return 1;
}

int	luaWidNextEve(lua_State *L)
{
	lua_pushlightuserdata(L, ywidNextEve(luaEntityAt(L, 1)));
	return 1;
}

int	luaWidNext(lua_State *L)
{
	lua_pushnumber(L, ywidNext(luaEntityAt(L, 1), luaEntityAt(L, 2)));
	return 1;
}

int	luayevMouseDown(lua_State *L)
{
	int ret2 = 0;
	lua_pushboolean(L, yevMouseDown(luaEntityAt(L, 1), &ret2));
	lua_pushnumber(L, ret2);
	return 2;
}

int	luayevCreateGrp(lua_State *L)
{
  switch (lua_gettop(L)) {
  case 2:
    lua_pushlightuserdata(L, yevCreateGrp(luaEntityAt(L, 1),
					  (uint64_t)luaNumberAt(L, 2)));
    break;
  case 3:
    lua_pushlightuserdata(L, yevCreateGrp(luaEntityAt(L, 1),
					  (uint64_t)luaNumberAt(L, 2),
					  (uint64_t)luaNumberAt(L, 3)));
    break;
  case 4:
    lua_pushlightuserdata(L, yevCreateGrp(luaEntityAt(L, 1),
					  (uint64_t)luaNumberAt(L, 2),
					  (uint64_t)luaNumberAt(L, 3),
					  (uint64_t)luaNumberAt(L, 4)));
    break;
default:
    DPRINT_ERR("internal error: wrong number of argument");
    return 0;
  }
  return 1;
}

int	luayevCheckKeys(lua_State *L)
{
	switch (lua_gettop(L)) {
	case 3:
		lua_pushboolean(L, yevCheckKeys(luaEntityAt(L, 1),
						luaNumberAt(L, 2),
						luaNumberAt(L, 3)));
		break;
	case 4:
		lua_pushboolean(L, yevCheckKeys(luaEntityAt(L, 1),
						luaNumberAt(L, 2),
						luaNumberAt(L, 3),
						luaNumberAt(L, 4)));
		break;
	case 5:
		lua_pushboolean(L, yevCheckKeys(luaEntityAt(L, 1),
						luaNumberAt(L, 2),
						luaNumberAt(L, 3),
						luaNumberAt(L, 4),
						luaNumberAt(L, 5)));
		break;
	case 6:
		lua_pushboolean(L, yevCheckKeys(luaEntityAt(L, 1),
						luaNumberAt(L, 2),
						luaNumberAt(L, 3),
						luaNumberAt(L, 4),
						luaNumberAt(L, 5),
						luaNumberAt(L, 6)));
		break;

	default:
		DPRINT_ERR("internal error: wrong number of argument");
		return 0;
	}
	return 1;
}

int	luaWidEveIsEnd(lua_State *L)
{
	lua_pushboolean(L, luaEntityAt(L, 1) == NULL);
	return 1;
}

int	luaEveType(lua_State *L)
{
	lua_pushnumber(L, ywidEveType(luaEntityAt(L, 1)));
	return 1;
}

int	luaEveKey(lua_State *L)
{
	lua_pushnumber(L, ywidEveKey(luaEntityAt(L, 1)));
	return 1;
}

int	luayeStrCaseCmp(lua_State *L)
{
	lua_pushnumber(L, yeStrCaseCmp(luaEntityAt(L, 1), lua_tostring(L, 2)));
	return 1;
}

int	luayeToLower(lua_State *L)
{
	lua_pushlightuserdata(L, yeToLower(luaEntityAt(L, 1)));
	return 1;
}

int	luayeGetBoolAt(lua_State *L)
{
	luayeGet(L);
	lua_pushboolean(L, yeGetInt(lua_touserdata(L, lua_gettop(L))));
	return 1;
}

int	luayeRemoveChild(lua_State *L)
{
	if (lua_isnumber(L, 2)) {
		lua_pushlightuserdata(
			L, yeRemoveChild(luaEntityAt(L, 1),
					 lua_tonumber(L, 2)));
	} else if (lua_isstring(L, 2)) {
		lua_pushlightuserdata(
			L, yeRemoveChild(luaEntityAt(L, 1),
					 lua_tostring(L, 2)));
	} else {
		lua_pushlightuserdata(L, yeRemoveChild(luaEntityAt(L, 1),
						       luaEntityAt(L, 2)));
	}
	return 1;
}

int	luaYwMapGetIdByElem(lua_State *L)
{
	lua_pushnumber(L, ywMapGetIdByElem(luaEntityAt(L, 1)));
	return 1;
}

int	luaYwMapGetResourceId(lua_State *L)
{
	lua_pushnumber(L, ywMapGetResourceId(luaEntityAt(L, 1),
					     luaEntityAt(L, 2)));
	return 1;
}

int	luaYwMapGetResource(lua_State *L)
{
	lua_pushlightuserdata(L, ywMapGetResource(luaEntityAt(L, 1),
						  luaEntityAt(L, 2)));
	return 1;
}

int	luaYwMapGetCase(lua_State *L)
{
	lua_pushlightuserdata(L, ywMapGetCase(luaEntityAt(L, 1),
					      luaEntityAt(L, 2)));
	return 1;
}

int	luaYwMapGetEntityById(lua_State *L)
{
	lua_pushlightuserdata(L, ywMapGetEntityById(luaEntityAt(L, 1),
						    luaEntityAt(L, 2),
						    lua_tonumber(L, 3)));
	return 1;
}

int	luaYMapAdvence(lua_State *L)
{
	if (lua_isnumber(L, 3)) {
		lua_pushnumber(L, ywMapAdvenceWithPos(luaEntityAt(L, 1),
						      luaEntityAt(L, 2),
						      lua_tonumber(L, 3),
						      lua_tonumber(L, 4),
						      luaEntityAt(L, 5)));
	} else {
		lua_pushnumber(L, ywMapAdvenceWithEPos(luaEntityAt(L, 1),
						       luaEntityAt(L, 2),
						       luaEntityAt(L, 3),
						       luaEntityAt(L, 4)));
	}
	return 1;
}

int	luaYwReplaceEntry(lua_State *L)
{
	lua_pushnumber(L, ywReplaceEntry(luaEntityAt(L, 1),
					 lua_tonumber(L, 2),
					 luaEntityAt(L, 3))
		);
	return 1;
}

int	luaYwCntGetEntry(lua_State *L)
{
	lua_pushlightuserdata(L, ywCntGetEntry(luaEntityAt(L, 1),
					       lua_tonumber(L, 2)));
	return 1;
}

int	luaywCntConstructChilds(lua_State *L)
{
	ywCntConstructChilds(luaEntityAt(L, 1));
	return 0;
}

int	luaywidAction(lua_State *L)
{
	lua_pushnumber(L, ywidAction(luaEntityAt(L, 1),
				     luaEntityAt(L, 2),
				     luaEntityAt(L, 3)));
	return 1;
}

int	luaYwCntPopLastEntry(lua_State *L)
{
	ywCntPopLastEntry(luaEntityAt(L, 1));
	return 0;
}

int	luaYwPushNewWidget(lua_State *L)
{
	lua_pushnumber(L, ywPushNewWidget(luaEntityAt(L, 1),
					  luaEntityAt(L, 2),
					  lua_tonumber(L, 3)));
	return 1;
}

int	luaywPosAngle(lua_State *L)
{
	lua_pushnumber(L, ywPosAngle(luaEntityAt(L, 1),
				     luaEntityAt(L, 2)));
	return 1;
}

int	luaywRectW(lua_State *L)
{
	lua_pushnumber(L, ywRectW(luaEntityAt(L, 1)));
	return 1;
}

int	luaywRectH(lua_State *L)
{
	lua_pushnumber(L, ywRectH(luaEntityAt(L, 1)));
	return 1;
}


int	luaYwRectCreate(lua_State *L)
{
	if (lua_isnumber(L, 1)) {
		lua_pushlightuserdata(L, ywRectCreateInts(lua_tonumber(L, 1),
							  lua_tonumber(L, 2),
							  lua_tonumber(L, 3),
							  lua_tonumber(L, 4),
							  luaEntityAt(L, 5),
							  lua_tostring(L, 6)));
	} else if (lua_gettop(L) == 3) {
		lua_pushlightuserdata(L, ywRectCreateEnt(luaEntityAt(L, 1),
							 luaEntityAt(L, 2),
							 lua_tostring(L, 3)));
	} else {
		lua_pushlightuserdata(
			L, ywRectCreatePosSize(luaEntityAt(L, 1),
					       luaEntityAt(L, 2),
					       luaEntityAt(L, 3),
					       lua_tostring(L, 4)));
	}
	return 1;
}

int	luaywRectCollision(lua_State *L)
{
  lua_pushboolean(L, ywRectCollision(luaEntityAt(L, 1),
				     luaEntityAt(L, 2)));
  return 1;
}

int	luaywPosCreate(lua_State *L)
{
	if (lua_isnumber(L, 1)) {
		lua_pushlightuserdata(L, ywPosCreate(lua_tonumber(L, 1),
						     lua_tonumber(L, 2),
						     luaEntityAt(L, 3),
						     lua_tostring(L, 4)));
	} else {
		lua_pushlightuserdata(L, ywPosCreate(luaEntityAt(L, 1),
						     0, luaEntityAt(L, 2),
						     lua_tostring(L, 3)));
	}
	return 1;
}

int	luaywPosSet(lua_State *L)
{
  if (lua_isnumber(L, 2)) {
    lua_pushlightuserdata(L, ywPosSet(luaEntityAt(L, 1), lua_tonumber(L, 2),
				      lua_tonumber(L, 3)));
  } else {
    lua_pushlightuserdata(L, ywPosSet(luaEntityAt(L, 1),
				      luaEntityAt(L, 2), 0));
  }
  return 1;
}

int	luaywPosIsSame(lua_State *L)
{
  if (lua_isnumber(L, 2))
    lua_pushboolean(L, ywPosIsSame(luaEntityAt(L, 1),
				      lua_tonumber(L, 2), lua_tonumber(L, 3)));
  else
    lua_pushboolean(L, ywPosIsSame(luaEntityAt(L, 1),
				      luaEntityAt(L, 2), 0));
  return 1;
}

int	luaywPosIsSameX(lua_State *L)
{
  if (lua_isnumber(L, 2))
    lua_pushboolean(L, ywPosIsSameX(luaEntityAt(L, 1),
				       lua_tonumber(L, 2)));
  else
    lua_pushboolean(L, ywPosIsSameX(luaEntityAt(L, 1),
				       luaEntityAt(L, 2)));
  return 1;
}

int	luaywPosIsSameY(lua_State *L)
{
  if (lua_isnumber(L, 2))
    lua_pushboolean(L, ywPosIsSameY(luaEntityAt(L, 1),
				       lua_tonumber(L, 2)));
  else
    lua_pushboolean(L, ywPosIsSameY(luaEntityAt(L, 1),
				       luaEntityAt(L, 2)));
  return 1;
}

int	luaywPosAdd(lua_State *L)
{
  if (lua_isnumber(L, 2)) {
    lua_pushlightuserdata(L, ywPosAddXY(luaEntityAt(L, 1),
					lua_tonumber(L, 2),
					lua_tonumber(L, 3)));
  } else {
    lua_pushlightuserdata(L, ywPosAdd(luaEntityAt(L, 1),
				      luaEntityAt(L, 2)));
  }
  return 1;
}

int	luaywPosMoveToward2(lua_State *L)
{
  lua_pushboolean(L, ywPosMoveToward2(luaEntityAt(L, 1),
				      luaEntityAt(L, 2),
				      lua_tonumber(L, 3),
				      lua_tonumber(L, 4)));
  return 1;
}

int	luaYwMapMove(lua_State *L)
{
  if (lua_isstring(L, 4))
    lua_pushnumber(L, ywMapMoveByStr(luaEntityAt(L, 1),
				     luaEntityAt(L, 2),
				     luaEntityAt(L, 3),
				     lua_tostring(L, 4)));
  else
    lua_pushnumber(L, ywMapMoveByEntity(luaEntityAt(L, 1),
					luaEntityAt(L, 2),
					luaEntityAt(L, 3),
					luaEntityAt(L, 4)));
  return 1;
}

int	luaYwMapSmootMove(lua_State *L)
{
  lua_pushnumber(L, ywMapSmootMove(luaEntityAt(L, 1),
				   luaEntityAt(L, 2),
				   luaEntityAt(L, 3),
				   luaEntityAt(L, 4)));
  return 1;
}

int	luaYwMapIntFromPos(lua_State *L)
{
  lua_pushnumber(L, ywMapIntFromPos(luaEntityAt(L, 1),
				    luaEntityAt(L, 2)));
  return 1;
}

int	luaYwMapPosFromInt(lua_State *L)
{
  lua_pushlightuserdata(L, ywMapPosFromInt(luaEntityAt(L, 1),
					   lua_tonumber(L, 2),
					   luaEntityAt(L, 3),
					   lua_tostring(L, 4)));
  return 1;
}

int	luaYwMapRemove(lua_State *L)
{
  if (lua_isstring(L, 3))
    ywMapRemove(luaEntityAt(L, 1), luaEntityAt(L, 2), lua_tostring(L, 3));
  else
    ywMapRemove(luaEntityAt(L, 1), luaEntityAt(L, 2),
		YE_TO_ENTITY(luaEntityAt(L, 3)));
  return 0;
}

int	luaYwMapW(lua_State *L)
{
  lua_pushnumber(L, ywMapW(luaEntityAt(L, 1)));
  return 1;
}

int	luaYwMapH(lua_State *L)
{
  lua_pushnumber(L, ywMapH(luaEntityAt(L, 1)));
  return 1;
}

int	luaYwMapSetSmootMovement(lua_State *L)
{
  ywMapSetSmootMovement(luaEntityAt(L, 1), lua_tonumber(L, 2));
  return 0;
}

int	luaMvTablePush(lua_State *L)
{
  lua_pushlightuserdata(L, ywMapMvTablePush(luaEntityAt(L, 1),
					    luaEntityAt(L, 2),
					    luaEntityAt(L, 3),
					    luaEntityAt(L, 4),
					    luaEntityAt(L, 5)));
  return 1;
}

int	luaYwMapPushNbr(lua_State *L)
{
  lua_pushlightuserdata(L, ywMapPushNbr(luaEntityAt(L, 1),
					lua_tonumber(L, 2),
					luaEntityAt(L, 3),
					lua_tostring(L, 4)));
  return 1;
}

int	luaGetMod(lua_State *L)
{
  lua_pushlightuserdata(L, ygGetMod(lua_tostring(L, 1)));
  return 1;
}

static inline void call_set_arg(lua_State *L, int i, union ycall_arg *args,
				int *types, int nb)
{
	if (lua_isnumber(L, i)) {
		types[nb] = YS_INT;
		args[nb].i = lua_tonumber(L, i);
	} else if (lua_isstring(L, i)) {
		types[nb] = YS_STR;
		args[nb].str = lua_tostring(L, i);
		printf("call s %s\n", args[nb].str);
	} else if (lua_isuserdata(L, i) &&
		   luaL_testudata(L, i, "Entity")) {
		types[nb] = YS_ENTITY;
		args[nb].e = luaEntityAt(L, i);
	} else {
		types[nb] = YS_VPTR;
		args[nb].vptr = lua_touserdata(L, i);
	}
}

int	luaGCall(lua_State *L)
{
	int ltop = lua_gettop(L);
	int nb = ltop - 2;
	const char *mod = lua_tostring(L, 1);
	const char *f = lua_tostring(L, 2);
	union ycall_arg args[nb];
	int types[nb];

	if (unlikely(nb < 0)) {
		DPRINT_ERR("not enough args");
		return 0;
	}

	for (int i = 0; i < nb; ++i) {
		call_set_arg(L, i + 3, args, types, i);
	}

	lua_pushlightuserdata(L, ygCallInt(mod, f, nb, args, types));
	return 1;
}

int	luaYesCall(lua_State *L)
{
	int ltop = lua_gettop(L);
	int nb = ltop - 1;
	Entity *f = luaEntityAt(L, 1);
	union ycall_arg args[nb];
	int types[nb];

	for (int i = 0; i < nb; ++i) {
		call_set_arg(L, i + 2, args, types, i);
	}
	lua_pushlightuserdata(L, yesCallInt(f, nb, args, types));
	return 1;
}

#undef LUA_YES_CALL
#undef LUA_YG_CALL

int	luaYGet(lua_State *L)
{
  if (lua_isstring(L, 1))
    lua_pushlightuserdata(L, ygGet(lua_tostring(L, 1)));
  else if (lua_islightuserdata(L, 1))
    lua_pushlightuserdata(L, ygGet(yeGetString(luaEntityAt(L, 1))));
  return 1;
}

int	luaYgRegistreFunc(lua_State *L)
{
  lua_pushnumber(L, ygRegistreFuncInternal(ygGetLuaManager(),
					   lua_tonumber(L, 1),
					   lua_tostring(L, 2),
					   lua_tostring(L, 3)));
  return 1;
}

int	luaYgFileToEnt(lua_State *L)
{
  lua_pushlightuserdata(L, ygFileToEnt(lua_tonumber(L, 1),
				       lua_tostring(L, 2),
				       luaEntityAt(L, 3)));
  return 1;
}

int	luaYgEntToFile(lua_State *L)
{
  lua_pushnumber(L, ygEntToFile(lua_tonumber(L, 1),
				lua_tostring(L, 2),
				luaEntityAt(L, 3)));
  return 1;
}

int	luaygStalk(lua_State *L)
{
  lua_pushboolean(L, !ygStalk(lua_tostring(L, 1), luaEntityAt(L, 2),
			      luaEntityAt(L, 3)));
  return 1;
}

int	luaygUnstalk(lua_State *L)
{
  lua_pushboolean(L, !ygUnstalk(lua_tostring(L, 1)));
  return 1;
}

int	luaygReCreateInt(lua_State *L)
{
  ygReCreateInt(lua_tostring(L, 1), lua_tonumber(L, 2));
  return 0;
}

int	luayeSetAt(lua_State *L)
{
  Entity *ent = NULL;
  if (lua_gettop(L) != 3)
    return 0;

  if (lua_isnumber(L, 2)) {
    ent = yeGet(luaEntityAt(L, 1), (int64_t)lua_tonumber(L, 2));
  } else if (lua_isstring(L, 2)) {
    ent = yeGet(luaEntityAt(L, 1), lua_tostring(L, 2));
  } else {
    return 0;
  }

  if (lua_isnumber(L, 3)) {
    yeSetInt(ent, lua_tonumber(L, 3));
  } else if (lua_isstring(L, 3)) {
    yeSetString(ent, lua_tostring(L, 3));
  }
  return 0;
}

int	luayeSetIntAt(lua_State *L)
{
	return luayeSetAt(L);
}

int	luayeCheckCondition(lua_State *L)
{
	lua_pushboolean(L, yeCheckCondition(luaEntityAt(L, 1)));
	return 1;
}

int	luaYTimerGet(lua_State *L)
{
	lua_pushnumber(L, YTimerGet(lua_touserdata(L, 1)));
	return 1;
}

int	luaYTimerReset(lua_State *L)
{
	YTimerReset(lua_touserdata(L, 1));
	return 0;
}

int	luaYTimerCreate(lua_State *L)
{
	lua_pushlightuserdata(L, YTimerCreate());
	return 1;
}
