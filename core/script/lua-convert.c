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

#include "lua-convert.h"

void	*luaGetPtr(lua_State *l, int idx)
{
  if (lua_islightuserdata(l, idx))
    return lua_touserdata(l, idx);
  if (lua_isuserdata(l, idx))
    return luaEntityAt(l, idx);
  if (lua_isnumber(l, idx)) {
    return (void *)(uintptr_t)lua_tonumber(l, idx);
  }
  if (lua_isstring(l, idx)) {
    return (void *)lua_tostring(l, idx);
  }
  if (lua_isboolean(l, idx))
    return (void *)lua_toboolean(l, idx);
  return NULL;
}

int     luaToPtr(lua_State *l)
{
  if (lua_islightuserdata(l, 1))
    return 1;
  if (lua_isuserdata(l, 1))
    lua_pushlightuserdata(l, luaEntityAt(l, 1));
  else if (lua_isnumber(l, 1))
    lua_pushlightuserdata(l, (void *)(uintptr_t)lua_tonumber(l, 1));
  else if (lua_isstring(l, 1))
    lua_pushlightuserdata(l, (void *)lua_tostring(l, 1));
  else if (lua_isboolean(l, 1))
    lua_pushlightuserdata(l, (void *)lua_toboolean(l, 1));
  else
    lua_pushnil(l);
  return 1;
}

int     luaPtrToNumber(lua_State *l)
{
  if (lua_isnumber(l, 1))
    return 1;
  lua_pushnumber(l, (int_ptr_t)lua_topointer(l, 1));
  return 1;
}

int     luaPtrToInt32(lua_State *l)
{
  if (lua_isnumber(l, 1))
    return 1;
  lua_pushnumber(l, (int32_t)(int_ptr_t)lua_topointer(l, 1));
  return 1;
}

int     luaPtrToString(lua_State *l)
{
  if (lua_isstring(l, 1))
    return 1;
  lua_pushstring(l, (char *)lua_topointer(l, 1));
  return 1;
}

int     luaNbrToPtr(lua_State *l)
{
  lua_pushlightuserdata(l, (void *)(uintptr_t)lua_tointeger(l, 1));
  return 1;
}

int     luaStringToPtr(lua_State *l)
{
  lua_pushlightuserdata(l, (void *)lua_tostring(l, 1));
  return 1;
}
