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

#include <stdlib.h>
#include <lualib.h>
#include <lauxlib.h>

#include "lua-script.h"
#include "lua-binding.h"
#include "debug.h"
#include "entity.h"

static int t = -1;

#define GET_OPS(sm) (((YScriptLua *)sm)->ops)
#define GET_L(sm) (((YScriptLua *)sm)->l)

static int luaInit(void *sm, void *args)
{
	lua_State *l;

	(void)args;
	l = luaL_newstate();
	if (l == NULL)
		return -1;
	luaL_openlibs(l);
	GET_L(sm) = l;
	return 0;
}

static int luaLoadString(void *sm, const char *str)
{
	return luaL_dostring(GET_L(sm), str) * -1;
}

static int luaLoadFile(void *sm, const char *filename)
{
	return luaL_dofile(GET_L(sm), filename) * -1;
}

static int luaRegistreFunc(void *sm, const char *name, void *arg)
{
	lua_register(GET_L(sm), name, arg);
	return 0;
}

static const char *luaGetError(void *sm)
{
	return (lua_tostring(GET_L(sm), -1));
}


static void e_destroy(void *manager, Entity *e)
{
	int ref = yeIData(e);

	if (ref) {
		lua_State *l = GET_L(manager);

		luaL_unref(l, LUA_REGISTRYINDEX, ref);
	}
}

static inline void *do_call(lua_State *l, int nb, union ycall_arg *args,
			    int *types)
{
	for (int i = 0; i < nb; ++i) {
		int t = types[i];
		if (t == YS_INT)
			lua_pushnumber(l, args[i].i);
		else if (t == YS_STR)
			lua_pushstring(l, args[i].str);
		else
			lua_pushlightuserdata(l, args[i].vptr);
	}
	lua_call(l, nb, 1);
	if (lua_isnumber(l, lua_gettop(l)))
		return (void *)lua_tointeger(l, lua_gettop(l));
	if (lua_isuserdata(l, lua_gettop(l))) {
		void *ret = luaEntityAt(l, lua_gettop(l));
		if (ret)
			return ret;
	}
	return (void *)lua_topointer(l, lua_gettop(l));
}

static void *fastCall(void *opac, void *opacFunction,
	       int nb, union ycall_arg *args,
	       int *types)
{
	lua_State *l = GET_L(opac);
	int r = (intptr_t)opacFunction;

	lua_rawgeti(l, LUA_REGISTRYINDEX, r);

	return do_call(l, nb, args, types);
}


static void *luaCall(void *sm, const char *name, int nb,
		     union ycall_arg *args, int *types)
{
	lua_State *l = GET_L(sm);

	lua_getglobal(l, name);
	if (lua_isnil(l, -1)) {
		return NULL;
	}

	return do_call(l, nb, args, types);
}

static int luaDestroy(void *sm)
{
	lua_close(GET_L(sm));
	free((YScriptLua *)sm);
	return 0;
}

static void addFuncSymbole(void *sm, const char *name, int nbArgs, Entity *func)
{
	Entity *str = yeCreateString("function ", NULL, NULL);
	char *tmp_name;

	if (!name)
		name = yeGetString(func);
	tmp_name = y_strdup_printf("%sGlobal", name);

	lua_pushlightuserdata(((YScriptLua *)sm)->l, func);
	lua_setglobal(((YScriptLua *)sm)->l, tmp_name);

	yeAddStr(str, name);
	yeAddStr(str, "(");
	for (int i = 0; i < nbArgs; ++i) {
		if (i)
			yeAddStr(str, ", ");
		yeAddStr(str, "var");
		yeAddInt(str, i);
	}

	yeStringAdd(str, ") return yesCall(");
	yeStringAdd(str, tmp_name);

	for (int i = 0; i < nbArgs; ++i) {
		yeAddStr(str, ", yLoveToPtr(var");
		yeAddInt(str, i);
		yeAddStr(str, ")");
	}
	yeStringAdd(str, ") end");
	luaLoadString(sm, yeGetString(str));
	free(tmp_name);
	yeDestroy(str);
}

static void trace(void *sm)
{
	lua_State *L = ((YScriptLua *)sm)->l;

	luaL_traceback(L, L, "lua trace function:", 1);
	printf("%s\n", lua_tostring(L, -1));

}

static void *luaAllocator(void)
{
	YScriptLua *ret;

	ret = y_new0(YScriptLua, 1);
	if (ret == NULL)
		return NULL;
	ret->l = NULL;
	ret->ops.init = luaInit;
	ret->ops.destroy = luaDestroy;
	ret->ops.loadFile = luaLoadFile;
	ret->ops.loadString = luaLoadString;
	ret->ops.e_destroy = e_destroy;
	ret->ops.call = luaCall;
	ret->ops.fastCall = fastCall;
	ret->ops.trace = trace;
	ret->ops.getError = luaGetError;
	ret->ops.registreFunc = luaRegistreFunc;
	ret->ops.addFuncSymbole = addFuncSymbole;
	return (void *)ret;
}

int ysLuaGetType(void)
{
	return t;
}

int ysLuaInit(void)
{
	t = ysRegister(luaAllocator);
	return t;
}

int ysLuaEnd(void)
{
	return ysUnregiste(t);
}

#undef GET_OPS
#undef GET_L
