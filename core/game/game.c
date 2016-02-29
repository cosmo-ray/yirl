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
#include "lua-binding.h"

/* widgets */
#include "widget-callback.h"
#include "utils.h"
#include "entity.h"
#include "curses-driver.h"
#include "sdl-driver.h"
#include "menu.h"
#include "map.h"
#include "text-screen.h"
#include "contener.h"


static int init;
static void *jsonManager;
static void *luaManager;
static void *tccManager;
static Entity *mainMod;

static YDescriptionOps *parsers[MAX_NB_MANAGER];

static int alive = 1;

static const char *sdl2 = "sdl2";
static const char *curses = "curses";

void *ygGetLuaManager(void)
{
  return luaManager;
}

void *ygGetTccManager(void)
{
  return tccManager;
}

#define CHECK_AND_RET(operation, err_val, ret, fmt, args...) do {	\
    if ((operation) == (err_val)) {					\
      DPRINT_ERR(fmt, ## args);						\
      return (ret);							\
    }									\
  } while (0)

#define CHECK_AND_GOTO(operation, err_val, label, fmt, args...) do {    \
    if ((operation) == (err_val)) {					\
      DPRINT_ERR(fmt, ## args);						\
      goto label;							\
    }									\
  } while (0)

#define TO_RC(X) ((RenderConf *)(X))

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

  /* trick use in case of failure in thbis function to free all */
  init = 1;
  /* Init parseurs */
  CHECK_AND_RET(t = ydJsonInit(), -1, -1,
		    "json init failed");
  CHECK_AND_GOTO(jsonManager = ydNewManager(t), NULL, error,
		    "json init failed");
  parsers[t] = jsonManager;

  /* Init scripting */
  /* TODO init internal lua function */
  CHECK_AND_GOTO(t = ysLuaInit(), -1, error, "lua init failed");
  CHECK_AND_GOTO(luaManager = ysNewManager(NULL, t), NULL, error,
		    "lua init failed");

  CHECK_AND_GOTO(yesLuaRegister(luaManager), -1, error, "lua init failed");

  /* Init widgets */
  CHECK_AND_GOTO(ywidInitCallback(), -1, error, "can not init callback");
  CHECK_AND_GOTO(ywinAddCallback(ywinCreateNativeCallback("FinishGame",
							 ygTerminateCallback)),
		 -1, error, "cannot add game's callback");
  CHECK_AND_GOTO(ywMenuInit(), -1, error, "Menu init failed");
  CHECK_AND_GOTO(ywMapInit(), -1, error, "Map init failed");
  CHECK_AND_GOTO(ywTextScreenInit(), -1, error, "Text Screen init failed");
  CHECK_AND_GOTO(ywContenerInit(), -1, error, "Contener init failed");

  /* Init sound */
  sound_init(LIB_VLC);

  for (GList *tmp = cfg->rConf; tmp; tmp = tmp->next) {
    //TODO check which render to use :)
    if (yuiStrEqual(TO_RC(tmp->data)->name, "curses")) {
#ifdef WITH_CURSES
      ycursInit();
      CHECK_AND_GOTO(ycursRegistreMenu(), -1, error, "Menu init failed");
      CHECK_AND_GOTO(ycursRegistreTextScreen(), -1, error,
			"Text Screen init failed");
      CHECK_AND_GOTO(ycursRegistreMap(), -1, error, "Map init failed");
#else
      DPRINT_ERR("yirl is not compille with curses support");
#endif
    } else if (yuiStrEqual(TO_RC(tmp->data)->name, "sdl2")) {
#ifdef WITH_SDL
      ysdl2Init();
      CHECK_AND_GOTO(ysdl2RegistreTextScreen(), -1, error,
			"Text Screen init failed");
      CHECK_AND_GOTO(ysdl2RegistreMenu(), -1, error, "Menu init failed");
      CHECK_AND_GOTO(ysdl2RegistreMap(), -1, error, "Map init failed");
#else
      DPRINT_ERR("yirl is not compille with SDL2 support");
#endif
    }
  }
  return 0;
error:
  ygEnd();
  return -1;
}

#undef TO_RC

void ygEnd()
{
  if (!init)
    return;

  ywidFreeWidgets();
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

Entity *ygLoadMod(const char *path)
{
  char *tmp;
  Entity *mod;
  Entity *type;
  Entity *file;
  Entity *starting_widget;
  Entity *preLoad;
  Entity *initScripts;

  tmp = g_strconcat(path, "/start.json", NULL);
  if (!tmp) {
    DPRINT_ERR("cannot allocated path(like something went really wrong)");
    return NULL;
  }
  mod = ydFromFile(jsonManager, tmp);
  g_free(tmp);
  if (!mod)
    return NULL;
  type = yeGet(mod, "type");
  file = yeGet(mod, "file");
  starting_widget = yeGet(mod, "starting widget");
  preLoad = yeGet(mod, "pre-load");
  initScripts = yeGet(mod, "init-scripts");

  YE_ARRAY_FOREACH(preLoad, var) {
    Entity *tmpType = yeGet(var, "type");
    Entity *tmpFile = yeGet(var, "file");

    if (yuiStrEqual0(yeGetString(tmpType), "lua")) {
      if (ysLoadFile(luaManager, yeGetString(tmpFile)) < 0) {
	DPRINT_ERR("Error when loading '%s': %s\n",
		   yeGetString(tmpFile), ysGetError(luaManager));
      }
    } else if (yuiStrEqual0(yeGetString(tmpType), "json")) {
      char *fileStr = NULL;
      Entity *as = yeGet(var, "as");

      fileStr = g_strconcat(path, "/",
			    yeGetString(tmpFile), NULL);
      tmpFile = ydFromFile(jsonManager, fileStr);
      g_free(fileStr);
      if (tmpFile && yeGetString(as) != NULL) {
	yePushBack(mod, tmpFile, yeGetString(as));
	YE_DESTROY(tmpFile);
      }
    }
  }

  YE_ARRAY_FOREACH(initScripts, var2) {
    ysCall(luaManager, yeGetString(var2), 1, mod);
  }


  if (type) {
    if (yuiStrEqual(yeGetString(type), "json")) {
      char *fileStr = NULL;

      fileStr = g_strconcat(path, "/",
			    yeGetString(file), NULL);
      file = ydFromFile(jsonManager, fileStr);
      g_free(fileStr);
      if (!file) {
	return NULL;
      }

      starting_widget = yeGet(file, yeGetString(starting_widget));
    } else {
      DPRINT_ERR("start does not suport loader of type %s", yeGetString(type));
    }
  } else {
    starting_widget = yeGet(mod, yeGetString(starting_widget));
  }

  yePushBack(mod, starting_widget, "$starting widget");
  return mod;
}

static int ygParseStartAndGame(GameConfig *config)
{
  YWidgetState *wid;
  Entity *starting_widget;

  mainMod = ygLoadMod(config->startingMod->path);
  alive = 1;

  starting_widget = yeGet(mainMod, "$starting widget");
  
  if (starting_widget) {
    wid = ywidNewWidget(starting_widget, NULL);
    if (!wid) {
      DPRINT_ERR("Fail to create widget of type '%s'.",
		 yeGetString(yeGet(starting_widget, "<type>")));
      return -1;
    }
    ywidSetMainWid(wid);
  }

  if (!ywidGetMainWid()) {
      DPRINT_ERR("No main widget has been set.\n"
		 "see documentation about starting_widget\n"
		 "or set it manually with \"ywidSetMainWid()\"");
      return -1;
  }

  do {
    wid = ywidGetMainWid();
    g_assert(ywidRend(wid) != -1);
    sched_yield();
    ywidDoTurn(wid);
  } while(alive);

  return 0;
}

int ygStartLoop(GameConfig *config)
{
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

  ret = ygParseStartAndGame(config);
  return ret;
}

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
