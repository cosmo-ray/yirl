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

#include <glib.h>
#include <stdint.h>
#include <sys/queue.h>
#include "entity.h"
#include "utils.h"
#include "keydef.h"

typedef enum
  {
    BUG = -1,
    NOTHANDLE = 0,
    NOACTION = 1,
    ACTION = 2,
  } InputStatue;

typedef enum
  {
    YKEY_DOWN,
    YKEY_UP,
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
struct yevent;

typedef struct yevent {
  EventType type;
  int key;
  unsigned int xMouse;
  unsigned int yMouse;
  InputStatue stat;
  SLIST_HEAD(EveListHead, yevent) *head;
  SLIST_ENTRY(yevent) lst;
} YEvent;

typedef struct {
  void *opac;
} YRenderState;

typedef struct WidgetState_ {
  Entity *entity;
  YRenderState renderStates[64];
  int (*render)(struct WidgetState_ *opac);
  InputStatue (*handleEvent)(struct WidgetState_ *opac, YEvent *event);
  InputStatue (*handleAnim)(struct WidgetState_ *opac);
  void (*resize)(void);
  int (*init)(struct WidgetState_ *opac, Entity *entity, void *args);
  int (*destroy)(struct WidgetState_ *opac);
  /* callback must be in the global structure
   * but signals must be in this structure */
  Entity *signals;
  int type;
  unsigned int hasChange;
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
  if (eve)					\
    SLIST_FOREACH(curEve, eve->head, lst)

#define ywidNextEve(eve)			\
  (SLIST_NEXT(eve, lst))

int ywidBgConfFill(Entity *entity, YBgConf *cfg);

YEvent *ywidGenericWaitEvent(void);
YEvent *ywidGenericPollEvent(void);

/**
 * Registre a new type of widget
 * @return the type of the new type
 */
int ywidRegister(void *(*allocator)(void), const char *name);

int ywidUnregiste(int t);

int ywidRegistreRender(void (*resizePtr)(YWidgetState *wid, int renderType),
		       YEvent *(*pollEvent)(void),
		       YEvent *(*waitEvent)(void));
void ywidRemoveRender(int renderType);

int ywidRegistreTypeRender(const char *type, int t,
			   int (*render)(YWidgetState *wid,
					 int renderType),
			   int (*init)(YWidgetState *opac, int t),
			   void (*destroy)(YWidgetState *opac, int t));

/* rename to push size */
void ywidResize(YWidgetState *wid);

static inline int ywidRend(YWidgetState *opac)
{
  if (opac->render)
    return (opac->render(opac));
  return -1;
}

static inline int ywidHandleEvent(YWidgetState *opac, YEvent *event)
{
  if (opac->handleEvent)
    return (opac->handleEvent(opac, event));
  return -1;
}

int ywidHandleAnim(YWidgetState *opac);

int ywidDoTurn(YWidgetState *opac);

#define ywidGenericCall(opac, widType, func)				\
  YUI_FOREACH_BITMASK(widgetOptTab[widType].rendersMask,		\
		      ywidGenericCallIt, useless_tmask) {		\
    widgetOptTab[widType].func[ywidGenericCallIt](opac, ywidGenericCallIt); \
  }

static inline int ywidType(void *opac)
{
  return ((YWidgetState *)opac)->type;
}

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


#endif
