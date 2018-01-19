#include "yirl/game.h"
#include "yirl/pos.h"
#include "yirl/canvas.h"
#include "yirl/rect.h"
#include "tests.h"

static void *moveImg(va_list ap)
{
  Entity *wid = va_arg(ap, Entity *);
  Entity *eve = va_arg(ap, Entity *);
  static int i;
  Entity *pos;

  if (eve && (ywidEveType(eve) == YKEY_DOWN)) {

    ywCanvasStringSet(yeGet(wid, "song"), yeGet(yeGet(wid, "strings"), i));
    ++i;
    i = i % 3;
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
    // if r is press ywCanvasRemoveObj() on rectangles
    if (ywidEveKey(eve) == 'r') {
      ywCanvasRemoveObj(wid, yeGet(wid, "r-rm"));
      yeRemoveChildByStr(wid, "r-rm");
      return (void *)ACTION;
    }
    if (ywidEveKey(eve) == 't') {
      ywCanvasRemoveObj(wid, yeGet(wid, "R-rm"));
      yeRemoveChildByStr(wid, "R-rm");
      return (void *)ACTION;
    }
    if (ywidEveKey(eve) == '0') {
      printf("sara: %d\n",
	     ywCanvasObjectsCheckColisions(ywCanvasObjFromIdx(wid, 0),
					   ywCanvasObjFromIdx(wid, 1)));
      printf("floop square: %d\n",
	     ywCanvasObjectsCheckColisions(ywCanvasObjFromIdx(wid, 0),
					   ywCanvasObjFromIdx(wid, 2)));
      printf("text: %d\n",
	     ywCanvasObjectsCheckColisions(ywCanvasObjFromIdx(wid, 0),
					   ywCanvasObjFromIdx(wid, 3)));
      printf("rect: %d\n",
	     ywCanvasObjectsCheckColisions(ywCanvasObjFromIdx(wid, 0),
					   yeGet(wid, "r-rm")));
      return (void *)NOTHANDLE;
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
  yeCreateString("tests/Street.png", resource, "img");
  ywRectCreateInts(320, 512, 96, 96, resource, "img-src-rect");

  resource = yeCreateArray(resources, NULL);
  yeCreateString("ohh you touch my tralala !", resource, "text");

  Entity *strs = yeCreateArray(canvas_example, "strings");

  yeCreateString("SPAM SPAM SPAM SPAM", strs, NULL);
  yeCreateString("LOVELY SPAM !", strs, NULL);
  yeCreateString("WONDERFUL SPAM !!!", strs, NULL);

  /* Put eye into the canvas */
  objs = yeCreateArray(canvas_example, "objs");
  ywCanvasNewObj(canvas_example, 25, 40, 0);
  /* Put sara into the canvas */
  obj = ywCanvasNewObj(canvas_example, 70, 100, 1);
  ywCanvasRotate(obj, -90);
  ywCanvasForceSize(obj, ywSizeCreate(100, 200, canvas_example, NULL));
  obj = ywCanvasNewObj(canvas_example, 470, 380, 2);
  ywCanvasForceSize(obj, ywSizeCreate(200, 200, canvas_example, NULL));
  obj = ywCanvasNewObj(canvas_example, 200, 250, 3);
  ywCanvasRotate(obj, 45);

  ywCanvasNewImgByPath(canvas_example, 200, 250, "tests/hero.png");

  obj = yeCreateArray(objs, NULL);
  yeCreateInt(YCanvasRect, obj, NULL);
  ywPosCreateInts(400, 250, obj, NULL);
  Entity *rect = yeCreateArray(obj, "rect");

  ywSizeCreate(200, 10, rect, NULL);
  yeCreateString("rgba: 180 0 0 160", rect, NULL);
  yePushBack(canvas_example, obj, "r-rm");

  yePushBack(canvas_example,
	     ywCanvasNewRect(canvas_example, 100, 250, rect),
	     "R-rm");
  ywCanvasForceSize(yeGet(canvas_example, "r-rm"),
		    ywSizeCreate(50, 50, canvas_example, NULL));
  yePushBack(canvas_example,
	     ywCanvasNewText(canvas_example, 100, 300, yeGet(strs, 0)),
	     "song");

  wid = ywidNewWidget(canvas_example, NULL);
  g_assert(wid);
  ywidSetMainWid(wid);
  ygDoLoop();
  yeDestroy(canvas_example);
  ygEnd();
  ygCleanGameConfig(&cfg);
}
