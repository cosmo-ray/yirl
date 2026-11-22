/*
**Copyright (C) 2019 Matthias Gatto
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

#ifdef _WIN32
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <glib.h>
#include <stdio.h>
#include "tests.h"
#include "quickjs-script.h"
#include "entity-script.h"
#include "script.h"

void testQjsScriptCall(void)
{
	void *sm = NULL;

	yeInitMem();
	g_assert(!ysQjsInit());
	g_assert(!ysQjsGetType());
	sm = ysNewManager(NULL, 0);
	g_assert(sm);

	Entity *e = yeCreateArray(NULL, NULL);
	Entity *i = yeCreateInt(1337, e, NULL);
	 /* I should dup 2 to a file and check the file after */
	ysLoadString(sm, "print('HELLO')");
	ysLoadFile(sm, "./tests/hi.js");

	Entity *f = yeCreateFunction("print", sm, e, NULL);

	printf("-----\n");
	ysCall(sm, "print", "hello !!");
	printf("=====\n");
	printf("ret ent: %zu\n", (intptr_t)ysCall(sm, "display_ent", e));

	printf("i: %p\n", i);
	g_assert((intptr_t)ysCall(sm, "display_eint", i) == 1337);
	yesCall(f, i);
	printf("mk_hi: %s\n", yeGetString(ysCall(sm, "mk_hello", e, "name")));
	printf("mk_hi 2: %s\n", yeGetStringAt(e, "name"));
	Entity *s2 = ysCall(sm, "mk_hello2");

	printf("%s\n", yeGetString(s2));
	printf("%d - %d\n", e->refCount,  s2->refCount);
	yeMultDestroy(e, s2);
	/* g_assert(!ysDestroyManager(sm)); */
	g_assert(!ysQjsEnd());
	yeEnd();
}

void testQjsNoDoubleFree(void)
{
	void *sm = NULL;

	yeInitMem();
	g_assert(!ysQjsInit());
	sm = ysNewManager(NULL, 0);
	g_assert(sm);

	for (int i = 0; i < 10000; ++i) {
		g_assert(!ysLoadString(sm, "'x';"));
		g_assert(!ysLoadString(sm, "({a:1});"));
		g_assert(!ysLoadString(sm, "[1,2,3];"));
	}
	g_assert(ysLoadString(sm, "throw 1;") < 0);

	Entity *arr = yeCreateArray(NULL, NULL);
	yeCreateInt(1, arr, NULL);
	yeCreateInt(2, arr, NULL);
	yeCreateInt(3, arr, NULL);

	g_assert(!ysLoadString(sm,
		"function fe(a){var n=0; a.forEach(function(v){n=n+1; return {x:v};}); return n;}"));
	g_assert(!ysLoadString(sm,
		"function ff(a){return a.filter(function(v){return {k:v};}).len();}"));
	for (int i = 0; i < 2000; ++i) {
		g_assert((intptr_t)ysCall(sm, "fe", arr) == 3);
		g_assert((intptr_t)ysCall(sm, "ff", arr) == 3);
	}

	yeDestroy(arr);
	g_assert(!ysQjsEnd());
	yeEnd();
}
