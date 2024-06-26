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

#include "entity-script.h"

void *yesCallInt(Entity *func, int nb, union ycall_arg *args,
		 int *types)
{
	void *fp;
	void *manager;

	if (unlikely(!func))
		return NULL;

	manager = YE_TO_FUNC(func)->manager;
	if (ysHasEntityCall(manager))
		return ysEntityCall(manager, func, nb, args, types);

	if (unlikely(!(fp = yeGetFunctionFastPath(func)) &&
		     !yeGetFunction(func)))
		return NULL;

	if (!fp) {
		YE_TO_FUNC(func)->fastPath =
			ysGetFastPath(manager, yeGetFunction(func));
		fp = yeGetFunctionFastPath(func);
	}

	if (fp)
		return ysFastCall(manager, fp, nb, args, types);
	return ysCallInt(manager, yeGetFunction(func), nb, args, types);
}
