#include "yirl/game.h"
#include "tests.h"

void testCanvasSdl2(void)
{
  yeInitMem();
  GameConfig cfg;
  Entity *dialogue_example = yeCreateArray(NULL, NULL);
  YWidgetState *wid;

  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  yeCreateString("canvas", dialogue_example, "<type>");
  wid = ywidNewWidget(dialogue_example, NULL);
  g_assert(wid);
  ywidSetMainWid(wid);
  ygDoLoop();
  yeDestroy(dialogue_example);
  ygEnd();
}
