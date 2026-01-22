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
	Entity *texts;
	Entity *paths = yeGet(s_info, "paths");

	if (!s_info) {
		return NULL;
	}
	ret = yeCreateArray(father, name);

	texts = yeCreateArray(ret, "text");
	yePushBack(ret, thing, "char");
	yePushBack(ret, s_info, "sp");
	if (paths) {
		Entity *p;
		YE_FOREACH(paths, p) {
			ywTextureNewImg(yeGetString(p), NULL, texts, NULL);
		}
	} else {
		ywTextureNewImg(yeGetStringAt(s_info, "path"),
				NULL, texts, NULL);
	}
	yePushBack(ret, canvas_wid, "wid");
	yeCreateInt(0, ret, "x");
	yeCreateInt(0, ret, "y");
	yeCreateInt(0, ret, "y_offset");
	yeCreateInt(0, ret, "text_idx");
	return ret;
}

void *handlerAdvance(int nargs, void **args)
{
	Entity *h = args[0];
	Entity *sp = yeGet(h, "sp");
	int size = yeGetIntAt(sp, "size");
	int l = yeGetIntAt(sp, "length");

	assert(l);
	yeAddAt(h, "x", size);
	if (yeGetIntAt(h, "x") >= size * l) {
		yeSetAt(h, "x", 0);
	}
	return NULL;
}

void *handlerSetAdvancement(int nargs, void **args)
{
	Entity *h = args[0];
	int where = (intptr_t)args[1];
	Entity *sp = yeGet(h, "sp");
	int size = yeGetIntAt(sp, "size");
	int l = yeGetIntAt(sp, "length");

	assert(l);
	yeSetAt(h, "x", where * size);
	printf("set at %d - %d\n", where * size, size * l);
	if (yeGetIntAt(h, "x") >= size * l) {
		yeSetAt(h, "x", 0);
	}
	return NULL;
}

void *handlerRefresh(int nargs, void **args)
{
	Entity *h = args[0];
	int x = 0, y = 0;
	Entity *w = yeGet(h, "wid");
	Entity *c;
	Entity *ret;
	Entity *sp = yeGet(h, "sp");
	Entity *src_pos = yeGet(sp, "src-pos");
	int size = yeGetIntAt(sp, "size");
	int cur_x = yeGetIntAt(h, "x");
	int sy = 0;
	int text_idx = 0;

	if (yeType(src_pos) == YARRAY) {
		cur_x += yeGetIntAt(src_pos, 0);
		sy = yeGetIntAt(src_pos, 1);
	} else {
		sy = yeGetInt(src_pos);
	}
	yeAutoFree Entity *rect =
		ywRectCreateInts(cur_x, sy + yeGetIntAt(h, "y_offset"),
				 size, size, NULL, NULL);

	assert(size);

	if ((c = yeGet(h, "canvas"))) {
		Entity *tmpp = ywCanvasObjPos(c);

		x = ywPosX(tmpp);
		y = ywPosY(tmpp);
		ywCanvasRemoveObj(w, c);
		yeRemoveChild(h, "canvas");
	}
	text_idx = yeGetIntAt(h, "text_idx");
	ret = ywCanvasNewImgFromTexture(w, x, y,
					yeGet(yeGet(h, "text"), text_idx),
					rect);
	yePushBack(h, ret, "canvas");
	return h;
}

void *handlerPos(int nba, void **args)
{
	Entity *h = args[0];
	Entity *c = yeGet(h, "canvas");

	return c ? ywCanvasObjPos(c) : NULL;
}

void *handlerSize(int nba, void **args)
{
	Entity *h = args[0];
	Entity *c = yeGet(h, "canvas");

	return c ? ywCanvasObjSize(NULL, c) : NULL;
}

void *handlerSetPos(int nbArg, void **args)
{
	Entity *h = args[0];
	Entity *p = args[1];
	if (!h || !p)
		return NULL;

	if (!yeGet(h, "canvas")) {
		handlerRefresh(1, (void *[]){h});
	}

	ywCanvasObjSetPosByEntity(yeGet(h, "canvas"), p);
	return NULL;
}

void *handlerRemoveCanva(int nbArg, void **args)
{
	Entity *h = args[0];
	if (!h)
		return NULL;
	Entity *w = yeGet(h, "wid");

	ywCanvasRemoveObj(w, yeGet(h, "canvas"));
	yeRemoveChild(h, "canvas");
	return NULL;
}

void *handlerNullify(int nbArg, void **args)
{
	Entity *h = args[0];
	if (!h)
		return NULL;
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
		mod.handlerRemoveCanva = handlerRemoveCanva;
		mod.handlerAdvance = handlerAdvance;
		mod.handlerSetAdvancement = handlerSetAdvancement;
		mod.handlerPos = handlerPos;
		mod.handlerSize = handlerSize;
	}
	printf("SPRITE MANAGER %p!!!\n", mod);
	return mod;
}
