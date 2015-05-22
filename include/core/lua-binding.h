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

#ifndef _LUA_BINDING_H_
#define _LUA_BINDING_H_

#include <lualib.h>

int	luaYAnd(lua_State *L);
int	luaPrintableName(lua_State *L);
int	luaGet(lua_State *L);
int	luaLen(lua_State *L);
int	luaCopy(lua_State *L);
int	luaCreateArray(lua_State *L);
int	luaPopBack(lua_State *L);
int	luaPushBack(lua_State *L);
int	luaGetString(lua_State *L);
int	luaGetInt(lua_State *L);
int	luaGetFloat(lua_State *L);
int	luaSetFunction(lua_State *L);
int	luaSetInt(lua_State *L);
int	luaSetFloat(lua_State *L);
int	luaRemoveChild(lua_State *L);
int	luaUnsetFunction(lua_State *L);
int	luaFunctionNumberArgs(lua_State *L);
int	luaType(lua_State *L);



#endif
