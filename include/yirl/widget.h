/*
**Copyright (C) 2015-2023 Matthias Gatto
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

/*
 * This work is based on the widget work of YIRL V2
 */
#ifndef	_YIRL_WIDGET_H_
#define	_YIRL_WIDGET_H_

#include <sys/queue.h>
#include "yirl/entity.h"
#include "yirl/utils.h"
#include "yirl/keydef.h"
#include "yirl/pos.h"
#include "yirl/rect.h"

extern int yeveWindowGetFocus;
extern int ywNeedTextureReload;

#define Y_REQUEST_ANIMATION_FRAME -1

extern int64_t ywidTurnTimer;

/**
 * @brief get length in us since last turn (frame ?)
 */
int64_t ywidGetTurnTimer(void);

typedef enum
{
	YRECALL_INIT = 0,
	YRESET = 1,
	YNOTHING = 2,
} RecreateLogic;

typedef enum
{
	BUG = -1,
	NOTHANDLE = 0,
	ACTION = 1,
} InputStatue;

/* here for retro compat, use to mean, need to refresh screen
 * but nother else happen, as we now refresh screen every frame,
 * this option is now useless */
#define NOACTION NOTHANDLE

static inline const char *ywidInputStatueToString(InputStatue i)
{
	const char *ret[] = {"BUG", "NOTHANDLE", "NOACTION", "ACTION"};

	return ret[1 + i];
}

typedef enum
{
	YKEY_DOWN,
	YKEY_UP,
	YKEY_MOUSEDOWN,
	YKEY_MOUSEUP,
	YKEY_MOUSEWHEEL,
	YKEY_MOUSEMOTION,
	YKEY_CT_DOWN,
	YKEY_CT_UP,
	YKEY_CT_AXIS_DOWN,
	YKEY_CT_AXIS_UP,
	YKEY_NONE
} EventType;

enum BACKGOUND_FLAG {
	YBG_NONE = 0,
	YBG_FIT_TO_SCREEN_H = 1,
	YBG_FIT_TO_SCREEN_W = 1 << 2,
	YBG_FIT_TO_SCREEN = 1 | 2 /* YBG_FIT_TO_SCREEN_H and YBG_FIT_TO_SCREEN_W */
};

typedef enum
{
	BG_BUG = -1,
	BG_IMG,
	BG_COLOR
} BgType;

typedef struct {
	BgType type;

	union {
		struct {
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};

		struct {
			char *path;
		};

		uint32_t rgba;
	};

	int flag;

} YBgConf;

struct WidgetState_;


enum {
	YEVE_TYPE = 0,
	YEVE_KEY = 1,
	YEVE_MOUSE = 2,
#define YEVE_INTENSITY YEVE_MOUSE
	YEVE_CONTROLLER_ID = 3,
	YEVE_AXIS_ID = 4,
	YEVE_TIMESTAMP = 5,
	YEVE_STATUS = 15,
	YEVE_NEXT = 16
};

enum {
	YEVE_AX_L,
	YEVE_AX_R,
	YEVE_TRIGER_L,
	YEVE_TRIGER_R,
	YEVE_MAX_AXIES,
};

#define YEVE_ACTION ((void *)ACTION)

#define YEVE_NOTHANDLE ((void *)NOTHANDLE)

static inline EventType ywidEveType(Entity *eve)
{
  return (EventType)yeGetInt(yeGet(eve, YEVE_TYPE));
}

static inline int ywidEveKey(Entity *eve)
{
    return yeGetInt(yeGet(eve, YEVE_KEY));
}

static inline Entity *ywidEveMousePos(Entity *eve)
{
    return yeGet(eve, YEVE_MOUSE);
}

static inline int ywidXMouse(Entity *eve)
{
  return ywPosX(ywidEveMousePos(eve));
}

static inline int ywidYMouse(Entity *eve)
{
  return ywPosY(ywidEveMousePos(eve));
}

static inline Entity *ywidEveStatus(Entity *eve)
{
    return yeGet(eve, YEVE_STATUS);
}

static inline void ywidEveSetStatus(Entity *eve, InputStatue is)
{
  yeSetIntAt(eve, YEVE_STATUS, is);
}

typedef struct {
	void *opac;
} YRenderState;

typedef struct WidgetState_ {
	Entity *entity;
	YRenderState renderStates[64];
	int (*render)(struct WidgetState_ *opac);
	void (*midRendEnd)(struct WidgetState_ *opac);
	InputStatue (*handleEvent)(struct WidgetState_ *opac, Entity *event);
	void (*resize)(struct WidgetState_ *opac);
	int (*init)(struct WidgetState_ *opac, Entity *entity, void *args);
	int (*destroy)(struct WidgetState_ *opac);
	/* callback must be in the global structure
	 * but signals must be in this structure */
	Entity *signals;
	int type;
	int actionIdx;
} YWidgetState;

/* struct which define what are common to every rendableWidget of the same type */
struct widgetOpt {
	char *name;
	uint64_t rendersMask;
	int (*render[MAX_NB_MANAGER])(YWidgetState *wid, int renderType);
	int (*init[MAX_NB_MANAGER])(YWidgetState *opac, int t);
	void (*destroy[MAX_NB_MANAGER])(YWidgetState *opac, int t);
};

extern struct widgetOpt widgetOptTab[MAX_NB_MANAGER];

#define YEVE_FOREACH(curEve, eve)		\
	for(curEve = eve; curEve; curEve = ywidNextEve(curEve))

static inline Entity *ywidNextEve(Entity *e)
{
	return yeGet(e, YEVE_NEXT);
}

int ywidColorFromString(const char *str, uint8_t *r, uint8_t *g,
			uint8_t *b, uint8_t *a);

void ywidChangeResolution(int w, int h);

void ywidMarkAsDestroyable(YWidgetState *kboumable);

int ywidBgConfFill(Entity *entity, YBgConf *cfg);

static inline int ywidInitBgConf(Entity *wid, YBgConf *cfg)
{
	if (ywidBgConfFill(yeGet(wid, "background"), cfg) >= 0) {
		if (yeGetIntAt(wid, "background-stretch-height"))
			cfg->flag |= YBG_FIT_TO_SCREEN_H;
		else if (yeGetIntAt(wid, "background-stretch-width"))
			cfg->flag |= YBG_FIT_TO_SCREEN_W;
		return 1;
	}
	return 0;
}

#define ywidCastState(w, t) ((t *)ywidGetState(w))

static inline YWidgetState *ywidGetState(Entity *wid)
{
  return (YWidgetState *)yeGetData(yeGetByStrFast(wid, "$wid"));
}


/**
 * request renderer for an event
 * poll return NULL if no events
 * if non NULL yeDestoy need to be call on returned value
 */
Entity *ywidGenericWaitEvent(void);


/**
 * request renderer for an event
 * Wait for it
 * if non NULL yeDestoy need to be call on returned value
 */
Entity *ywidGenericPollEvent(void);

extern int ywidWindowWidth;
extern int ywidWindowHight;
extern int ywidXMouseGlobalPos;
extern int ywidYMouseGlobalPos;
extern int ywIsSmootOn;
extern int ywTurnPecent;

extern int ywTurnLengthOverwrite;

void ywSetTurnLengthOverwrite(int i);
int ywGetTurnLengthOverwrite(void);
unsigned int ywTurnId(void);

static inline int yWindowWidth(void)
{
	return ywidWindowWidth;
}

static inline int yWindowHeight(void)
{
	return ywidWindowHight;
}

static int ywWidth(Entity *w)
{
	return ywRectW(yeGet(w, "wid-pix"));
}

static int ywHeight(Entity *w)
{
	return ywRectH(yeGet(w, "wid-pix"));
}

/**
 * Registre a new type of widget
 * @return the type of the new type
 */
int ywidRegister(void *(*allocator)(void), const char *name);

int ywidUnregiste(int t);

int ywidRegistreRender(void (*resizePtr)(YWidgetState *wid, int renderType),
		       Entity *(*pollEvent)(void),
		       Entity *(*waitEvent)(void),
		       int (*draw)(void),
		       int (*changeResolution)(void),
		       void (*changeWinName)(const char *));

/**
 * check if widget is 'active' as main widget or its sub widget
 */
_Bool ywidInTree(Entity *widget);

void ywidRemoveRender(int renderType);

int ywidRegistreTypeRender(const char *type, int t,
			   int (*render)(YWidgetState *wid,
					 int renderType),
			   int (*init)(YWidgetState *opac, int t),
			   void (*destroy)(YWidgetState *opac, int t));

/* rename to push size */
void ywidResize(YWidgetState *wid);

/**
 * @brief interal function use to draw screen when the texture has been update
 */
int ywidDrawScreen(void);

static inline void ywidUpdate(Entity *w)
{
}

static inline void ywidMidRendEnd(YWidgetState *opac)
{
	if (unlikely(!opac))
		return;
	if (opac->midRendEnd) {
		opac->midRendEnd(opac);
	}
}

static inline int ywidSubRend(YWidgetState *opac)
{
	int ret = 0;
	if (opac->render) {
		ret = opac->render(opac);
	}
	return ret;
}

int ywidRend(YWidgetState *opac);

int ywidHandleEvent(YWidgetState *opac, Entity *event);

int ywidDoTurn(YWidgetState *opac);

#define ywidGenericCall(wid_, widType, func)				\
	YUI_FOREACH_BITMASK(widgetOptTab[widType].rendersMask,		\
			    ywidGenericCallIt, useless_tmask) {		\
		widgetOptTab[widType].func[ywidGenericCallIt]		\
			(wid_, ywidGenericCallIt);			\
	}

#define ywidGenericRend(wid_, widType, func)		\
	do {						\
		ywidGenericCall(wid_, widType, func);	\
	} while (0);

static inline int ywidType(void *opac)
{
  return ((YWidgetState *)opac)->type;
}

const char *ywidTypeName(YWidgetState *wid);

static inline void *ywidGetRenderData(YWidgetState *state, int t)
{
 return state->renderStates[t].opac;
}

/**
 * ywidAddSubType, but doesn't free subType
 */
int ywidAddSubTypeNF(Entity *subType);

/**
 * Register a new widget type, yeDestroy subType
 */
int ywidAddSubType(Entity *subType);

YWidgetState *ywidNewWidget(Entity *entity, const char *type);

void YWidDestroy(YWidgetState *wid);

int ywidNext(Entity *next, Entity *target);

void ywidSetMainWid(YWidgetState *wid);
YWidgetState *ywidGetMainWid(void);
void ywidFreeWidgets(void);

InputStatue ywidActions(Entity *wid, Entity *actionWid, Entity *eve);
InputStatue ywidAction(Entity *func, Entity *wid, Entity *event);
InputStatue ywidEventCallActionSin(YWidgetState *opac, Entity *event);

_Bool ywIsPixsOnWid(Entity *widget, int posX, int posY);

static inline _Bool ywIsMouseEveOnWid(Entity *w, Entity *eve)
{
	return ywIsPixsOnWid(w, ywidXMouse(eve), ywidYMouse(eve));
}

/*
 * that's kind of dirty to put that here, but he....
 * events handling have the problen of having been implemented first in
 * wdigets then in envents
 */
int yeveMouseX(void);
int yeveMouseY(void);

static inline _Bool ywIsMouseOnWid(Entity *w, Entity *events)
{
	return ywIsPixsOnWid(w, yeveMouseX(), yeveMouseY());
}

const char *ywidWindowName(void);
void ywidSetWindowName(const char *str);

static inline void ywidRendMainWid(void)
{
	ywidRend(ywidGetMainWid());
}

int ywidFontSize(void);
int ywidFontW(void);
int ywidFontH(void);

#endif
