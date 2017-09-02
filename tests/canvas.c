#include "yirl/game.h"
#include "tests.h"

void testCanvasSdl2(void)
{
  yeInitMem();
  GameConfig cfg;
  Entity *dialogue_example = yeCreateArray(NULL, NULL);
  Entity *resources;
  Entity *resource;
  Entity *objs;
  Entity *obj;
  YWidgetState *wid;

  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  yeCreateString("canvas", dialogue_example, "<type>");
  yeCreateString("QuitOnKeyDown", dialogue_example, "action");
  resources = yeCreateArray(dialogue_example, "resources");
  yeCreateString("rgba: 180 40 200 255", dialogue_example, "background");
  resource = yeCreateArray(resources, NULL);
  yeCreateString("tests/head.png", resource, "img");
  objs = yeCreateArray(dialogue_example, "objs");
  obj = yeCreateArray(objs, NULL);
  ywPosCreateInts(25, 40, obj, "pos");
  yeCreateInt(0, obj, "img");
  wid = ywidNewWidget(dialogue_example, NULL);
  g_assert(wid);
  ywidSetMainWid(wid);
  ygDoLoop();
  yeDestroy(dialogue_example);
  ygEnd();
}
