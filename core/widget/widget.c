/*
**Copyright (C) 2015 Matthias Gatto
**
**This program is free software: you can redistribute it and/or modify
**it under the terms of the GNU Lesser General Public License as published by
**the Free Software Foundation, either version 3 of the License, or
**(at your option) any later version.
**
**This program is distributed in the hope that it will be useful,
**but WITHOUT ANY WARRANTY; without even the implied warranty of
**MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**GNU General Public License for more details.
**
**You should have received a copy of the GNU Lesser General Public License
**along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "game.h"
#include "timer.h"
#include "widget.h"
#include "rect.h"
#include "container.h"
#include "entity-script.h"
#include "sdl2/canvas-sdl.h"

int ywNeedTextureReload;

static Entity *subTypes = NULL;
static Entity *configs = NULL;

static YManagerAllocator widgetTab = {
	{NULL},
	0
};

static uint64_t rendersMask = 0;

/* struct which define what are common to every render of the same type */
struct renderOpt {
	Entity *(*waitEvent)(void);
	Entity *(*pollEvent)(void);
	int (*draw)(void);
	void (*resizePtr) (YWidgetState *wid, int renderType);
	int (*changeResolution) (void);
	void (*changeWinName)(const char *);
};

/* contain the options unique to one type of widget */
struct widgetOpt widgetOptTab[MAX_NB_MANAGER];
/* Contain the options unique to one type of render */
struct renderOpt renderOpTab[MAX_NB_MANAGER];

static YWidgetState *mainWid = NULL;
static YWidgetState *oldWid = NULL;

int ywidWindowWidth = 640;
int ywidWindowHight = 480;
int ywidXMouseGlobalPos;
int ywidYMouseGlobalPos;
int ywidXMouseLastClick;
int ywidYMouseLastClick;
int ywIsSmootOn;
int ywTurnPecent;
unsigned int ywTurnId_;

int yeveWindowGetFocus;

struct Kaboumable {
	YWidgetState *kwid;
	struct Kaboumable *prev;
} *kaboumables;

int ywTurnLengthOverwrite = -2;


unsigned int ywTurnId(void)
{
	return ywTurnId_;
}

int ywGetTurnLengthOverwrite(void)
{
	return ywTurnLengthOverwrite;
}

void ywSetTurnLengthOverwrite(int i)
{
	ywTurnLengthOverwrite = i;
}

int ywidRend(YWidgetState *opac)
{
	int ret = 0;
	if (opac->render) {
		yeveWindowGetFocus = 0;
		ret = opac->render(opac);
		ywidDrawScreen();
	}
	return ret;
}

int ywidFontSize(void)
{
	return sgGetFontSize();
}

int ywidFontW(void)
{
	return sgGetTxtW();
}

int ywidFontH(void)
{
	return sgGetTxtH();
}

/**
 * this destroy all widget mark as destroyable in a very american way
 * hence KBOUM ! (BOUM could have been enough, but I like KDE)
 */
static void doKaboum(void)
{
	while (kaboumables) {
		struct Kaboumable *pk = kaboumables->prev;

		yeRemoveChildByStr(kaboumables->kwid->entity,
				   "$father-container");
		YWidDestroy(kaboumables->kwid);
		free(kaboumables);
		kaboumables = pk;
	}
}

void ywidMarkAsDestroyable(YWidgetState *kboumable)
{
	struct Kaboumable *ck = kaboumables;

	if (!kboumable)
		return;
	kaboumables = malloc(sizeof(struct Kaboumable));
	kaboumables->kwid = kboumable;
	kaboumables->prev = ck;
}

void ywidChangeResolution(int w, int h)
{
	ywidWindowWidth = w;
	ywidWindowHight = h;
	YUI_FOREACH_BITMASK(rendersMask, i, tmask) {
		if (renderOpTab->changeResolution)
			renderOpTab->changeResolution();
	}
	YWidgetState *wid = ywidGetMainWid();
	if (wid) {
		ywidResize(wid);
	}
}

void ywidSetWindowName(const char *str)
{
	if (!configs)
		configs = yeCreateArray(NULL, NULL);
	yeCreateString(str, configs, "win-name");

	YUI_FOREACH_BITMASK(rendersMask, i, tmask) {
		if (renderOpTab->changeWinName)
			renderOpTab->changeWinName(str);
	}
}

const char *ywidWindowName(void)
{
	const char *ret = yeGetString(yeGet(configs, "win-name"));
	if (ret)
		return  ret;
	return "YIRL isn't a rogue like";
}

void ywidFreeWidgets(void)
{
	doKaboum();
	YWidDestroy(mainWid);
	YWidDestroy(oldWid);
	yeDestroy(subTypes);
	yeDestroy(configs);
	configs = NULL;
	subTypes = NULL;
	mainWid = NULL;
	oldWid = NULL;
}

inline YWidgetState *ywidGetMainWid(void)
{
	return mainWid;
}

const char *ywidTypeName(YWidgetState *wid)
{
	return widgetOptTab[ywidType(wid)].name;
}

void ywidSetMainWid(YWidgetState *wid)
{
	if (oldWid && oldWid != wid && oldWid != mainWid)
		YWidDestroy(oldWid);
	oldWid = mainWid;
	mainWid = wid;
}

_Bool ywidInTree(Entity *widget)
{
	if (widget == mainWid->entity)
		return 1;
	return ywCntInTree(mainWid->entity, widget);
}

int ywidNext(Entity *next, Entity *target)
{
	YWidgetState *newWid;
	const char *str = yeGetString(next);

	if (!next) {
		DPRINT_ERR("next is null");
		return -1;
	}

	if (yeType(next) == YARRAY && yeGet(next, "<type>"))
		goto next_is_wid;

	if (yeType(next) != YSTRING) {
		DPRINT_ERR("next is of type %s, should be a string",
			   yeTypeToString(yeType(next)));
		return -1;
	}

	next = ygGet(str);
	if (!next) {
		DPRINT_ERR("fail to retrive %s", str);
		return -1;
	}

next_is_wid:
	if (!target) {
		target = mainWid->entity;
	}

	if (yeType(target) == YSTRING) {
		if (!yeStrCmp(target, "main")) {
			target = ywidGetMainWid()->entity;
		} else {
			target = ygGet(yeGetString(target));
		}
	}

	if (ywidGetState(target) == mainWid) {

		YWidDestroy(oldWid);
		oldWid = NULL;
		if ((newWid = ywidNewWidget(next, NULL)) == NULL) {
			DPRINT_ERR("fail when creating new widget");
			return -1;
		}

		ywidSetMainWid(newWid);
		return 0;
	}
	return ywReplaceEntryByEntity(ywCntWidgetFather(target), target, next);
}

int ywidColorFromString(const char *str, uint8_t *r, uint8_t *g,
			uint8_t *b, uint8_t *a)
{
	size_t len;
	int limiterPos = 0;
	int ret = -1;

	if (!str)
		return -1;
	for (int i = 0; str[i]; ++i) {
		if (str[i] == ':') {
			limiterPos = i;
			break;
		}
	}

	len = strlen(str);
	// collor or whatever
	if (limiterPos) {
		char str_cpy[len + 1];

		strcpy(str_cpy, str);
		str = str_cpy;
		str_cpy[limiterPos] = '\0';

		if (yuiStrEqual(str, "rgba")) {
			const char *rgba;

			str += (limiterPos + 1);
			if (len < sizeof("rgba:r,g,b")) {
				DPRINT_ERR("invalide rgba color string: '%s'",
					   str);
				return -1;
			}

#define CHECK_RGBA(rgba, str)						\
			if (!rgba)					\
			{						\
				DPRINT_ERR("invalide rgba color string: %s", \
					   str);			\
				return -1;				\
			}						\

			rgba = str;
			*r = atoi(rgba);
			rgba = strpbrk(rgba+1, ", :");
			CHECK_RGBA(rgba, str);
			*g = atoi(rgba);
			rgba = strpbrk(rgba+1, ", :");
			CHECK_RGBA(rgba, str);
			*b = atoi(rgba);
			rgba = strpbrk(rgba+1, ", :");
			if (rgba)
				*a = atoi(rgba);
		ret = 0;

		}

	} else if (str[0] == '#') {
		char *eptr;
		long c = strtol(str + 1, &eptr, 16);

		if (eptr == str + 1)
			return -1;
		*r = (c & 0x00ff0000) >> 16;
		*g = (c & 0x0000ff00) >> 8;
		*b = c & 0x000000ff;
		*a = 0xff;
		ret = 0;
	} else if (!strcmp(str, "white")) {
		*r = 255;
		*g = 255;
		*b = 255;
		*a = 255;
		ret = 0;
	} else if (!strcmp(str, "black")) {
		*r = 0;
		*g = 0;
		*b = 0;
		*a = 255;
		ret = 0;
	}
	return ret;
}

int ywidBgConfFill(Entity *entity, YBgConf *cfg)
{
	char *str = (char *)yeGetString(entity);

	cfg->type = BG_BUG;
	cfg->flag = 0;
	if (!str)
		return -1;

	if (!ywidColorFromString(str, &cfg->r, &cfg->g, &cfg->b, &cfg->a))
		cfg->type = BG_COLOR;
	else if ((cfg->path = strdup(str)) != NULL)
		cfg->type = BG_IMG;
	else
		return -1;

	return 0;
}

void ywidRemoveRender(int renderType)
{
	rendersMask ^= (1LLU << renderType);
	renderOpTab[renderType].resizePtr = NULL;
	for(int i = 0; i < MAX_NB_MANAGER; ++i) {
		widgetOptTab[i].rendersMask ^= (1LLU << renderType);
		widgetOptTab[i].init[renderType] = NULL;
		widgetOptTab[i].render[renderType] = NULL;
	}
}

int ywidRegister(void *(*allocator)(void), const char *name)
{
	int ret = yuiRegister(&widgetTab, allocator);
	if (ret < 0)
		return -1;

	widgetOptTab[ret].name = strdup(name);
	if (!widgetOptTab[ret].name)
		return -1;

	widgetOptTab[ret].rendersMask = 0;
	memset(widgetOptTab[ret].init, 0, 64 * sizeof(void *));
	memset(widgetOptTab[ret].render, 0, 64 * sizeof(void *));
	memset(widgetOptTab[ret].destroy, 0, 64 * sizeof(void *));
	return ret;
}

int ywidUnregiste(int t)
{
	free(widgetOptTab[t].name);
	widgetOptTab[t].name = NULL;
	return yuiUnregiste(&widgetTab, t);
}

static YWidgetState *ywidNewWidgetInternal(int t,
					   Entity *entity,
					   int shouldInit)
{
	YWidgetState *ret;
	Entity *pos = yeGet(entity, "wid-pos");
	Entity *initer = yeGet(entity, "init");

	if (yeType(initer) != YFUNCTION)
		initer = ygGetFuncExt(yeGetString(initer));
	if (widgetTab.len <= t || widgetTab.allocator[t] == NULL) {
		return NULL;
	}

	if (pos == NULL)
		pos = ywRectCreateInts(0, 0, 1000, 1000, entity, "wid-pos");
	ywRectCreateInts(ywRectX(pos) * ywidWindowWidth / 1000,
			 ywRectY(pos) * ywidWindowHight / 1000,
			 ywRectW(pos) * ywidWindowWidth / 1000,
			 ywRectH(pos) * ywidWindowHight / 1000,
			 entity, "wid-pix");

	ret = widgetTab.allocator[t]();

	if (ret == NULL) {
		DPRINT_ERR("fail to allocate widget\n");
		return NULL;
	}

	ret->entity = entity;

	if (yeGet(entity, "$wid")) {
		DPRINT_WARN("$wid alerady existe,"
			    " this may lead to odd and"
			    " very unexpected behavior !!!!");
	}
	yeReCreateData(ret, entity, "$wid");

	/* Init widget */
	if (ret->init(ret, entity, NULL)) {
		DPRINT_ERR("fail durring init\n");
		goto error;
	}

	/* Init sub widget */
	if (shouldInit && initer) {
		yesCall(initer, entity);
	}

	return ret;

error:
	ret->destroy(ret);
	return NULL;
}

int ywidAddSubTypeNF(Entity *subType)
{
	if (!subTypes)
		subTypes = yeCreateArray(NULL, NULL);
	yePushBack(subTypes, subType, NULL);
	return 0;
}

int ywidAddSubType(Entity *subType)
{
	ywidAddSubTypeNF(subType);
	yeDestroy(subType);
	return 0;
}

YWidgetState *ywidNewWidget(Entity *entity, const char *type)
{
	Entity *recreateLogic;
	int shouldInit = 1;

	if (!entity) {
		DPRINT_ERR("entity is NULL");
		return NULL;
	}

	if (!type) {
		type = yeGetString(yeGet(entity, "<type>"));
	}

	if (!type) {
		DPRINT_ERR("unable to get type");
		return NULL;
	}

	if ((recreateLogic = yeGet(entity, "recreate-logic")) != NULL) {
		switch (yeGetInt(recreateLogic)) {
		case YRECALL_INIT:
			break;
		case YRESET:
			return yesCall(yeGet(entity, "reset"), entity);
		case YNOTHING:
			shouldInit = 0;
		}
	}

	if (shouldInit) {
		YE_ARRAY_FOREACH(subTypes, tmpType) {
			if (yuiStrEqual0(type, yeGetStringAt(tmpType, "name"))) {
				YWidgetState *ret;

				if (yeGet(tmpType, "callback-ent")) {
					type = yeGetStringAt(tmpType, "sub-type");
					yesCall(yeGet(tmpType, "callback-ent"), entity, tmpType);

					goto init_basic_widget;
				}
				ret = yesCall(yeGet(tmpType, "callback"),
					      entity, tmpType);

				if (!ret) {
					DPRINT_ERR("init for type '%s' fail",
						   type);
				}
				if (unlikely(yeIsPtrAnEntity(ret))) {
					DPRINT_ERR("widget init returned and Entity instead of a YWidgetState");
					ret = NULL;
				}
				return ret;
			}
		}
	}

init_basic_widget:
	for (int i = 0; i < 64; ++i) {
		if (widgetOptTab[i].name &&
		    yuiStrEqual(type, widgetOptTab[i].name)) {
			yeReCreateString(type, entity, "<r_t>");
			Entity *post_init = yeGet(entity, "post-init");
			YWidgetState *ret = ywidNewWidgetInternal(i, entity, shouldInit);
			if (!ret)
				return NULL;
			if (post_init) {
				yesCall(post_init, entity);
			}
			return ret;
		}
	}
	DPRINT_ERR("unable to find widget type: '%s'", type);
	return NULL;
}

void ywidResize(YWidgetState *wid)
{
	if (!wid) {
		DPRINT_ERR("wid is NULL");
		return;
	}
	Entity *pos = yeGet(wid->entity, "wid-pos");

	ywRectSet(yeGet(wid->entity, "wid-pix"),
		  ywRectX(pos) * ywidWindowWidth / 1000,
		  ywRectY(pos) * ywidWindowHight / 1000,
		  ywRectW(pos) * ywidWindowWidth / 1000,
		  ywRectH(pos) * ywidWindowHight / 1000);
	if (wid->resize)
		wid->resize(wid);
	YUI_FOREACH_BITMASK(rendersMask, i, tmask) {
		if (wid->renderStates[i].opac)
			renderOpTab[i].resizePtr(wid, i);
	}
}

/**
 * this functions set the callbacks of the render of type
 * @renderType use by the widget of type @type
 */
int ywidRegistreTypeRender(const char *type, int renderType,
			   int (*render)(YWidgetState *wid,
					 int renderType),
			   int (*init)(YWidgetState *opac, int t),
			   void (*destroy)(YWidgetState *opac, int t))
{
	if (renderType < 0) {
		return -1;
	}
	for (int i = 0; i < 64; ++i) {
		if (widgetOptTab[i].name &&
		    yuiStrEqual(type, widgetOptTab[i].name)) {
			widgetOptTab[i].rendersMask |= ONE64 << renderType;
			widgetOptTab[i].render[renderType] = render;
			widgetOptTab[i].init[renderType] = init;
			widgetOptTab[i].destroy[renderType] = destroy;
			return i;
		}
	}
	return -1;
}


int ywidRegistreRender(void (*resizePtr)(YWidgetState *wid, int renderType),
		       Entity *(*pollEvent)(void),
		       Entity *(*waitEvent)(void),
		       int (*draw)(void),
		       int (*changeResolution)(void),
		       void (*changeWinName)(const char *))
{
	YUI_FOREACH_BITMASK(~rendersMask, i, tmask) {
		rendersMask |= (1 << i);
		renderOpTab[i].resizePtr = resizePtr;
		renderOpTab[i].pollEvent = pollEvent;
		renderOpTab[i].waitEvent = waitEvent;
		renderOpTab[i].draw = draw;
		renderOpTab[i].changeResolution = changeResolution;
		renderOpTab[i].changeWinName = changeWinName;
		return i;
	}
	return -1;
}


static inline void defaultButtonConv(Entity *e)
{
	if (!ywidEveKey(e))
		yeSetIntAt(e, YEVE_KEY, '\n');
	else if (ywidEveKey(e) == 1)
		yeSetIntAt(e, YEVE_KEY, Y_ESC_KEY);
	else if (ywidEveKey(e) == 2)
		yeSetIntAt(e, YEVE_KEY, 'x');
	else if (ywidEveKey(e) == 3)
		yeSetIntAt(e, YEVE_KEY, ' ');
	else if (ywidEveKey(e) == 6)
		yeSetIntAt(e, YEVE_KEY, Y_ESC_KEY);
	else if (ywidEveKey(e) == 11)
		yeSetIntAt(e, YEVE_KEY, Y_UP_KEY);
	else if (ywidEveKey(e) == 12)
		yeSetIntAt(e, YEVE_KEY, Y_DOWN_KEY);
	else if (ywidEveKey(e) == 13)
		yeSetIntAt(e, YEVE_KEY, Y_LEFT_KEY);
	else if (ywidEveKey(e) == 14)
		yeSetIntAt(e, YEVE_KEY, Y_RIGHT_KEY);
	else
		printf("no convertion for controller key: %d\n",
		       ywidEveKey(e));
}

static void try_convert_axis_to_wasd(Entity *e)
{
	if (yeGetIntAt(e, YEVE_AXIS_ID))
		return;

	if (ywidEveKey(e) == Y_UP_KEY)
		yeSetIntAt(e, YEVE_KEY, 'w');
	else if (ywidEveKey(e) == Y_DOWN_KEY)
		yeSetIntAt(e, YEVE_KEY, 's');
	else if (ywidEveKey(e) == Y_LEFT_KEY)
		yeSetIntAt(e, YEVE_KEY, 'a');
	else if (ywidEveKey(e) == Y_RIGHT_KEY)
		yeSetIntAt(e, YEVE_KEY, 'd');
}

/* should be confiurrable, but for now only convert controller key to keyboard */
static Entity *keyReBind(Entity *e)
{
	if (!e)
		return NULL;
	int ekt = ywidEveType(e);

	if (ekt == YKEY_CT_AXIS_DOWN) {
		yeSetIntAt(e, YEVE_TYPE, YKEY_DOWN);
		try_convert_axis_to_wasd(e);
	} else if (ekt == YKEY_CT_AXIS_UP) {
		yeSetIntAt(e, YEVE_TYPE, YKEY_UP);
		try_convert_axis_to_wasd(e);
	} else if (ekt == YKEY_CT_DOWN) {
		yeSetIntAt(e, YEVE_TYPE, YKEY_DOWN);
		defaultButtonConv(e);
	} else if (ekt == YKEY_CT_UP) {
		yeSetIntAt(e, YEVE_TYPE, YKEY_UP);
		defaultButtonConv(e);
        /* we should recive a non negligable number of none when we use axies*/
	}

	return e;
}

Entity *ywidGenericPollEvent(void)
{
	YUI_FOREACH_BITMASK(rendersMask, i, tmask) {
		Entity *ret = keyReBind(renderOpTab[i].pollEvent());
		if (ret)
			return ret;
	}
	return NULL;
}

int ywidDrawScreen(void)
{
	YUI_FOREACH_BITMASK(rendersMask, i, tmask) {
		int ret;

		ret = renderOpTab[i].draw();
		if (ret)
			return ret;
	}
	return 0;
}


Entity *ywidGenericWaitEvent(void)
{
	Entity *ret = NULL;
	static int ax_timestamp;

	if (!rendersMask)
		return NULL;

	while (1) {
		yeDestroy(ret);
		ret = NULL;
		YUI_FOREACH_BITMASK(rendersMask, i, tmask) {
			if (renderOpTab[i].pollEvent &&
			    (ret = renderOpTab[i].pollEvent())) {
				if (ywidEveType(ret) == YKEY_CT_AXIS_DOWN) {
					if ((yeGetIntAt(ret, YEVE_TIMESTAMP) -
					     ax_timestamp) < 200)
						continue;
					ax_timestamp =
						yeGetIntAt(ret, YEVE_TIMESTAMP);
				}

				if ( ywidEveType(keyReBind(ret)) == YKEY_NONE) {
					continue;
				}
				return ret;
			}
		}
#ifndef USING_EMCC
		usleep(50);
#endif
	}

	return NULL;
}

void ywMapSetSmootMovement(Entity *map, int smoot)
{
	(void)map;
	ywIsSmootOn = smoot;
}

void YWidDestroy(YWidgetState *wid)
{
	if (!wid)
		return;

	Entity *ent = wid->entity;
	yesCall(yeGet(ent, "destroy"), ent);
	ywidGenericCall(wid, wid->type, destroy);
	if (wid->destroy)
		wid->destroy(wid);
	else
		free(wid);
	yeRemoveChild(ent, "$wid");
	if (yeGetIntAt(ent, "need_yedestroy"))
		yeDestroy(ent);
}

static void trackMouse(Entity *event)
{
	if (ywidEveType(event) == YKEY_MOUSEMOTION ||
	    ywidEveType(event) == YKEY_MOUSEWHEEL ||
	    ywidEveType(event) == YKEY_MOUSEDOWN) {
		ywidXMouseGlobalPos = ywidXMouse(event);
		ywidYMouseGlobalPos = ywidYMouse(event);
		if (ywidEveType(event) == YKEY_MOUSEDOWN) {
			ywidXMouseLastClick = ywidXMouseGlobalPos;
			ywidYMouseLastClick = ywidYMouseGlobalPos;
		}
	}
}

int64_t ywidTurnTimer;

int64_t ywidGetTurnTimer(void)
{
	return ywidTurnTimer;
}

int ywidDoTurn(YWidgetState *opac)
{
	int turnLength = ywTurnLengthOverwrite >= -1 ? ywTurnLengthOverwrite :
		yeGetInt(yeGet(opac->entity, "turn-length"));
	int ret;
	static YTimer *cnt = NULL;
	Entity *old = NULL;
	Entity *head;
	Entity *event;

	++ywTurnId_;
	doKaboum();
	if (!cnt)
		cnt = YTimerCreate();

	ywidTurnTimer = YTimerGet(cnt);
	if (turnLength == Y_REQUEST_ANIMATION_FRAME) {
		YTimerReset(cnt);
		for (event = ywidGenericPollEvent(), head = event; event;
		     event = ywidGenericPollEvent()) {
			trackMouse(event);
			if (old) {
				yePushAt(old, event, YEVE_NEXT);
				/* decrement event refcount */
				yeDestroy(event);
			}
			old = event;
		}
	} else if (turnLength > 0) {
		int64_t i = 0;

		i = turnLength - ywidTurnTimer;
		ywTurnPecent = 100 - (i * 100 / turnLength);
		while (i > 0 && ywTurnPecent < 95) {
			int newPercent;

			if (ywIsSmootOn) {
				ywidRend(mainWid);
			}
			do {
				usleep(10);
				i = turnLength - YTimerGet(cnt);
				newPercent = 100 - (i * 100 / turnLength);
			} while (newPercent == ywTurnPecent);
			ywTurnPecent = newPercent;
		}
		if (i > 0) {
			usleep(i);
		}
		YTimerReset(cnt);

		for (event = ywidGenericPollEvent(), head = event; event;
		     event = ywidGenericPollEvent()) {
			trackMouse(event);
			if (old) {
				yePushAt(old, event, YEVE_NEXT);
				yeDestroy(event); /* decrement event refcount */
			}
			old = event;
		}
	} else {
		head = ywidGenericWaitEvent();
		if (!head)
			return NOTHANDLE;
		trackMouse(head);
		YTimerReset(cnt);
	}
	ywTurnPecent = 0;
	ywidMidRendEnd(mainWid);
	ywNeedTextureReload = 0;
	ret = ywidHandleEvent(opac, head);
	yeDestroy(head);
	return ret;
}

InputStatue ywidAction(Entity *action, Entity *wid, Entity *eve)
{
	if (unlikely(!action))
		return NOTHANDLE;

	if (yeType(action) == YSTRING) {
		Entity *f = ygGet(yeGetString(action));
		InputStatue r = (InputStatue)(intptr_t)yesCall(f, wid, eve);

		if (unlikely(!ygIsInit())) {
			yeDestroy(f);
		}
		return r;
	} else if (yeType(action) == YFUNCTION) {
		return (InputStatue)(intptr_t)yesCall(action, wid, eve);
	} else {
		Entity *f = yeGet(action, 0);
		int nb = yeLen(action);
		union ycall_arg args[nb + 2];
		int types[5] = {YS_ENTITY};

		args[0].e = wid;
		args[1].e = eve;
		for (int i = 0; i < nb; ++i)
			args[i + 2].e = yeGet(action, i + 1);

		if (yeType(f) == YSTRING)
			f = ygGet(yeGetString(f));

		InputStatue r = (intptr_t)yesCallInt(f, nb + 1, args, types);

		if (unlikely(!ygIsInit() && yeType(f) == YSTRING))
			yeDestroy(f);
		return r;
	}
}

InputStatue ywidActions(Entity *wid, Entity *actionWid, Entity *eve)
{
	int wid_type = yeType(actionWid);
	if (wid_type != YARRAY && wid_type != YHASH)
		return NOTHANDLE;
	Entity *actions = yeGet(actionWid, "action");
	if (actions)
		return ywidAction(actions, wid, eve);
	actions = yeGet(actionWid, "actions");
	InputStatue ret = NOTHANDLE;

	if (!actions)
		return NOTHANDLE;
	switch (yeType(actions)) {
	case YSTRING:
	case YFUNCTION:
		return ywidAction(actions, wid, eve);
	case YARRAY:
	{
		YE_ARRAY_FOREACH(actions, action) {
			int cur_ret = ywidAction(action, wid, eve);

			if (cur_ret > ret)
				ret = cur_ret;
		}
	}
	break;
	default:
		DPRINT_ERR("Bad entity type: (%d) %s", yeType(actions),
			   yeTypeToString(yeType(actions)));
		return NOTHANDLE;
	}
	return ret;
}

InputStatue ywidEventCallActionSin(YWidgetState *opac,
				   Entity *event)
{
	return ywidActions(opac->entity, opac->entity, event);
}

int ywidHandleEvent(YWidgetState *opac, Entity *event)
{
	int ret = 0;
	Entity *postAction;

	if (opac->handleEvent)
		ret = opac->handleEvent(opac, event);

	if (ygIsAlive() && (postAction = yeGet(opac->entity,
					       "post-action")) != NULL)
		ret = (intptr_t)yesCall(postAction, ret, opac->entity, event);

	return ret;
}

_Bool ywIsPixsOnWid(Entity *widget, int posX, int posY)
{
	Entity *pixR = yeGet(widget, "wid-pix");

	return posX > ywRectX(pixR) && posX < (ywRectX(pixR) + ywRectW(pixR)) &&
		posY > ywRectY(pixR) && posY < (ywRectY(pixR) + ywRectH(pixR));
}
