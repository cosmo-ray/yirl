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
#include "native-script.h"
#include "entity.h"

static int t = -1;
static void *manager;
GHashTable *weed;

static int nativeInit(void *opac, void *args)
{
	(void)args;
	(void)opac;
	return 0;
}

static int nativeDestroy(void *opac)
{
	g_free(opac);
	return 0;
}

static int nativeRegistreFunc(void *opac, const char *name, void *arg)
{
	(void)opac;
	g_hash_table_replace(weed, (char *)name, arg);
	return 0;
}

static void *nativeGetFastPath(void *sm, const char *name)
{
	(void)sm;
	return g_hash_table_lookup(weed, name);
}

static void *nativeFastCall(void *opacFunc, int nb,
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
		g_hash_table_lookup(weed, name);
	if (unlikely(!func))
		return NULL;
	return func(nb, args, types);
}

static void *nativeAllocator(void)
{
	YScriptOps *ret;

	ret = g_new0(YScriptOps, 1);
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
	weed = g_hash_table_new(g_str_hash, g_str_equal);
	return t;
}

int ysNativeEnd(void)
{
	g_hash_table_destroy(weed);
	weed = NULL;
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
