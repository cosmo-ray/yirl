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

#if TCC_ENABLE > 0

#include "libtcc.h"
#include "script.h"

#define TCC_MAX_SATES 256

extern const char *ysTccPath;

typedef struct {
	YScriptOps ops;
	TCCState *l;
	TCCState *states[TCC_MAX_SATES];
	int nbStates;
	int needRealloc;
} YTccScript;

void	tccAddSyms(TCCState *l);

int ysTccInit(void);
int ysTccEnd(void);
int ysTccGetType(void);

#else /* tcc is disable */

#include "utils.h"

typedef void TCCState;

static void tccAddSyms(TCCState *l)
{
	fatal("TCC DISABLE\n");
}

static int ysTccInit(void)
{
	fatal("TCC DISABLE\n");
	return 0;
}

static int ysTccEnd(void)
{
	fatal("TCC DISABLE\n");
	return 0;	
}

static int ysTccGetType(void)
{
	fatal("TCC DISABLE\n");
	return 0;
}

#endif

#endif
