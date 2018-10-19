#include <glib.h>
#include "entity.h"
#include "json-desc.h"

void testEntityPatch(void)
{
  yeInitMem();
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  Entity *mn;
  Entity *mn2;
  Entity *entries;
  Entity *patch;

  jsonManager = ydNewManager(t);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  mn = yeGet(ret, "MenuTest");
  mn2 = yeCreateArray(NULL, NULL);

  yeCopy(mn, mn2);
  entries = yeGet(mn2, "entries");

  yeRemoveChild(entries, 2);
  g_assert(yeLen(entries) == 2);

  patch = yePatchCreate(mn, mn2, NULL, NULL);
  g_assert(patch);
  yePatchAply(mn, patch);
  yeDestroy(patch);

  g_assert(yeLen(yeGet(mn, "entries")) == 2);

  /* Manu rentre chez toi c'est a moi qu'tu fait d'la peine... */
  yeReCreateString("manu", mn2, "<type>");
  yeCreateString("ayoyoyo", mn2, "wololo");
  yeRemoveChild(mn2, "background");
  yeReCreateInt(1337, mn2, "button_background");
  yeSetAt(yeGet(entries, 0), "text", "new text");
  yeCreateFloat(3.14, entries, NULL);

  patch = yePatchCreate(mn, mn2, NULL, NULL);
  g_assert(patch);

  yePatchAply(mn, patch);

  g_assert(!yeStrCmpAt(mn, "ayoyoyo", "wololo"));
  /* une gonsesse de perdu c'est 10 copains qui reviennes */
  g_assert(!yeStrCmpAt(mn, "manu", "<type>"));
  g_assert(yeGetIntAt(mn, "button_background") == 1337);
  g_assert(!yeGet(mn, "background"));
  entries = yeGet(mn, "entries");
  g_assert(yeGetFloatAt(entries, 2) == 3.14);
  g_assert(!yeStrCmpAt(yeGet(entries, 0), "new text", "text"));
  ydJsonEnd();
  ydDestroyManager(jsonManager);
  yeEnd();
}
