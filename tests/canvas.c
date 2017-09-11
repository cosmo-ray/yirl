#include "yirl/game.h"
#include "yirl/pos.h"
#include "yirl/canvas.h"
#include "tests.h"

static void *moveImg(va_list ap)
{
  Entity *wid = va_arg(ap, Entity *);
  Entity *eve = va_arg(ap, Entity *);
  Entity *pos;

  if (eve && (ywidEveType(eve) == YKEY_DOWN)) {

    if (ywidEveKey(eve) == Y_UP_KEY) {
      pos = ywPosCreate(0, -10, NULL, NULL);
      ywCanvasMoveObjByIdx(wid, 0, pos);
      yeDestroy(pos);
      return (void *)ACTION;
    }
    if (ywidEveKey(eve) == Y_DOWN_KEY) {
      pos = ywPosCreate(0, 10, NULL, NULL);
      ywCanvasMoveObjByIdx(wid, 0, pos);
      yeDestroy(pos);
      return (void *)ACTION;
    }
    if (ywidEveKey(eve) == Y_LEFT_KEY) {
      pos = ywPosCreate(-10, 0, NULL, NULL);
      ywCanvasMoveObjByIdx(wid, 0, pos);
      yeDestroy(pos);
      return (void *)ACTION;
    }
    if (ywidEveKey(eve) == Y_RIGHT_KEY) {
      pos = ywPosCreate(10, 0, NULL, NULL);
      ywCanvasMoveObjByIdx(wid, 0, pos);
      yeDestroy(pos);
      return (void *)ACTION;
    }
  }
  return (void *)NOTHANDLE;
}

void testCanvasSdl2(void)
{
  yeInitMem();
  GameConfig cfg;
  Entity *dialogue_example = yeCreateArray(NULL, NULL);
  Entity *actions;
  Entity *resources;
  Entity *resource;
  Entity *objs;
  Entity *obj;
  YWidgetState *wid;

  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  ysRegistreNativeFunc("moveImg", moveImg);
  yeCreateString("canvas", dialogue_example, "<type>");
  actions = yeCreateArray(dialogue_example, "actions");
  yeCreateString("QuitOnKeyDown", actions, NULL);
  yeCreateString("moveImg", actions, NULL);
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
