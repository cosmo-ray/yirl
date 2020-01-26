/*
**Copyright (C) 2020 Matthias Gatto
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

#include <glib.h>
#include "tests.h"
#include <yirl/utils.h>

void testMaths(void)
{
	int x, y;

	/* Lines */
	g_assert(!yuiLinesIntersect(0, 0, 2, 2,
				    3, 3, 5, 5, &x, &y));
	g_assert(!yuiLinesIntersect(0, 0, 2, 2,
				    3, 3, 5, 5, NULL, NULL));

	g_assert(yuiLinesIntersect(0, 0, 20, 20,
				   0, 20, 20, 0, &x, &y));

	g_assert(x == 10 && y == 10);


	/* Line x Rect */
	g_assert(yuiLinesRectIntersect(25, 25, 10, 10, 0, 0, 20, 20, &x, &y));
	g_assert(x == 20 && y == 20);
	g_assert(yuiLinesRectIntersect(25, 25, 10, 10, 0, 0, 20, 10, &x, &y));
	g_assert(x == 10 && y == 10);
	g_assert(yuiLinesRectIntersect(25, 10, 0, 10, 0, 0, 20, 20, &x, &y));
	g_assert(x == 20 && y == 10);
	g_assert(yuiLinesRectIntersect(0, 10, 25, 10, 1, 1, 20, 20, &x, &y));
	g_assert(x == 1 && y == 10);
}
