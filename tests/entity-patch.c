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
  char *tmp;

  jsonManager = ydNewManager(t);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  mn = yeGet(ret, "MenuTest");
  mn2 = yeCreateArray(NULL, NULL);

  tmp = yeToCStr(mn, 10, 0);
  printf("hi \n%s\n", tmp);
  free(tmp);
  yeCopy(mn, mn2);
  /* Manu rentre chez toi c'est  moi qu'tu fait d'la peine... */
  yeReCreateString("manu", mn2, "<type>");
  yeCreateString("ayoyoyo", mn2, "wololo");
  yeRemoveChild(mn2, "background");
  yeReCreateInt(1337, mn2, "button_background");
  entries = yeGet(mn2, "entries");
  yeSetAt(yeGet(entries, 0), "text", "new text");
  yeCreateFloat(3.14, entries, NULL);

  patch = yePatchCreate(mn, mn2, NULL, NULL);
  g_assert(patch);
  tmp = yeToCStr(patch, 10, 0);
  printf("patch \n%s\n", tmp);
  free(tmp);
  yePatchAply(mn, patch);
  tmp = yeToCStr(mn, 10, 0);
  printf("hi \n%s\n", tmp);
  free(tmp);
  ydJsonEnd();
  ydDestroyManager(jsonManager);
  yeEnd();
}
