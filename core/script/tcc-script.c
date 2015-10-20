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
#include <glib.h>
#include <libtcc.h>

#include "tcc-script.h"
#include "debug.h"

static int t = -1;

#define GET_OPS(sm) (((YTccScript *)(sm))->ops)
#define GET_TCC_S(sm) (((YTccScript *)(sm))->l)

static int tccInit(void *sm, void *args)
{
  TCCState *l;

  (void)args;
  l = tcc_new();
  if (l == NULL)
    return -1;
  GET_TCC_S(sm) = l;
  return 0;
}

static int tccLoadFile(void *sm, const char *filename)
{
  return tcc_add_file(GET_TCC_S(sm), filename);
}

static int tccRegistreFunc(void *sm, const char *name, void *arg)
{
  tcc_add_symbol(GET_TCC_S(sm), name, arg);
  return 0;
}

/* static void tccPrintError(void *sm) */
/* { */
/*   DPRINT_ERR("error in lua script\nerror: %s\n", lua_tostring(GET_TCC_S(sm), -1)); */
/* } */

static void *tccCall(void *sm, const char *name, int nbArg, va_list *ap)
{
  TCCState *tcc_s = GET_TCC_S(sm);
  void *sym;

  if ((sym = tcc_get_symbol(tcc_s, name)) == NULL)
    return NULL;
  return ((void *(*)(int, va_list *ap))sym)(nbArg, ap);
}

static int tccDestroy(void *sm)
{
  tcc_delete(GET_TCC_S(sm));
  g_free(sm);
  return 0;
}

static void *tccAllocator(void)
{
  YTccScript *ret;
  
  ret = g_new(YTccScript, 1);
  if (ret == NULL)
    return NULL;
  ret->l = NULL;
  ret->ops.init = tccInit;
  ret->ops.destroy = tccDestroy;
  ret->ops.loadFile = tccLoadFile;
  ret->ops.call = tccCall;
  ret->ops.printError = NULL;
  ret->ops.registreFunc = tccRegistreFunc;
  return (void *)ret;
}

int ysTccGetType(void)
{
  return t;
}

int ysTccInit(void)
{
  t = ysRegister(tccAllocator);
  return t;
}

int ysTccEnd(void)
{
  return ysUnregiste(t);
}

#undef GET_OPS
#undef GET_TCC_S
