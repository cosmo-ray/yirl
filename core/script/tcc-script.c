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

#ifdef TCC_FILETYPE_C
#define tcc_add_c_file(s, filename) tcc_add_file(s, filename, TCC_FILETYPE_C)
#else
#define tcc_add_c_file(s, filename) tcc_add_file(s, filename)
#endif

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
  tcc_add_sysinclude_path(l, "/usr/include/");
  tcc_add_sysinclude_path(l, "/usr/lib/tcc/include/");
  tcc_add_sysinclude_path(l, YIRL_INCLUDE_PATH "/widget");
  tcc_add_sysinclude_path(l, YIRL_INCLUDE_PATH "/core");
  return 0;
}

static int tccLoadFile(void *sm, const char *filename)
{
  int ret = tcc_add_c_file(GET_TCC_S(sm), filename);

  if (ret < 0)
    return ret;
  return tcc_relocate(GET_TCC_S(sm), TCC_RELOCATE_AUTO);
}

static int tccRegistreFunc(void *sm, const char *name, void *arg)
{
  tcc_add_symbol(GET_TCC_S(sm), name, arg);
  return 0;
}

static void *tccCall(void *sm, const char *name, int nbArg, va_list *ap)
{
  /* should be declared as thread local */
  static void *args[16];
  TCCState *tcc_s = GET_TCC_S(sm);
  void *sym;

  if (!name) {
    DPRINT_ERR("can not call anonymous function...");
    return NULL;
  }

  if ((sym = tcc_get_symbol(tcc_s, name)) == NULL) {
    DPRINT_ERR("unable to find '%s' symbol", name);
    return NULL;
  }

  for (int i = 0; i < nbArg || i < 16; ++i) {
    args[i] = va_arg(*ap, void *);
  }
  return ((void *(*)(int, void **args))sym)(nbArg, args);
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
  ret->ops.getError = NULL;
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
