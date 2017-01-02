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

#include <glib.h>
#include "ybytecode.h"
#include "ybytecode-script.h"
#include "entity.h"

static int t = -1;
static void *manager;

struct YBytecodeScript {
  YScriptOps ops;
  Entity *map;
};

#define GET_MAP(sm) (((struct YBytecodeScript *)sm)->map)

static int ybytecodeRegistreFunc(void *sm, const char *name, void *arg)
{
  yeCreateData(arg, GET_MAP(sm), name);
  return 0;
}

static void *ybytecodeGetFastPath(void *sm, const char *name)
{
  return yeGetData(yeGet(GET_MAP(sm), name));
}

static void *ybytecodeFastCall(void *opacFunc, va_list ap)
{
  Entity *stack  = yeCreateArrayExt(NULL, NULL,
				    YBLOCK_ARRAY_NOINIT |
				    YBLOCK_ARRAY_NOMIDFREE);
  Entity *ret;
  int iret;
  double fret;
  void *dret;

  for (void *tmp = va_arg(ap, void *);
       tmp != Y_END_VA_LIST; tmp = va_arg(ap, void *)) {
    if (yeIsPtrAnEntity(tmp))
      yePushBack(stack, tmp, NULL);
    else
      yeCreateData(tmp, stack, NULL);
  }
  ret = ybytecode_exec(stack, opacFunc);
  yeDestroy(stack);
  switch (yeType(ret)) {
  case YINT:
    iret = yeGetInt(ret);
    yeDestroy(ret);
    return (void *)(size_t)iret;
  case YFLOAT:
    fret = yeGetFloat(ret);
    yeDestroy(ret);
    return (void *)(size_t)fret;
  case YDATA:
    dret = yeGetData(ret);
    yeDestroy(ret);
    return dret;
  default:
    break;
  }
  return ret;
}

static void *ybytecodeCall(void *sm, const char *name, va_list ap)
{
  return ybytecodeFastCall(ybytecodeGetFastPath(sm, name), ap);
}

static int ybytecodeDestroy(void *sm)
{
  if (!sm)
    return -1;
  yeDestroy(GET_MAP(sm));
  return 0;
}

static void *ybytecodeAllocator(void)
{
  struct YBytecodeScript *ybRet = g_new0(struct YBytecodeScript, 1);
  YScriptOps *ret = &ybRet->ops;

  if (ret == NULL)
    return NULL;
  ybRet->map = yeCreateArray(NULL, NULL);
  ret->call = ybytecodeCall;
  ret->getFastPath = ybytecodeGetFastPath;
  ret->fastCall = ybytecodeFastCall;
  ret->registreFunc = ybytecodeRegistreFunc;
  ret->destroy = ybytecodeDestroy;
  return (void *)ret;
}

static int ysYBytecodeInit(void)
{
  t = ysRegister(ybytecodeAllocator);
  return t;
}

void * ysYBytecodeManager(void)
{
  if (manager)
    return manager;
  ysYBytecodeInit();
  manager = ysNewManager(NULL, t);
  return manager;
}

int ysYBytecodeEnd(void)
{
  ysDestroyManager(manager);
  manager = NULL;
  return ysUnregiste(t);
}
