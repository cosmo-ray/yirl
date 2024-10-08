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

#ifndef _YIRL_ENTITY_SCRIPT_H_
#define _YIRL_ENTITY_SCRIPT_H_

#include "yirl/script.h"
#include "yirl/entity.h"

void *yesCallInt(Entity *func, int nb, union ycall_arg *args,
		 int *types);

struct ys_ret yesCall2Int(Entity *func, int nb, union ycall_arg *args,
			  int *types);

#define yesCall0(func) yesCallInt(func, 0, NULL, NULL)

#define yesCall(func, args...) yesCallInt(func,				\
					  YUI_GET_ARG_COUNT(args),	\
					  YS_ARGS(args),		\
					  YS_ATYPES(args))

#endif
