#include <glib.h>

#include "tests.h"
#include "yscript.h"

void ysciptAdd(void)
{
	yeInitMem();
	Entity *args = yeCreateArrayExt(NULL, NULL,
					YBLOCK_ARRAY_NOINIT |
					YBLOCK_ARRAY_NOMIDFREE);

	int64_t test0[] = {0,
			   'i', 4, //stack 4
			   'i', 3, //stack 3
				   //stack 0 will be use to store resulte
			   'i', 0,
			   // add stack 0 to stack 1
			   // and store resulte in stack 2
			   '+', 0, 1, 2,
			   'E', 2 // return 2nd elem
	};

	for (int i = 0; i < 5000000; ++i) {
		yeDestroy(yscript_exec(args, test0));
	}
	yeDestroy(args);
	yeEnd();
}

void yscriptLoop(void)
{
	yeInitMem();
	Entity *args = yeCreateArrayExt(NULL, NULL,
					YBLOCK_ARRAY_NOINIT |
					YBLOCK_ARRAY_NOMIDFREE);
	Entity *tmp;

	// this is a simple for(int i = 0; i < 500 000; ++i)
	int64_t test1[] = {0,
			   'i', 0, //stack 1 - 2
			   'i', 1, //stack 0 - 4
			   'i', 50000000, // 6
			   'j', 13, // jmp to loop - 8
			   '+', 0, 1, 0, // 12
			   // if stack 0 is iferior to stack 2, goto 9
			   '<', 0, 2, 9,
			   'i', 2, // stack 2 in 3
			   '*', 0, 3, 0, // mult 0 by 2
			   'E', 0 // return 2nd elem
	};

	tmp = yscript_exec(args, test1);
	g_assert(yeGetIntDirect(tmp) == (50000000 * 2));
	yeDestroy(tmp);
	yeDestroy(args);
	yeEnd();
}
