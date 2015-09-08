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
#include "widget.h"

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

int ywidBgConfFill(Entity *cfg, YBgConf *cfg)
{
  char *str = (char *)yeGetString(cfg);
  char tmp;
  size_t len;
  ret = -1;

  if (!str)
    return -1;
  len = strlen(str);
  tmp = str[5];
  str[5] = '\0';

  if (yuiStrEqual(str, "rgba:")) {
    ret = 0;
  }

  str[5] = tmp;
  if (!ret) {
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

static int copyPos(YWidPos *dst, const YWidPos *src)
{
  if (!dst || !src)
    return -1;
  memcpy(dst, src, sizeof(YWidPos));
  return 0;
}


static YWidgetState *ywidNewWidgetInternal(int t,
					   Entity *entity,
					   const YWidPos *pos,
					   void *args)
{
  YWidgetState *ret;
  YWidPos defaultPos = {0,0,1000,1000};

  if (widgetTab.len <= t || widgetTab.allocator[t] == NULL)
    return NULL;
  if (pos == NULL)
    pos = &defaultPos;
  ret = widgetTab.allocator[t]();
  if (ret == NULL)
    return NULL;
  ret->type = t;
  if (copyPos(&ret->pos, pos) == -1)
    goto error;
  if (ret->init(ret, entity, args))
    goto error;
  return ret;
 error:
  ret->destroy(ret);
  return NULL;
}

YWidgetState *ywidNewWidget(Entity *entity, const YWidPos *pos, void *args)
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
      return ywidNewWidgetInternal(i, entity, pos, args);
    }
  }
  return NULL;
}

void ywidResize(YWidgetState *wid)
{
  for (int i =0; i < 64; ++i) {
    if ((1 << i) & rendersMask)
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
  if (YUI_COUNT_1_BYTE(rendersMask) == 1) {
    return renderOpTab[YUI_GET_FiRST_BYTE(rendersMask)].waitEvent();
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
  ywidGenericDestroy(wid, wid->type);
  if (wid->destroy)
    wid->destroy(wid);
  else
    g_free(wid);
}
