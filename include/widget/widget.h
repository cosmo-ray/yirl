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
#ifndef	_WIDGET_H_
#define	_WIDGET_H_

#include <stdint.h>
#include "entity.h"
#include "utils.h"
#include "keydef.h"

typedef enum
  {
    NOTHANDLE,
    NOACTION,
    ACTION
  } InputStatue;

typedef enum
  {
    YKEY_DOWN,
    YKEY_UP,
    NONE
  } EventType;

struct WidgetState_;
typedef struct {
  EventType type;
  int key;
  unsigned int xMouse;
  unsigned int yMouse;
  InputStatue stat;
} YEvent;


typedef struct {
  void *opac;
} YRenderState;

typedef struct {
  unsigned int x;
  unsigned int y;
  unsigned int w;
  unsigned int h;
} YWidPos;

typedef struct WidgetState_ {
  Entity *entity;
  YRenderState renderStates[64];
  int (*render)(struct WidgetState_ *opac);
  InputStatue (*handleEvent)(struct WidgetState_ *opac, YEvent *event);
  void (*resize)(void);
  int (*init)(struct WidgetState_ *opac, Entity *entity, void *args);
  int (*destroy)(struct WidgetState_ *opac);
  YWidPos pos;
  int type;
} YWidgetState;


YEvent *ywidGenericWaitEvent(void);
YEvent *ywidGenericPollEvent(void);

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

void ywidResize(YWidgetState *wid);

static inline int ywidRend(YWidgetState *opac)
{
  if (opac->render)
    return (opac->render(opac));
  return -1;
}

static inline int ywidHandleEvent(YWidgetState *opac)
{
  if (opac->handleEvent)
    return (opac->handleEvent(opac, NULL));
  return -1;
}

int ywidGenericRend(YWidgetState *opac, int widType);
int ywidGenericInit(YWidgetState *opac, int widType);


static inline int ywidType(void *opac)
{
  return ((YWidgetState *)opac)->type;
}

static inline void *ywidGetRenderData(YWidgetState *state, int t)
{
 return state->renderStates[t].opac;
}

YWidgetState *ywidNewWidget(int t, Entity *entity,
			    const YWidPos *pos, void *args);

void YWidDestroy(YWidgetState *wid);

#endif
