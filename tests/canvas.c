#include "yirl/game.h"
#include "yirl/pos.h"
#include "yirl/canvas.h"
#include "yirl/rect.h"
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
  Entity *canvas_example = yeCreateArray(NULL, NULL);
  Entity *actions;
  Entity *resources;
  Entity *resource;
  Entity *objs;
  Entity *obj;
  YWidgetState *wid;

  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  ysRegistreNativeFunc("moveImg", moveImg);
  yeCreateString("canvas", canvas_example, "<type>");

  actions = yeCreateArray(canvas_example, "actions");
  yeCreateString("QuitOnKeyDown", actions, NULL);
  yeCreateString("moveImg", actions, NULL);

  resources = yeCreateArray(canvas_example, "resources");

  /* Background */
  yeCreateString("rgba: 180 40 200 255", canvas_example, "background");

  /* Load eye into resources */
  resource = yeCreateArray(resources, NULL);
  yeCreateString("tests/head.png", resource, "img");

  /* Load sara into resources */
  resource = yeCreateArray(resources, NULL);
  yeCreateString("tests/SaraFullSheet.png", resource, "img");
  ywRectCreateInts(0, 0, 50, 70, resource, "img-src-rect");

  resource = yeCreateArray(resources, NULL);
  yeCreateString("tests/SaraFullSheet.png", resource, "img");
  ywRectCreateInts(0, 70, 50, 70, resource, "img-src-rect");

  /* Put eye into the canvas */
  objs = yeCreateArray(canvas_example, "objs");
  obj = yeCreateArray(objs, NULL);
  ywPosCreateInts(25, 40, obj, "pos");
  yeCreateInt(0, obj, "id");

  /* Put sara into the canvas */
  obj = yeCreateArray(objs, NULL);
  ywPosCreateInts(70, 100, obj, "pos");
  yeCreateInt(1, obj, "id");

  obj = yeCreateArray(objs, NULL);
  ywPosCreateInts(70, 180, obj, "pos");
  yeCreateInt(2, obj, "id");

  wid = ywidNewWidget(canvas_example, NULL);
  g_assert(wid);
  ywidSetMainWid(wid);
  ygDoLoop();
  yeDestroy(canvas_example);
  ygEnd();
}
