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
#include <sched.h>
#include "game.h"

/* description */
#include "json-desc.h"
#include "description.h"

/* scripting */
#include "lua-script.h"

#include "widget-callback.h"
#include "utils.h"
#include "entity.h"
#include "curses-driver.h"
#include "sdl-driver.h"
#include "menu.h"
#include "map.h"
#include "text-screen.h"


static int init;
static void *jsonManager;
static void *luaManager;

static YDescriptionOps *parsers[MAX_NB_MANAGER];


#define CHECK_AND_RET(operation, err_val, ret, fmt, args...) do {	\
    if ((operation) == (err_val)) {					\
      DPRINT_ERR(fmt, ## args);						\
      return (ret);							\
    }									\
  } while (0)

#define CHECK_AND_GOTO(operation, err_val, label, fmt, args...) do { \
    if ((operation) == (err_val)) {					\
      DPRINT_ERR(fmt, ## args);						\
      goto label;							\
    }									\
  } while (0)

#define TO_RC(X) ((RenderConf *)(X))

static int alive = 1;

int ygTerminateCallback(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  (void)wid;
  (void)eve;
  (void)arg;
  alive = 0;
  return ACTION;
}

int ygInit(GameConfig *cfg)
{
  static int t;

  /* Init parseurs */
  CHECK_AND_RET(t = ydJsonInit(), -1, -1,
		    "json init failed");
  CHECK_AND_RET(jsonManager = ydNewManager(t), NULL, -1,
		    "json init failed");
  parsers[t] = jsonManager; 

  /* Init scripting */
  /* TODO init internal lua function */
  CHECK_AND_RET(t = ysLuaInit(), -1, -1, "lua init failed");
  CHECK_AND_RET(luaManager = ysNewManager(NULL, t), NULL, -1,
		    "lua init failed");

  /* Init widgets */
  CHECK_AND_RET(ywidInitCallback(), -1, -1, "can not init callback");
  CHECK_AND_RET(ywinAddCallback(ywinCreateNativeCallback("FinishGame",
							 ygTerminateCallback)),
		-1, -1, "can not add game's callback");
  CHECK_AND_RET(ywMenuInit(), -1, -1, "Menu init failed");
  CHECK_AND_RET(ywMapInit(), -1, -1, "Map init failed");
  CHECK_AND_RET(ywTextScreenInit(), -1, -1, "Text Screen init failed");

  for (GList *tmp = cfg->rConf; tmp; tmp = tmp->next) {
    //TODO check which render to use :)
    if (yuiStrEqual(TO_RC(tmp->data)->name, "curses")) {
#ifdef WITH_CURSES
      ycursInit();
      CHECK_AND_RET(ycursRegistreMenu(), -1, -1, "Menu init failed");
      CHECK_AND_RET(ycursRegistreTextScreen(), -1, -1,
			"Text Screen init failed");
      CHECK_AND_RET(ycursRegistreMap(), -1, -1, "Map init failed");
#else
      /* print error */
#endif
    } else if (yuiStrEqual(TO_RC(tmp->data)->name, "sdl2")) {
#ifdef WITH_SDL
      ysdl2Init();
      CHECK_AND_RET(ysdl2RegistreTextScreen(), -1, -1,
			"Text Screen init failed");
      CHECK_AND_RET(ysdl2RegistreMenu(), -1, -1, "Menu init failed");
#else
      /* print error */
#endif
    }
  }
  init = 1;
  return 0;
}

#undef TO_RC

void ygEnd()
{
  ydDestroyManager(jsonManager);
  ydJsonEnd();
  ywTextScreenEnd();
  ywMapEnd();
  ywMenuEnd();
#ifdef WITH_CURSES
  ycursDestroy();
#endif
#ifdef WITH_SDL
  ysdl2Destroy();
#endif
  ywidFinishCallbacks();
  init = 0;
}

static inline int checkStartingPoint(Entity *type, Entity *file,
				     Entity *starting_widget)
{
  CHECK_AND_RET(file, NULL, -1, "No file in start");
  CHECK_AND_RET(type, NULL, -1, "No type in start");
  CHECK_AND_RET(starting_widget, NULL, -1, "No starting widget in start");
  if (yeType(type) != YSTRING ||
      yeType(file) != YSTRING ||
      yeType(starting_widget) != YSTRING) {
    DPRINT_ERR("Bad entity type in start, all type's should be strigs");
    return -1;
  }

  
  return 0;
}

static int ygParseStartAndGame(GameConfig *config, Entity *mainMod)
{
  Entity *type = yeGet(mainMod, "type");
  Entity *file = yeGet(mainMod, "file");
  Entity *starting_widget = yeGet(mainMod, "starting widget");
  YWidgetState *wid;
  int ret = ACTION;

  (void)ret;
  alive = 1;
  if (checkStartingPoint(type, file, starting_widget) < 0)
    return -1;

  if (yuiStrEqual(yeGetString(type), "json")) {
    char *tmp = NULL;

    tmp = g_strconcat(config->startingMod->path, "/", yeGetString(file), NULL);
    file = ydFromFile(jsonManager, tmp);
    g_free(tmp);
    if (!file) {
      return -1;
    }

    starting_widget = yeGet(file, yeGetString(starting_widget));
  } else {
    DPRINT_ERR("start does not suport loader of type %s", yeGetString(type));
    return -1;
  }

  wid = ywidNewWidget(starting_widget, NULL, NULL);
  do {
    g_assert(ywidRend(wid) != -1);
    sched_yield();
    ywidHandleEvent(wid);
  } while(alive);

  return 0;
}

int ygStartLoop(GameConfig *config)
{
  char *tmp;
  Entity *mainMod;
  int ret = -1;

  if (!init) {
        DPRINT_ERR("ygInit() should be call before calling ygStartLoop");
    return -1;
  }

  if (!config || !config->rConf || !config->startingMod) {
    DPRINT_ERR("GameConfig is brocken(some ptr are NULL)");
    return -1;
  }

  if (!config->startingMod->path) {
    DPRINT_ERR("!config->startingMod->path is NULL");
    return -1;
  }

  tmp = g_strconcat(config->startingMod->path, "/start.json", NULL);
  if (!tmp) {
    DPRINT_ERR("can not allocated path(like something went really wrong)");
    return -1;
  }

  mainMod = ydFromFile(jsonManager, tmp);
  if (!mainMod) {
    DPRINT_ERR("fail to parse file: %s", tmp);
    goto cleanup;
  }
  
  ret = ygParseStartAndGame(config, mainMod);
 cleanup:
  g_free(tmp);
  YE_DESTROY(mainMod);
  return ret;
}

static const char *sdl2 = "sdl2";
static const char *curses = "curses";

int ygInitGameConfig(GameConfig *cfg, const char *path, RenderType t)
{
  if (!t) {
    return -1;
  }

  cfg->rConf = NULL;
  cfg->startingMod = g_new(ModuleConf, 1);
  cfg->startingMod->path = path;

  YUI_FOREACH_BITMASK(t, i, tmp) {
    RenderConf *rConf = g_new(RenderConf, 1);

    if (1LLU << i ==  SDL2)
      rConf->name = sdl2;
    else if (1LLU << i == CURSES)
      rConf->name = curses;
    else {
      DPRINT_ERR("garbage Render Type type in ygInitGameConfig");
      ygCleanGameConfig(cfg);
      return -1;
    }
    cfg->rConf = g_list_append(cfg->rConf,
			       rConf);
  }
  return 0;
}

void ygCleanGameConfig(GameConfig *cfg)
{
  g_free(cfg->startingMod);
  g_list_free(cfg->rConf);
}

#undef CHECK_AND_GOTO
#undef CHECK_AND_RET
