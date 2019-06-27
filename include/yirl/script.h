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

#ifndef _YIRL_SCRIPT_H_
#define _YIRL_SCRIPT_H_

#include <stdarg.h>
#include "entity.h"
#include "utils.h"

#define Y_END_VA_LIST ((void *)0xDEAD0000)

typedef struct {
	int (*init)(void *opac, void *args);
	int (*loadFile)(void *opac, const char *fileName);
	int (*loadString)(void *opac, const char *str);
	int (* registreFunc)(void *opac, const char *name, void *arg);
	void (*addFuncSymbole)(void *sm, const char *name, int nbArgs,
			       Entity *func);
	void *(*call)(void *opac, const char *name, va_list ap);
	void *(*fastCall)(void *opacFunction, va_list ap);
	void *(*getFastPath)(void *scriptManager, const char *name);
	void (*e_destroy)(void *manager, Entity *e);
	int (*addDefine)(void *opac, const char *name, const char *val);
	const char *(*getError)(void *opac);
	int (*destroy)(void *opac);
} YScriptOps;

YManagerAllocator *ysScriptsTab(void);

void *ysCallInt(void *sm, const char *name, ...);

#define ysCall(sm, name, args...) ysCallInt(sm, name,			\
					    YUI_VA_ARGS_HANDELER(Y_END_VA_LIST,	\
								 args))

void *ysFCallInt(void *sm, void *name,  ...);

#define ysFCall(sm, name, args...) ysFCallInt(sm, name,			\
					      YUI_VA_ARGS_HANDELER(Y_END_VA_LIST, \
								   args))


static inline void ysEDestroy(void *sm, Entity *f)
{
	if (unlikely(!sm) || likely(!((YScriptOps *)sm)->e_destroy))
		return;

	((YScriptOps *)sm)->e_destroy(sm, f);
}

static inline int ysAddDefine(void *sm, const char *name, const char *val)
{
  if (unlikely(!((YScriptOps *)sm)->addDefine))
    return -1;
  return ((YScriptOps *)sm)->addDefine(sm, name, val);
}

static inline void *ysGetFastPath(void *sm, const char *name)
{
  if (!((YScriptOps *)sm)->getFastPath)
    return NULL;
  return ((YScriptOps *)sm)->getFastPath(sm, name);
}

static inline void *ysFastCall(void *sm, void *opacFunc, va_list ap)
{
  return ((YScriptOps *)sm)->fastCall(opacFunc, ap);
}

static inline void *ysVCall(void *sm, const char *name, va_list ap)
{
  return ((YScriptOps *)sm)->call(sm, name, ap);
}

static inline void ysAddFuncSymbole(void *sm, const char *name,
				    int nbArgs, Entity *func)
{
	return ((YScriptOps *)sm)->addFuncSymbole(sm, name, nbArgs, func);
}

static inline int ysRegistreFunc(void *sm, const char *name, void *arg)
{
  return ((YScriptOps *)sm)->registreFunc(sm, name, arg);
}

static inline int ysLoadFile(void *sm, const char *name)
{
  return ((YScriptOps *)sm)->loadFile(sm, name);
}

static inline int ysLoadString(void *sm, const char *name)
{
  return ((YScriptOps *)sm)->loadString(sm, name);
}

static inline const char *ysGetError(void *sm)
{
  if (!sm)
    return "script manager is NULL";
  if (!((YScriptOps *)sm)->getError)
    return "(nil)";
  return ((YScriptOps *)sm)->getError(sm);
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
