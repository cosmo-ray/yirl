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
#include "game.h"
#include "timer.h"
#include "widget.h"
#include "rect.h"
#include "entity-script.h"

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
};

/* contain the options unique to one type of widget */
struct widgetOpt widgetOptTab[MAX_NB_MANAGER];
/* Contain the options unique to one type of render */
struct renderOpt renderOpTab[MAX_NB_MANAGER];

static YWidgetState *mainWid = NULL;
static YWidgetState *oldWid = NULL;

int ywidWindowWidth = 640;
int ywidWindowHight = 480;

void ywidChangeResolution(int w, int h)
{
  ywidWindowWidth = w;
  ywidWindowHight = h;
}

void ywidSetWindowName(const char *str)
{
  if (!configs)
    configs = yeCreateArray(NULL, NULL);
  yeCreateString(str, configs, "win-name");
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

int ywidNext(Entity *next)
{
  YWidgetState *newWid;
  const char *str = yeGetString(next);

  if (!next) {
    DPRINT_ERR("next is null");
    return -1;
  }

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

  if ((newWid = ywidNewWidget(next, NULL)) == NULL) {
    DPRINT_ERR("fail when creating new widget");
    return -1;
  }

  ywidSetMainWid(newWid);
  return 0;
}

int ywidColorFromString(char *str, uint8_t *r, uint8_t *g,
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
    char tmp;

    tmp = str[limiterPos];
    str[limiterPos] = '\0';

    if (yuiStrEqual(str, "rgba")) {
      char **rgba;
      int i;

      str += (limiterPos + 1);
      if (len < sizeof("rgba:r,g,b,a"))
	goto exit;

      rgba = g_strsplit_set(str, ", :", 5);
      for (i = 0; i < 4 && rgba[i] != NULL; ++i);

      if (i >= 4) {
	/* rgba[0] contain "rgba:" */
	*r = atoi(rgba[1]);
	*g = atoi(rgba[2]);
	*b = atoi(rgba[3]);
	*a = atoi(rgba[4]);
	ret = 0;
      } else {
	DPRINT_ERR("invalide rgba color string: %s", str);
      }

      g_strfreev(rgba);
    }

  exit:
    str -= (limiterPos + 1);
    str[limiterPos] = tmp;
  }
  return ret;
}

int ywidBgConfFill(Entity *entity, YBgConf *cfg)
{
  char *str = (char *)yeGetString(entity);

  cfg->type = BG_BUG;
  if (!str)
    return -1;

  if (!ywidColorFromString(str, &cfg->r, &cfg->g, &cfg->b, &cfg->a))
    cfg->type = BG_COLOR;
  else if ((cfg->path = g_strdup(str)) != NULL)
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
					   int shouldInit)
{
  YWidgetState *ret;
  Entity *pos = yeGet(entity, "wid-pos");
  Entity *initer = ygGetFuncExt(yeGetString(yeGet(entity, "init")));

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
    DPRINT_WARN("$wid alerady existe, this may lead to odd and very unexpected behavior !!!!");
  }
  yeReCreateData(ret, entity, "$wid");

  /* Init widget */
  if (ret->init(ret, entity, NULL)) {
    DPRINT_ERR("fail durring init\n");
    goto error;
  }

  /* Init sub widget */
  if (shouldInit && initer) {
    yesCall(initer, ret->entity, entity);
  }

  ret->hasChange = 1;
  ret->shouldDraw = 1;

  return ret;

 error:
  ret->destroy(ret);
  return NULL;
}

int ywidAddSubType(Entity *subType)
{
  if (!subTypes)
    subTypes = yeCreateArray(NULL, NULL);
  yePushBack(subTypes, subType, NULL);
  YE_DESTROY(subType);
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
      if (yuiStrEqual0(type, yeGetString(yeGet(tmpType, "name")))) {
	YWidgetState *ret = yesCall(yeGet(tmpType, "callback"), entity);

	if (!ret)
	  DPRINT_ERR("init for type '%s' fail", type);
	return ret;
      }
    }
  }

  for (int i = 0; i < 64; ++i) {
    if (widgetOptTab[i].name &&
	yuiStrEqual(type, widgetOptTab[i].name)) {
      return ywidNewWidgetInternal(i, entity, shouldInit);
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
  wid->hasChange = 1;
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
  printf("oh\n");
  if (renderType < 0) {
    printf("poum \n\n");
    return -1;
  }
  printf("I'm ready !\n");
  for (int i = 0; i < 64; ++i) {
    printf("%i\n", i);
    if (widgetOptTab[i].name && g_str_equal(type, widgetOptTab[i].name)) {
      widgetOptTab[i].rendersMask |= ONE64 << renderType;
      widgetOptTab[i].render[renderType] = render;
      widgetOptTab[i].init[renderType] = init;
      widgetOptTab[i].destroy[renderType] = destroy;
      widgetOptTab[i].midRend[renderType] = NULL;
      return i;
    }
  }
  return -1;
}

void ywidRegistreMidRend(void (*midRender)(YWidgetState *, int, int),
			 int widgetType, int renderType)
{
  widgetOptTab[widgetType].midRend[renderType] = midRender;
}

int ywidRegistreRender(void (*resizePtr)(YWidgetState *wid, int renderType),
		       Entity *(*pollEvent)(void),
		       Entity *(*waitEvent)(void),
		       int (*draw)(void))
{
  YUI_FOREACH_BITMASK(~rendersMask, i, tmask) {
    rendersMask |= (1 << i);
    renderOpTab[i].resizePtr = resizePtr;
    renderOpTab[i].pollEvent = pollEvent;
    renderOpTab[i].waitEvent = waitEvent;
    renderOpTab[i].draw = draw;
    return i;
  }
  return -1;
}


Entity *ywidGenericPollEvent(void)
{
  YUI_FOREACH_BITMASK(rendersMask, i, tmask) {
    Entity *ret = renderOpTab[i].pollEvent();
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
  if (!rendersMask)
    return NULL;
  if (YUI_COUNT_1_BIT(rendersMask) == 1) {
    return renderOpTab[YUI_GET_FIRST_BIT(rendersMask)].waitEvent();
  } else {
    Entity *ret;
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

void YWidDestroy(YWidgetState *wid)
{
  if (!wid)
    return;
  ywidGenericCall(wid, wid->type, destroy);
  if (wid->destroy)
    wid->destroy(wid);
  else
    g_free(wid);
}

static void ywidFreeEvents(Entity *event)
{
  yeDestroy(event);
}

int ywidDoTurn(YWidgetState *opac)
{
  int turnLength = yeGetInt(yeGet(opac->entity, "turn-length"));
  int ret;
  static YTimer *cnt = NULL;
  Entity *old = NULL;
  Entity *head;
  Entity *event;

  if (!cnt)
    cnt = YTimerCreate();

  if (turnLength > 0) {
    int64_t i = 0;
    uint16_t percent;

    i = turnLength - YTimerGet(cnt);
    percent = 100 - (i * 100 / turnLength);
    while (i > 0 && percent < 95) {
      int newPercent;

      ywidMidRend(mainWid, percent);
      do {
	usleep(1);
	i = turnLength - YTimerGet(cnt);
	newPercent = 100 - (i * 100 / turnLength);
      } while (newPercent == percent);
      percent = newPercent;
    }
    if (i > 0) {
      usleep(i);
    }
    YTimerReset(cnt);

    for (event = ywidGenericPollEvent(), head = event; event;
	 event = ywidGenericPollEvent()) {
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
  }

  ywidMidRendEnd(mainWid);
  ret = ywidHandleEvent(opac, head);
  ywidFreeEvents(head);
  return ret;
}

InputStatue ywidAction(Entity *action, Entity *wid, Entity *eve, Entity *arg)
{
  if (yeType(action) == YSTRING) {
    Entity *f = ygGet(yeGetString(action));
    InputStatue r = (InputStatue)yesCall(f, wid, eve, arg);

    if (unlikely(!ygIsInit())) {
      yeDestroy(f);
    }
    return r;
  } else if (yeType(action) == YFUNCTION) {
    return (InputStatue)yesCall(action, wid, eve, arg);
  } else {
    Entity *f = yeGet(action, 0);
    Entity *arg1 = yeLen(action) > 1 ? yeGet(action, 1) : Y_END_VA_LIST;
    Entity *arg2 = yeLen(action) > 2 ? yeGet(action, 2) : Y_END_VA_LIST;
    Entity *arg3 = yeLen(action) > 3 ? yeGet(action, 3) : Y_END_VA_LIST;

    if (yeType(f) == YSTRING)
      f = ygGet(yeGetString(f));
    InputStatue r = (InputStatue)yesCall(f, wid, eve, arg, arg1, arg2, arg3);
    if (unlikely(!ygIsInit() && yeType(f) == YSTRING)) {
      yeDestroy(f);
    }
    return r;
  }
}

InputStatue ywidActions(Entity *wid, Entity *actionWid, Entity *eve, void *arg)
{
  Entity *actions = yeGet(actionWid, "action");
  if (actions)
    return ywidAction(actions, wid, eve, arg);
  actions = yeGet(actionWid, "actions");
  InputStatue ret = NOTHANDLE;

  if (!actions)
    return NOTHANDLE;
  switch (yeType(actions)) {
  case YSTRING:
  case YFUNCTION:
    return ywidAction(actions, wid, eve, arg);
  case YARRAY:
    {
      YE_ARRAY_FOREACH(actions, action) {
	int cur_ret = ywidAction(action, wid, eve, arg);

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
  return ywidActions(opac->entity, opac->entity, event, NULL);
}

int ywidHandleEvent(YWidgetState *opac, Entity *event)
{
  int ret = 0;
  Entity *postAction;

  if (opac->handleEvent)
    ret = opac->handleEvent(opac, event);

  if (!opac->hasChange)
    opac->hasChange = (ret == NOTHANDLE ? 0 : 1);
  else
    ret = (ret == NOTHANDLE ? NOACTION : ret);
  if ((postAction = yeGet(opac->entity, "post-action")) != NULL)
    yesCall(postAction, ret, opac->entity, event);
  return ret;
}

int ywIsPixsOnWid(Entity *widget, int posX, int posY)
{
  Entity *pixR = yeGet(widget, "wid-pix");

  return posX > ywRectX(pixR) && posX < (ywRectX(pixR) + ywRectW(pixR)) &&
    posY > ywRectY(pixR) && posY < (ywRectY(pixR) + ywRectH(pixR));
}
