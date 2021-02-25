#include <yirl/entity.h>
#include <yirl/events.h>
#include <yirl/entity-script.h>
#include <yirl/game.h>
#include <yirl/canvas.h>
#include <yirl/texture.h>

void *createHandler(int nbArg, void **args)
{
	Entity *thing = args[0];
	Entity *canvas_wid = args[1];
	Entity *father = nbArg > 2 ? args[2] : NULL;
	const char *name = nbArg > 3 ? args[3] : NULL;
	Entity *s_info = yeGet(thing, "sprite");
	Entity *ret;
	Entity *text;

	if (!s_info) {
		return NULL;
	}
	ret = yeCreateArray(father, name);

	yePushBack(ret, thing, "char");
	yePushBack(ret, s_info, "sp");
	text = ywTextureNewImg(yeGetStringAt(s_info, "path"), NULL,
			       ret, "text");
	yePushBack(ret, canvas_wid, "wid");
	yeCreateInt(0, ret, "x");
	yeCreateInt(0, ret, "y");
	yeCreateInt(0, ret, "y_offset");
	return ret;
}

void *handlerRefresh(int nargs, void **args)
{
	Entity *h = args[0];
	int x = 0, y = 0;
	Entity *w = yeGet(h, "wid");
	Entity *c;
	Entity *ret;
	Entity *sp = yeGet(h, "sp");
	int size = yeGetIntAt(sp, "size");
	int sy = yeGetIntAt(sp, "src-pos");
	yeAutoFree Entity *rect =
		ywRectCreateInts(0, sy + yeGetIntAt(h, "y_offset"),
				 size, size, NULL, NULL);

	assert(size);


	if ((c = yeGet(h, "canvas"))) {
		Entity *tmpp = ywCanvasObjPos(c);
		x = ywPosX(tmpp);
		y = ywPosY(tmpp);
		ywCanvasRemoveObj(w, c);
		yeRemoveChild(h, "canvas");
	}
	ret = ywCanvasNewImgFromTexture(w, x, y, yeGet(h, "text"), rect);
	yePushBack(h, ret, "canvas");
	return h;
}

void *handlerPos(int nba, void **args)
{
	Entity *h = args[0];
	Entity *c = yeGet(h, "canvas");

	return c ? ywCanvasObjPos(c) : NULL;
}

void *handlerSetPos(int nbArg, void **args)
{
	Entity *h = args[0];
	Entity *p = args[1];

	if (!yeGet(h, "canvas")) {
		handlerRefresh(1, (void *[]){h});
	}

	ywCanvasObjSetPosByEntity(yeGet(h, "canvas"), p);
	return NULL;
}

void *handlerNullify(int nbArg, void **args)
{
	Entity *h = args[0];
	Entity *w = yeGet(h, "wid");

	ywCanvasRemoveObj(w, yeGet(h, "canvas"));
	yeRemoveChild(h, "canvas");
	yeRemoveChild(h, "char");
	return NULL;
}

void *mod_init(int nbArg, void **args)
{
	Entity *mod = args[0];

	YEntityBlock {
		mod.name = "sprite-man";
		mod.createHandler = createHandler;
		mod.handlerRefresh = handlerRefresh;
		mod.handlerSetPos = handlerSetPos;
		mod.handlerNullify = handlerNullify;
		mod.handlerPos = handlerPos;
	}
	printf("SPRITE MANAGER %p!!!\n", mod);
	return mod;
}
