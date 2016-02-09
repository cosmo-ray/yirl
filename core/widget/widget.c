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
#include <glib.h>
#include <stdlib.h>
#include "timer.h"
#include "widget.h"
#include "widget-callback.h"

static YManagerAllocator widgetTab = {
  {NULL },
  0
};


/* struct which define what are common to every rendableWidget of the same type */
struct widgetOpt {
  char *name;
  uint64_t rendersMask;
  int (*render[MAX_NB_MANAGER])(YWidgetState *wid, int renderType);
  int (*init[MAX_NB_MANAGER])(YWidgetState *opac, int t);
  void (*destroy[MAX_NB_MANAGER])(YWidgetState *opac, int t);
};

static uint64_t rendersMask = 0;

/* struct which define what are common to every render of the same type */
struct renderOpt {
  YEvent *(*waitEvent)(void);
  YEvent *(*pollEvent)(void);
  void (*resizePtr) (YWidgetState *wid, int renderType);
};

/* contain the options unique to one type of widget */
struct widgetOpt widgetOptTab[MAX_NB_MANAGER];
/* Contain the options unique to one type of render */
struct renderOpt renderOpTab[MAX_NB_MANAGER];

static YWidgetState *mainWid = NULL;
static YWidgetState *oldWid = NULL;

void ywidFreeWidgets(void)
{
  YWidDestroy(mainWid);
  YWidDestroy(oldWid);
  mainWid = NULL;
  oldWid = NULL;
}

inline YWidgetState *ywidGetMainWid(void)
{
  return mainWid;
}

void ywidSetMainWid(YWidgetState *wid)
{
  YWidDestroy(oldWid);
  oldWid = mainWid;
  mainWid = wid;
}

int ywidNext(Entity *next)
{
  YWidgetState *newWid;

  if (!next) {
    DPRINT_ERR("next is null");
    return -1;
  }

  if ((newWid = ywidNewWidget(next, NULL)) == NULL) {
    DPRINT_ERR("fail when creating new widget");
    return -1;
  }

  ywidSetMainWid(newWid);
  return 0;
}

int ywidBgConfFill(Entity *entity, YBgConf *cfg)
{
  char *str = (char *)yeGetString(entity);
  size_t len;
  int limiterPos = 0;
  int ret = -1;

  cfg->type = BG_BUG;
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
    char tmp;

    tmp = str[limiterPos];
    str[limiterPos] = '\0';

    if (yuiStrEqual(str, "rgba")) {
      char **rgba;
      int i;

      str += (limiterPos + 1);
      if (len < sizeof("r,g,b,q"))
	goto exit;

      rgba = g_strsplit_set(str, ", :", 5);
      for (i = 0; i < 4 && rgba[i] != NULL; ++i);

      if (i >= 4) {
	cfg->r = atoi(rgba[0]);
	cfg->g = atoi(rgba[1]);
	cfg->b = atoi(rgba[2]);
	cfg->a = atoi(rgba[3]);
	cfg->type = BG_COLOR;
	ret = 0;

      } else {
	DPRINT_ERR("invalide rgba color string: %s", str);
      }

      g_strfreev(rgba);
    }

  exit:
    str -= (limiterPos + 1);
    str[limiterPos] = tmp;

  } else { // path
    if ((cfg->path = g_strdup(str)) != NULL) {
      cfg->type = BG_IMG;
      ret = 0;
    }
  }

  return ret;
}

void ywidRemoveRender(int renderType)
{
  rendersMask ^= (1LLU << renderType);
  renderOpTab[renderType].resizePtr = NULL;
  for(int i = 0; i < MAX_NB_MANAGER; ++i) {
    widgetOptTab[i].rendersMask ^= (1LLU << renderType);    
    widgetOptTab[i].init[renderType] = NULL;
    widgetOptTab[i].render[renderType] = NULL;
    widgetOptTab[i].render[renderType] = NULL;
  }
}

int ywidRegister(void *(*allocator)(void), const char *name)
{
  int ret = yuiRegister(&widgetTab, allocator);
  if (ret < 0)
    return -1;

  widgetOptTab[ret].name = g_strdup(name);
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
  g_free(widgetOptTab[t].name);
  widgetOptTab[t].name = NULL;
  return yuiUnregiste(&widgetTab, t);
}


static YWidgetState *ywidNewWidgetInternal(int t,
					   Entity *entity,
					   void *args)
{
  YWidgetState *ret;
  Entity *pos= yeGet(entity, "pos");

  if (widgetTab.len <= t || widgetTab.allocator[t] == NULL)
    return NULL;

  if (pos == NULL) {
    pos = yeCreateArray(entity, "pos");
    yeCreateInt(0, pos, "x");
    yeCreateInt(0, pos, "y");
    yeCreateInt(1000, pos, "w");
    yeCreateInt(1000, pos, "h");
  }

  ret = widgetTab.allocator[t]();

  if (ret == NULL)
    return NULL;

  ret->entity = entity;
  ret->signals = g_array_new(1, 1, sizeof(YSignal *));


  if (ret->init(ret, entity, args))
    goto error;

  ret->hasChange = 1;
  return ret;

 error:
  ret->destroy(ret);
  return NULL;
}

YWidgetState *ywidNewWidget(Entity *entity, void *args)
{
  Entity *tmp;

  if (!entity)
    return NULL;

  tmp = yeGet(entity, "<type>");

  if (!tmp)
    return NULL;

  for (int i = 0; i < 64; ++i) {
    if (widgetOptTab[i].name &&
	yuiStrEqual(yeGetString(tmp), widgetOptTab[i].name)) {
      return ywidNewWidgetInternal(i, entity, args);
    }
  }
  return NULL;
}

void ywidResize(YWidgetState *wid)
{
  YUI_FOREACH_BITMASK(rendersMask, i, tmask) {
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
  for (int i = 0; i < 64; ++i) {
    if (widgetOptTab[i].name && g_str_equal(type, widgetOptTab[i].name)) {
      widgetOptTab[i].rendersMask |= 1LLU << renderType;
      widgetOptTab[i].render[renderType] = render;
      widgetOptTab[i].init[renderType] = init;
      widgetOptTab[i].destroy[renderType] = destroy;
      return i;
    }
  }
  return -1;
}

int ywidRegistreRender(void (*resizePtr)(YWidgetState *wid, int renderType),
		       YEvent *(*pollEvent)(void),
		       YEvent *(*waitEvent)(void))
{
  YUI_FOREACH_BITMASK(~rendersMask, i, tmask) {
    rendersMask |= (1 << i);
    renderOpTab[i].resizePtr = resizePtr;
    renderOpTab[i].pollEvent = pollEvent;
    renderOpTab[i].waitEvent = waitEvent;
    return i;
  }
  return -1;
}


YEvent *ywidGenericPollEvent(void)
{
  YUI_FOREACH_BITMASK(rendersMask, i, tmask) {
    YEvent *ret = renderOpTab[i].pollEvent();
    if (ret)
      return ret;
  }
  return NULL;
}

YEvent *ywidGenericWaitEvent(void)
{
  if (!rendersMask)
    return NULL;
  if (YUI_COUNT_1_BIT(rendersMask) == 1) {
    return renderOpTab[YUI_GET_FIRST_BIT(rendersMask)].waitEvent();
  } else {
    YEvent *ret;
    while (1) {
      YUI_FOREACH_BITMASK(rendersMask, i, tmask) {
	if (renderOpTab[i].pollEvent && (ret = renderOpTab[i].pollEvent()))
	  return ret;
      }
      usleep(50);
    }
  }
  return NULL;
}

int ywidGenericRend(YWidgetState *opac, int widType)
{
  YUI_FOREACH_BITMASK(widgetOptTab[widType].rendersMask,
		      i, tmask) {
    if (widgetOptTab[widType].render[i] != NULL) {
      widgetOptTab[widType].render[i](opac, i);
    }
  }
  return 0;
}

int ywidGenericInit(YWidgetState *opac, int widType)
{
  YUI_FOREACH_BITMASK(widgetOptTab[widType].rendersMask,
		      i, tmask) {
    if (widgetOptTab[widType].init[i] != NULL) {
      widgetOptTab[widType].init[i](opac, i);
    }
  }
  return 0;
}

static int ywidGenericDestroy(YWidgetState *opac, int widType)
{
  ywidFinishSignal(opac);
  YUI_FOREACH_BITMASK(widgetOptTab[widType].rendersMask,
		      i, tmask) {
    if (widgetOptTab[widType].destroy[i] != NULL) {
      widgetOptTab[widType].destroy[i](opac, i);
    }
  }
  return 0;
}

void YWidDestroy(YWidgetState *wid)
{
  if (!wid)
    return;
  ywidGenericDestroy(wid, wid->type);
  if (wid->destroy)
    wid->destroy(wid);
  else
    g_free(wid);
}

static void ywidFreeEvents(YEvent *event)
{
  struct EveListHead *head;

  if (!event)
    return;
  head = event->head;
  while (!SLIST_EMPTY(head)) {
    event = SLIST_FIRST(head);
    SLIST_REMOVE_HEAD(head, lst);
    free(event);
  }
}

int ywidDoTurn(YWidgetState *opac)
{
  YEvent *event;
  int turnLength = yeGetInt(yeGet(opac->entity, "turn-length"));
  int ret;
  struct EveListHead head = SLIST_HEAD_INITIALIZER(head);
  static YTimer *cnt = NULL;

  if (!cnt)
    cnt = YTimerCreate();

  if (turnLength > 0) {
    int64_t i = 0;

    i = turnLength - YTimerGet(cnt);
    if (i > 0)
      usleep(i);
    YTimerReset(cnt);

    for (event = ywidGenericPollEvent(); event;
	 event = ywidGenericPollEvent()) {
      event->head = &head;
      SLIST_INSERT_HEAD(&head, event, lst);
      ++i;
    }

  } else {
    event = ywidGenericWaitEvent();
    if (!event)
      return NOTHANDLE;
    event->head = &head;
    SLIST_INSERT_HEAD(&head, event, lst);
  }

  ret = ywidHandleEvent(opac, SLIST_FIRST(&head));
  ywidFreeEvents(SLIST_FIRST(&head));
  return ret;
}
