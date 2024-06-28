#include <glib.h>
#include "all.h"
#include "tests.h"

static void *cmp(int nb, union ycall_arg *args, int *types)
{
	Entity *e0 = args[0].e;
	Entity *e1 = args[1].e;

	return (void *) yeGetInt(e0) - yeGetInt(e1);
}

static void *invCmp(int nb, union ycall_arg *args, int *types)
{
	Entity *e0 = args[0].e;
	Entity *e1 = args[1].e;

	return (void *) yeGetInt(e1) - yeGetInt(e0);
}

void testSwap(void)
{
	GameConfig cfg;

	g_assert(!ygInitGameConfig(&cfg, NULL, YNONE));
	g_assert(!ygInit(&cfg));
	ygCleanGameConfig(&cfg);

	Entity *array = yeCreateArray(NULL, NULL);
	yeCreateInt(0, array, NULL);
	yeCreateInt(1, array, NULL);
	yeCreateIntAt(3, array, NULL, 3);

	g_assert(yeGetIntAt(array, 0) == 0);
	g_assert(yeGetIntAt(array, 1) == 1);
	g_assert(yeGetIntAt(array, 3) == 3);

	yeSwapByIdx(array, 0, 1);
	g_assert(yeGetIntAt(array, 0) == 1);
	g_assert(yeGetIntAt(array, 1) == 0);
	g_assert(yeGetIntAt(array, 3) == 3);

	yeSwapByIdx(array, 2, 3);
	g_assert(yeGetIntAt(array, 0) == 1);
	g_assert(yeGetIntAt(array, 1) == 0);
	g_assert(yeGetIntAt(array, 2) == 3);

	ygEnd();
}

void testSorting(void)
{
	GameConfig cfg;

	g_assert(!ygInitGameConfig(&cfg, NULL, YNONE));
	g_assert(!ygInit(&cfg));
	ygCleanGameConfig(&cfg);

	ysRegistreNativeFunc("cmp", cmp);
	ysRegistreNativeFunc("invCmp", invCmp);
	Entity *cmp = yeCreateFunction("cmp", ysNativeManager(), NULL, NULL);
	Entity *invCmp = yeCreateFunction("invCmp", ysNativeManager(), NULL, NULL);


	Entity *array = yeCreateArray(NULL, NULL);
	yeCreateInt(4, array, NULL);
	yeCreateInt(3, array, NULL);
	yeCreateInt(2, array, NULL);
	yeCreateInt(1, array, NULL);
	yeCreateInt(0, array, NULL);


	yeQuickSort(array, cmp, 0);
	g_assert(yeGetIntAt(array, 0) == 0);
	g_assert(yeGetIntAt(array, 1) == 1);
	g_assert(yeGetIntAt(array, 2) == 2);
	g_assert(yeGetIntAt(array, 3) == 3);
	g_assert(yeGetIntAt(array, 4) == 4);
	yeQuickSort(array, invCmp, 0);
	g_assert(yeGetIntAt(array, 4) == 0);
	g_assert(yeGetIntAt(array, 3) == 1);
	g_assert(yeGetIntAt(array, 2) == 2);
	g_assert(yeGetIntAt(array, 1) == 3);
	g_assert(yeGetIntAt(array, 0) == 4);
	yeQuickSort(array, cmp, 2);
	g_assert(yeGetIntAt(array, 0) == 4);
	g_assert(yeGetIntAt(array, 1) == 3);
	g_assert(yeGetIntAt(array, 2) == 0);
	g_assert(yeGetIntAt(array, 3) == 1);
	g_assert(yeGetIntAt(array, 4) == 2);

	yeDumbSort(array, cmp, 0);
	g_assert(yeGetIntAt(array, 0) == 0);
	g_assert(yeGetIntAt(array, 1) == 1);
	g_assert(yeGetIntAt(array, 2) == 2);
	g_assert(yeGetIntAt(array, 3) == 3);
	g_assert(yeGetIntAt(array, 4) == 4);


	ygEnd();
}
