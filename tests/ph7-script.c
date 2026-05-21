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
#include "ph7-script.h"
#include "entity-script.h"

void testPH7ScriptCall(void)
{
	void *sm = NULL;

	yeInitMem();
	g_assert(!ysPH7Init());
	g_assert(!ysPH7GetType());
	sm = ysNewManager(NULL, 0);
	g_assert(sm);

	Entity *e = yeCreateArray(NULL, NULL);
	Entity *i = yeCreateInt(1337, e, NULL);
	Entity *f = yeCreateFunction("display_eint", sm, e, NULL);

	g_assert(!ysLoadFile(sm, "./tests/hi.php"));

	printf("-----\n");
	ysCall(sm, "mod_init", e);
	printf("=====\n");

	printf("i: %p\n", i);
	g_assert((intptr_t)yesCall(f, i) == 1337);

	Entity *mk = yeCreateFunction("mk_hello", sm, e, NULL);
	yesCall(mk, e, "name");
	printf("mk_hi 2: %s\n", yeGetStringAt(e, "name"));

	/* Entity-based state test: create storage, increment, read back */
	printf("--- entity state test ---\n");
	Entity *st = (Entity *)ysCall(sm, "make_storage", e);
	ysCall(sm, "inc_storage", st);
	ysCall(sm, "inc_storage", st);
	ysCall(sm, "inc_storage", st);
	intptr_t sval = (intptr_t)ysCall(sm, "read_storage", st);
	printf("  storage value: %ld\n", (long)sval);
	g_assert(sval == 3);

	Entity *mk2 = yeCreateFunction("mk_hello2", sm, NULL, NULL);
	Entity *s2 = yesCall(mk2);
	printf("%s\n", yeGetString(s2));
	printf("%d - %d\n", e->refCount, s2->refCount);
	yeMultDestroy(e, s2);

	/* Loop test: call accumulate with incrementing values */
	printf("--- accumulate loop ---\n");
	intptr_t acc = 0;
	for (int i = 1; i <= 5; ++i) {
		acc = (intptr_t)ysCall(sm, "accumulate", i);
		printf("  after %d: %ld\n", i, (long)acc);
	}
	g_assert(acc == 15);

	g_assert(!ysDestroyManager(sm));
	g_assert(!ysPH7End());
	yeEnd();
}
