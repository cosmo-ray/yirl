#include <glib.h>
#include "tests.h"
#include <yirl/widget.h>


void testMisc(void)
{
	unsigned char r, g, b, a;
	g_assert(!ywidColorFromString("#ffffff", &r, &g, &b, &a));
	g_assert(r == 0xff && g == 0xff && b == 0xff && a == 0xff);
	g_assert(!ywidColorFromString("#000000", &r, &g, &b, &a));
	g_assert(r == 0x0 && g == 0x0 && b == 0x0 && a == 0xff);
	g_assert(!ywidColorFromString("#cc77bb", &r, &g, &b, &a));
	g_assert(r == 0xcc && g == 0x77 && b == 0xbb && a == 0xff);
}
