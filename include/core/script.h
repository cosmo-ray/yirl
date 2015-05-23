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

#ifndef MAX_SCRIPT_LANG
#define MAX_SCRIPT_LANG 64
#endif

typedef struct {
  void *(*allocator[MAX_SCRIPT_LANG])(void);
  int len;
} YScriptsTab;


typedef struct {
  int (*init)(void *opac, void *args);
  int (*loadFile)(void *opac, char *);
  int (* registreFunc)(void *opac, char *name, void *arg);
  /* void *(*vCall)(void *opac, const char *name, int nbArg, va_list *ap); */
  void *(*call)(void *opac, const char *name, int nbArg, va_list *ap);
  int (*destroy)(void *opac);
} YScriptOps;

YScriptsTab *ysScriptsTab(void);

void *ysCall(void *sm, const char *name, int nbArg, ...);

static inline void *ysVCall(void *sm, const char *name, int nbArg, va_list *ap)
{
  return ((YScriptOps *)sm)->call(sm, name, nbArg, ap);
}

static inline int ysRegistreFunc(void *sm, char *name, void *arg)
{
  return ((YScriptOps *)sm)->registreFunc(sm, name, arg);
}

/**
 * registre a new type in scriptsTab
 */
int ysRegister(void *(*allocator)(void));

int ysUnregiste(int t);

/**
 * @args the arguments 
 * @type the type of script
 */
void *ysNewScriptManager(void *args, int type);

/**
 * @scr the opaque scriptionManager object 
 */
int ysDestroyScriptManager(void *sm);

#endif
