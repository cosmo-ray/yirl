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

int     luaPtrToNumber(lua_State *l)
{
  lua_pushnumber(l, (long)lua_topointer(l, 1));
  return 1;
}

int     luaPtrToString(lua_State *l)
{
  lua_pushstring(l, (char *)lua_topointer(l, 1));
  return 1;
}

int     luaNbrToPtr(lua_State *l)
{
  lua_pushlightuserdata(l, (void *)(uintptr_t)lua_tointeger(l, 1));
  return 1;
}
