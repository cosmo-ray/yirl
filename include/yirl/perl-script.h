/*
**Copyright (C) 2023 Matthias Gatto
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

#ifndef YIRL_PERL_SCRIPT_H_
#define YIRL_PERL_SCRIPT_H_

#include "script.h"

#if PERL_ENABLE > 0

int ysPerlInit(void);
int ysPerlEnd(void);
int ysPerlGetType(void);

#else

static int ysPerlInit(void)
{
	fatal("Perl DISABLE\n");
	return 0;
}

static int ysPerlEnd(void)
{
	fatal("Perl DISABLE\n");
	return 0;
}

static int ysPerlGetType(void)
{
	fatal("Perl DISABLE\n");
	return 0;
}

#endif

#endif
