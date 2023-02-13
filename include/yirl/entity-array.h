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

#ifndef	_YIRL_ENTITY_ARRAY_H
#define	_YIRL_ENTITY_ARRAY_H

#include "game.h"

static inline Entity *yaeInt(int value, Entity *parent, const char *key)
{
	ygAssert(parent);
	ygAssert(yeCreateInt(value, parent, key));
	return parent;
}

static inline Entity *yaeString(const char *str, Entity *parent, const char *key)
{
	ygAssert(parent);
	ygAssert(yeCreateString(str, parent, key));
	return parent;
}

static inline Entity *yaeFloat(double value, Entity *parent, const char *key)
{
	ygAssert(parent);
	ygAssert(yeCreateFloat(value, parent, key));
	return parent;
}

#endif
