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

#include <stdlib.h>
#include "script.h"

static YManagerAllocator scriptsTab = {
  {NULL },
  0
};

int ysRegister(void *(*allocator)(void))
{
  return yuiRegister(&scriptsTab, allocator);
}

int ysUnregiste(int t)
{
  return yuiUnregiste(&scriptsTab, t);
}

YManagerAllocator *ysScriptsTab(void)
{
  return &scriptsTab;
}

void *ysNewManager(void *args, int t)
{
  YScriptOps *ret;

  if (scriptsTab.len <= t || scriptsTab.allocator[t] == NULL)
    return NULL;
  ret = scriptsTab.allocator[t]();
  if (ret == NULL)
    return NULL;
  if (ret->init && ret->init(ret, args)) {
    ret->destroy(ret);
    return NULL;
  }
  return ret;
}

int ysDestroyManager(void *sm)
{
  if (!sm)
    return -1;
  return ((YScriptOps *)sm)->destroy(sm);
}

void *ysCallInt(void *sm, const char *name, int nb, union ycall_arg *args,
		int *types)
{
  	return ((YScriptOps *)sm)->call(sm, name, nb, args, types);
}

