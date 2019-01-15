#include <yirl/entity.h>
#include <yirl/game.h>
#include <yirl/canvas.h>

void *viewerAction(int nbArg, void **args)
{
  Entity *wid = args[0];
  Entity *eve = args[1];

  if (!eve)
    return (void *)NOTHANDLE;
  if ((ywidEveType(eve) == YKEY_DOWN)) {
    if (ywidEveKey(eve) == Y_UP_KEY) {
      ywPosAddXY(yeGet(wid, "cam"), 0, -10);
      return (void *)ACTION;
    }
    if (ywidEveKey(eve) == Y_DOWN_KEY) {
      ywPosAddXY(yeGet(wid, "cam"), 0, 10);
      return (void *)ACTION;
    }
    if (ywidEveKey(eve) == Y_LEFT_KEY) {
      ywPosAddXY(yeGet(wid, "cam"), 10, 0);
      return (void *)ACTION;
    }
    if (ywidEveKey(eve) == Y_RIGHT_KEY) {
      ywPosAddXY(yeGet(wid, "cam"), -10, 0);
      return (void *)ACTION;
    }
  } else if (ywidEveType(eve) == YKEY_MOUSEMOTION) {
    Entity *tmpPos = ywPosCreate(ywidEveMousePos(eve), 0, NULL, NULL);
    Entity *str;
    Entity *txt = yeGet(wid, "txt");

    ywPosAdd(tmpPos, yeGet(wid, "cam"));
    str = yeCreateString(ywPosToString(tmpPos), NULL, NULL);
    if (!txt) {
      txt = ywCanvasNewText(wid, ywPosX(tmpPos), ywPosY(tmpPos), str);
      yePushBack(wid, txt, "txt");
    } else {
      ywCanvasStringSet(txt, str);
      ywCanvasObjSetPosByEntity(txt, tmpPos);
    }
    yeMultDestroy(tmpPos, str);
    return (void *)ACTION;
  } else if (ywidEveType(eve) == YKEY_MOUSEDOWN) {
    Entity *tmpPos = ywPosCreate(ywidEveMousePos(eve), 0, NULL, NULL);

    ywPosAdd(tmpPos, yeGet(wid, "cam"));
    printf("[%d, %d]\n", ywPosX(tmpPos), ywPosY(tmpPos));
    yeDestroy(tmpPos);
  }
  return (void *)NOTHANDLE;
}

void *viewerInit(int nbArg, void **args)
{
  Entity *canvas = args[0];

  ywCanvasNewImg(canvas, 0, 0, yProgramArg, NULL);
  ywPosCreateInts(0, 0, canvas, "cam");
}


void *mod_init(int nbArg, void **args)
{
  Entity *mod = YE_TO_ENTITY(args[0]);

  Entity *canvas = ywCreateCanvasEnt(mod, "main");
  Entity *actions = yeCreateArray(canvas, "actions");
  yeCreateFunction("viewerAction", ygGetManager("tcc"), actions, NULL);
  yeCreateString("QuitOnKeyDown", actions, NULL);
  yeCreateFunction("viewerInit", ygGetManager("tcc"), canvas, "init");
  yeCreateString("main", mod, "starting widget");
}
