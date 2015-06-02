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

#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include <stdarg.h>
#include "utils.h"

typedef struct {
  int (*init)(void *opac, void *args);
  int (*loadFile)(void *opac, const char *fileName);
  int (* registreFunc)(void *opac, const char *name, void *arg);
  void *(*call)(void *opac, const char *name, int nbArg, va_list *ap);
  void (*printError)(void *opac);
  int (*destroy)(void *opac);
} YScriptOps;

YManagerAllocator *ysScriptsTab(void);


void *ysCall(void *sm, const char *name, int nbArg, ...);



static inline void *ysVCall(void *sm, const char *name, int nbArg, va_list *ap)
{
  return ((YScriptOps *)sm)->call(sm, name, nbArg, ap);
}

static inline int ysRegistreFunc(void *sm, char *name, void *arg)
{
  return ((YScriptOps *)sm)->registreFunc(sm, name, arg);
}

static inline int ysLoadFile(void *sm, const char *name)
{
  return ((YScriptOps *)sm)->loadFile(sm, name);
}

static inline void ysPrintError(void *sm)
{
  return ((YScriptOps *)sm)->printError(sm);
}

int ysRegister(void *(*allocator)(void));
int ysUnregiste(int t);


/**
 * @args the arguments 
 * @type the type of script
 */
void *ysNewManager(void *args, int type);

/**
 * @scr the opaque scriptionManager object 
 */
int ysDestroyManager(void *sm);

#endif
