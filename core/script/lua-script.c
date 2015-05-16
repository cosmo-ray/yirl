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

#include <glib.h>
#include <lualib.h>
#include <lauxlib.h>

#include "lua-script.h"
#include "debug.h"
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

static int luaLoadFile(void *sm, char *filename)
{
  return luaL_dofile(GET_L(sm), filename);
}

static void *luaVNameCall(void *sm, const char *name, int nbArg, va_list *ap)
{
  lua_State *l = GET_L(sm);
  
  lua_getglobal(l, name);
  if (lua_isnil(l, -1))
    return NULL;

  Entity *tmp;

  for (int i = 0; i < nbArg; ++i)
    {
      tmp = va_arg(*ap, Entity *);
      DPRINT_INFO("pushing %p\n", tmp);
      lua_pushlightuserdata(l, tmp);
    }
  lua_call(l, nbArg, 1);
  return (Entity *)lua_touserdata(l, 0);
}

static void *luaNameCall(void *sm, const char *name, int nbArg, ...)
{
  void *ret;
  va_list ap;

  va_start(ap, nbArg);
  ret = luaVNameCall(sm, name, nbArg, &ap);
  va_end(ap);
  return ret;
}

static void *luaVCall(void *sm, Entity *func, va_list *ap)
{
  if (func == NULL)
    {
      DPRINT_ERR("entity is NULL\n");
      return NULL;
    }
  if (!yeGetFunction(func))
    return NULL;
  return luaVNameCall(sm, yeGetFunction(func), YE_TO_FUNC(func)->nArgs, ap);
}

static void *luaCall(void *sm, Entity *func, ...)
{
  void *ret;
  va_list ap;

  if (!func)
    return NULL;
  va_start(ap, func);
  ret = luaVCall(sm, func, &ap);
  va_end(ap);
  return ret;
}

static int luaDestroy(void *sm)
{
  lua_close(GET_L(sm));
  g_free((YScriptLua *)sm);
  return 0;
}

static void *luaAllocator(void)
{
  YScriptLua *ret;
  
  ret = g_new(YScriptLua, 1);
  if (ret == NULL)
    return NULL;
  ret->l = NULL;
  ret->ops.init = luaInit;
  ret->ops.destroy = luaDestroy;
  ret->ops.loadFile = luaLoadFile;
  ret->ops.vNameCall = luaVNameCall;
  ret->ops.nameCall = luaNameCall;
  ret->ops.vCall = luaVCall;
  ret->ops.call = luaCall;
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
