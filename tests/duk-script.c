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

#include <glib.h>
#include <stdio.h>
#include "tests.h"
#include "duk-script.h"
#include "entity-script.h"

void testDukScriptCall(void)
{
	void *sm = NULL;

	yeInitMem();
	g_assert(!ysDukInit());
	g_assert(!ysDukGetType());
	sm = ysNewManager(NULL, 0);
	g_assert(sm);

	Entity *e = yeCreateArray(NULL, NULL);
	Entity *i = yeCreateInt(1337, e, NULL);
	 /* I should dup 2 to a file and check the file after */
	ysLoadString(sm, "alert('HELLO')");
	ysLoadFile(sm, "./tests/hi.js");

	Entity *f = yeCreateFunction("alert", sm, e, NULL);

	ysCall(sm, "alert", e);
	printf("ret ent: %ld\n", (intptr_t)ysCall(sm, "display_ent", e));

	g_assert((intptr_t)ysCall(sm, "display_eint", i) == 1337);
	yesCall(f, i);
	printf("mk_hi: %s\n", yeGetString(ysCall(sm, "mk_hello", e, "name")));
	printf("mk_hi 2: %s\n", yeGetStringAt(e, "name"));
	Entity *s2 = ysCall(sm, "mk_hello2");

	printf("%s\n", yeGetString(s2));
	printf("%d - %d\n", e->refCount,  s2->refCount);
	yeMultDestroy(e, s2);
	g_assert(!ysDestroyManager(sm));
	g_assert(!ysDukEnd());
	yeEnd();

}
