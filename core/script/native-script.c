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

#ifdef INCLUDE_HSEARCH
#include <search_hsearch_r.h>
#else
#include <search.h>
#endif

#include "native-script.h"
#include "entity.h"

static int t = -1;
static void *manager;

struct hsearch_data weed;

static int nativeInit(void *opac, void *args)
{
	(void)args;
	(void)opac;
	return 0;
}

static int nativeDestroy(void *opac)
{
	free(opac);
	return 0;
}

static int nativeRegistreFunc(void *opac, const char *name, void *arg)
{
	ENTRY *ep = NULL;

	(void)opac;
	hsearch_r((ENTRY){.key = (char *)name, .data = 0}, FIND, &ep, &weed);
	if (!ep) {
		hsearch_r((ENTRY){.key = (char *)name, .data = arg},
			  ENTER, &ep, &weed);
		if (!ep) {
			DPRINT_ERR("could not registre %s\n", name);
			return -1;
		}
	}
	return 0;
}

static void *nativeGetFastPath(void *sm, const char *name)
{
	ENTRY *ep = NULL;

	(void)sm;
	hsearch_r((ENTRY){.key = (char *)name}, FIND, &ep, &weed);
	if (!ep) {
		return NULL;
	}
	return ep->data;

}

static void *nativeFastCall(void *sm, void *opacFunc, int nb,
			    union ycall_arg *args, int *types)
{
	void *(*f)(int, union ycall_arg *, int *) = opacFunc;
	return f(nb, args, types);
}

static void *nativeCall(void *opac, const char *name, int nb,
			union ycall_arg *args, int *types)
{
	(void)opac;
	void *(*func)(int, union ycall_arg *, int *) =
		nativeGetFastPath(NULL, name);
	if (unlikely(!func))
		return NULL;
	return func(nb, args, types);
}

static void *nativeAllocator(void)
{
	YScriptOps *ret;

	ret = y_new0(YScriptOps, 1);
	if (ret == NULL)
		return NULL;
	ret->init = nativeInit;
	ret->destroy = nativeDestroy;
	ret->call = nativeCall;
	ret->getFastPath = nativeGetFastPath;
	ret->fastCall = nativeFastCall;
	ret->registreFunc = nativeRegistreFunc;
	return (void *)ret;
}

static int ysNativeInit(void)
{
	t = ysRegister(nativeAllocator);
	hcreate_r(1024, &weed);
	return t;
}

int ysNativeEnd(void)
{
	hdestroy_r(&weed);
	ysDestroyManager(manager);
	manager = NULL;
	return ysUnregiste(t);
}

void *ysNativeManager(void)
{
	if (manager)
		return manager;
	ysNativeInit();
	manager = ysNewManager(NULL, t);
	return manager;
}

Entity *ysRegistreCreateNativeEntity(void *(*value)(int, union ycall_arg *, int *),
				     const char *name,
				     Entity *father, const char *entityName)
{
	ysRegistreFunc(ysNativeManager(), (char *)name, value);
	if (!entityName)
		entityName = name;
	return yeCreateFunction(name, ysNativeManager(), father, entityName);

}
