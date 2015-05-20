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

static YScriptsTab scriptsTab = {
  {NULL },
  0
};

YScriptsTab *ysScriptsTab(void)
{
  return &scriptsTab;
}

void *ysNewScriptManager(void *args, int t)
{
  YScriptOps *ret;

  if (scriptsTab.len <= t || scriptsTab.allocator[t] == NULL)
    return NULL;
  ret = scriptsTab.allocator[t]();
  if (ret == NULL)
    return NULL;
  if (ret->init(ret, args)) {
    ret->destroy(ret);
    return NULL;
  }
  return ret;
}

int ysDestroyScriptManager(void *sm)
{
  if (!sm)
    return -1;
  return ((YScriptOps *)sm)->destroy(sm);
}

int ysRegister(void *(*allocator)(void))
{
  if (scriptsTab.len >= MAX_SCRIPT_LANG - 1)
    return -1;
  scriptsTab.allocator[scriptsTab.len] = allocator;
  scriptsTab.len += 1;
  return scriptsTab.len - 1;
}

int ysUnregiste(int t)
{
  if (scriptsTab.len <= t)
    return -1;
  scriptsTab.allocator[scriptsTab.len] = NULL;
  if (t == scriptsTab.len - 1)
    scriptsTab.len -= 1;
  return 0;
}

void *ysCall(void *sm, const char *name, int nbArg, ...)
{
  void *ret;
  va_list ap;

  va_start(ap, nbArg);
  ret = ysVCall(sm, name, nbArg, &ap);
  va_end(ap);
  return ret;
}

