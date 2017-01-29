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

#ifndef _YIRL_TCC_SCRIPT_H_
#define _YIRL_TCC_SCRIPT_H_

#include "libtcc.h"
#include "script.h"

typedef struct {
  YScriptOps ops;
  TCCState *l;
  int needRealloc;
} YTccScript;


int ysTccInit(void);
int ysTccEnd(void);
int ysTccGetType(void);

#endif
