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
    NOACTION = 1,
    ACTION = 2,
  } InputStatue;

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
    YKEY_MOUSEWHEEL,
    YKEY_MOUSEMOTION,
    YKEY_NONE
  } EventType;

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

  };

} YBgConf;

struct WidgetState_;

/* typedef struct yevent { */
/*   EventType type; */
/*   int key; */
/*   unsigned int xMouse; */
/*   unsigned int yMouse; */
/*   InputStatue stat; */
/*   SLIST_HEAD(EveListHead, yevent) *head; */
/*   SLIST_ENTRY(yevent) lst; */
/* } YEvent; */

enum {
    YEVE_TYPE = 0,
    YEVE_KEY = 1,
    YEVE_MOUSE = 2,
    YEVE_STATUS = 15,
    YEVE_NEXT = 16
};

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
  void (*midRend)(struct WidgetState_ *opac, int turnPercent);
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
  unsigned int hasChange;
  unsigned int shouldDraw;
} YWidgetState;

/* struct which define what are common to every rendableWidget of the same type */
struct widgetOpt {
  char *name;
  uint64_t rendersMask;
  int (*render[MAX_NB_MANAGER])(YWidgetState *wid, int renderType);
  void (*midRend[MAX_NB_MANAGER])(struct WidgetState_ *opac, int t,
				  int turnPercent);
  int (*init[MAX_NB_MANAGER])(YWidgetState *opac, int t);
  void (*destroy[MAX_NB_MANAGER])(YWidgetState *opac, int t);
};

extern struct widgetOpt widgetOptTab[MAX_NB_MANAGER];

#define YEVE_FOREACH(curEve, eve)		\
  for(curEve = eve; curEve; curEve = ywidNextEve(curEve))

#define ywidNextEve(eve)			\
	(yeGet(eve, YEVE_NEXT))

int ywidColorFromString(char *str, uint8_t *r, uint8_t *g,
			uint8_t *b, uint8_t *a);

int ywidBgConfFill(Entity *entity, YBgConf *cfg);

static inline YWidgetState *ywidGetState(Entity *wid)
{
  return (YWidgetState *)yeGetData(yeGetByStrFast(wid, "$wid"));
}

Entity *ywidGenericWaitEvent(void);
Entity *ywidGenericPollEvent(void);

extern int ywidWindowWidth;
extern int ywidWindowHight;

/**
 * Registre a new type of widget
 * @return the type of the new type
 */
int ywidRegister(void *(*allocator)(void), const char *name);

int ywidUnregiste(int t);

int ywidRegistreRender(void (*resizePtr)(YWidgetState *wid, int renderType),
		       Entity *(*pollEvent)(void),
		       Entity *(*waitEvent)(void),
		       int (*draw)(void));
void ywidRegistreMidRend(void (*midRender)(YWidgetState *, int, int),
			 int widgetType, int renderType);

void ywidRemoveRender(int renderType);

int ywidRegistreTypeRender(const char *type, int t,
			   int (*render)(YWidgetState *wid,
					 int renderType),
			   int (*init)(YWidgetState *opac, int t),
			   void (*destroy)(YWidgetState *opac, int t));

/* rename to push size */
void ywidResize(YWidgetState *wid);

static inline void ywidMidRend(YWidgetState *opac, int turnPercent)
{
  if (opac->midRend) {
    opac->midRend(opac, turnPercent);
  }
}

static inline void ywidMidRendEnd(YWidgetState *opac)
{
  if (opac && opac->midRendEnd) {
    opac->midRendEnd(opac);
  }
}

static inline int ywidRend(YWidgetState *opac)
{
  int ret = 0;
  if (opac->render && opac->hasChange) {
    ret = opac->render(opac);
    opac->hasChange = 0;
  }
  return ret;
}

/**
 * @brief interal function use to draw screen when the texture has been update
 */
int ywidDrawScreen(void);

int ywidHandleEvent(YWidgetState *opac, Entity *event);

int ywidDoTurn(YWidgetState *opac);

#define ywidGenericCall(wid_, widType, func)				\
  YUI_FOREACH_BITMASK(widgetOptTab[widType].rendersMask,		\
		      ywidGenericCallIt, useless_tmask) {		\
    widgetOptTab[widType].func[ywidGenericCallIt](wid_, ywidGenericCallIt); \
  }

#define ywidGenericRend(wid_, widType, func) do {	\
    ywidGenericCall(wid_, widType, func);		\
    if (wid_->shouldDraw) {ywidDrawScreen();}		\
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

int ywidAddSubType(Entity *subType);

YWidgetState *ywidNewWidget(Entity *entity,
			    const char *type);

void YWidDestroy(YWidgetState *wid);

int ywidNext(Entity *next);

void ywidSetMainWid(YWidgetState *wid);
YWidgetState *ywidGetMainWid(void);
void ywidFreeWidgets(void);

InputStatue ywidAction(Entity *func, Entity *wid, Entity *event, Entity *arg);
InputStatue ywidEventCallActionSin(YWidgetState *opac, Entity *event);

int ywIsPixsOnWid(Entity *widget, int posX, int posY);

const char *ywidWindowName(void);
void ywidSetWindowName(const char *str);

#endif
