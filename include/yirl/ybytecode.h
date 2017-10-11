/*
**Copyright (C) 2016 Matthias Gatto
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

#ifndef _YIRL_BYTECODE_H_
#define _YIRL_BYTECODE_H_

#include "entity.h"

enum ybutecode_ops {
	YB_LEAVE = 'e',
	YB_RETURN = 'E',
	YB_ADD = '+',
	YB_SUB = '-',
	YB_DIV = '/',
	YB_MULT = '*',
	YB_INF = '<',
	YB_SUP = '>',
	YB_JMP = 'j',
	YB_CREATE_STRING = 's',
	YB_CREATE_INT = 'i',
	YB_CREATE_ARRAY = 'a',
	YB_SET_INT = 'I',
	YB_COMPILLE_FUNC = 'F',
	YB_CALL = 'c',
	YB_BRUTAL_CAST = 127,
	YB_JMP_IF_0 = 128,
	YB_INF_COMP_NBR = 129,
	YB_SUP_COMP_NBR = 130,
	YB_EQUAL_COMP_NBR = 131,
	YB_EQUAL = 132,
	YB_NOT_EQUAL_COMP_NBR = 133,
	YB_YG_GET_PUSH = 256,
	YB_PUSH_BACK = 257,
	YB_GET_AT_IDX = 258,
	YB_GET_AT_STR = 259,
	YB_INCR = 300,
	YB_PRINT_ENTITY = 400,
	YB_PRINT_POS = 401,
	YB_WID_ADD_SUBTYPE = 410,
	YB_NEW_WID = 411
};

extern char *ybytecode_error;

Entity *ybytecode_exec(Entity *stack, int64_t *script);

#endif
