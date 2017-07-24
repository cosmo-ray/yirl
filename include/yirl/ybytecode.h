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
	YB_VOID_RETURN = 'e',
	YB_RETURN = 'E',
	YB_BRUTAL_CAST = 127,
	JMP_IF_0 = 128,
	YB_INF_COMP_NBR = 129,
	YB_YG_GET_PUSH = 256,
	YB_INCR = 300,
};

extern char *ybytecode_error;

Entity *ybytecode_exec(Entity *stack, int64_t *script);

#endif
