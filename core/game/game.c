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
#include <string.h>
#include <glib.h>

#include "game.h"

/* description */
#include "json-desc.h"
#include "description.h"

/* scripting */
#include "entity-script.h"
#include "lua-script.h"
#include "lua-binding.h"
#include "tcc-script.h"
#include "native-script.h"

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
#include "native-script.h"
#include "condition.h"

static int init;
static void *jsonManager;
static void *luaManager;
static void *tccManager;

static Entity *globFunctions;

static Entity *mainMod;
static Entity *modList;
static Entity *baseMod;
static Entity *globalsFunctions;

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


static void *ygTerminateCallback(va_list va)
{
  ysdl2WindowMode();
  (void)va;
  alive = 0;
  return (void *)ACTION;
}

static void *quitOnKeyDown(va_list ap)
{
  va_list tmp_ap;
  va_copy(tmp_ap, ap);
  va_arg(tmp_ap, Entity *);
  YEvent *events = va_arg(tmp_ap, YEvent *);
  YEvent *eve = events;
  void *ret = (void *)NOTHANDLE;

  YEVE_FOREACH(eve, events) {
    if (ywidEveType(eve) == YKEY_DOWN) {
      if (ywidEveKey(eve) == 'q' ||
	  ywidEveKey(eve) == '\n' ||
	  ywidEveKey(eve) == Y_ESC_KEY) {
	alive = 0;
	return (void *)ACTION;
      }
    }
  }
  va_end(tmp_ap);
  return ret;
}

static void *fullScreenOnOff(va_list ap)
{
  static int fs;
  (void)ap;

  if (fs) {
    ysdl2WindowMode();
    fs = 0;
  } else {
    ysdl2FullScreen();
    fs = 1;
  }
  return (void *)NOTHANDLE;
}

static void *nextWid(va_list ap)
{
  Entity *wid = va_arg(ap, Entity *);
  Entity *next = yeGet(wid, "next");

  if (!next) {
    DPRINT_ERR("unable to get next widget");
    return (void *)BUG;
  }

  if (ywidNext(next) < 0)
    return (void *)BUG;
  return (void *)ACTION;
}

static void *setInt(va_list ap)
{
  Entity *wid = va_arg(ap, Entity *);
  void *useless = va_arg(ap, YEvent *);
  Entity *toSet;
  Entity *value;

  useless = va_arg(ap, void *);
  toSet = va_arg(ap, Entity *);
  value = va_arg(ap, Entity *);
  (void)useless;
  (void)wid;
  ygSetInt(yeGetString(toSet), yeGetInt(value));
  return (void *)NOACTION;
}

static void *nextOnKeyDown(va_list ap)
{
  va_list tmp_ap;
  va_copy(tmp_ap, ap);
  va_arg(tmp_ap, Entity *);
  YEvent *events = va_arg(tmp_ap, YEvent *);
  YEvent *eve = events;
  void *ret = (void *)NOTHANDLE;

  YEVE_FOREACH(eve, events) {
    if (ywidEveType(eve) == YKEY_DOWN) {
      if (ywidEveKey(eve) == ' ' ||
	  ywidEveKey(eve) == '\n' ||
	  ywidEveKey(eve) == Y_ESC_KEY) {
	ret = nextWid(ap);
      }
    }
  }
  va_end(tmp_ap);
  return ret;
}

#define TO_RC(X) ((RenderConf *)(X))

static void addNativeFuncToBaseMod(void)
{
  ysRegistreCreateNativeEntity(fullScreenOnOff, "FullScreenOnOff",
			       baseMod, NULL);
  ysRegistreCreateNativeEntity(ygTerminateCallback, "FinishGame", baseMod, NULL);
  ysRegistreCreateNativeEntity(quitOnKeyDown, "QuitOnKeyDown", baseMod, NULL);
  ysRegistreCreateNativeEntity(nextWid, "callNext", baseMod, NULL);
  ysRegistreCreateNativeEntity(nextOnKeyDown, "nextOnKeyDown",
			       baseMod, NULL);
  ysRegistreCreateNativeEntity(setInt, "setInt", baseMod, NULL);
  yeCreateFunctionSimple("menuMove", ysNativeManager(), baseMod);
  yeCreateFunctionSimple("menuNext", ysNativeManager(), baseMod);
  yeCreateFunctionSimple("menuActions", ysNativeManager(), baseMod);
  ygRegistreFunc(ysNativeManager(), 0, "FinishGame", "yFinishGame");
}

int ygInit(GameConfig *cfg)
{
  static int t;

  /* trick use in case of failure in this function to free all */
  init = 1;

  yuiDebugInit();

  /* Init parseurs */
  yeInitMem();

  /*
   * mega useless, but as cmake try to be samrt,
   * it doesn't link this to game if not call
   * TODO: remove cmake
   */
  yeCheckCondition(NULL);

  globalsFunctions = yeCreateArray(NULL, NULL);
  CHECK_AND_RET(t = ydJsonInit(), -1, -1,
		    "json init failed");
  CHECK_AND_GOTO(jsonManager = ydNewManager(t), NULL, error,
		    "json init failed");

  /* Init scripting */
  /* TODO init internal lua function */
  CHECK_AND_GOTO(t = ysLuaInit(), -1, error, "lua init failed");
  CHECK_AND_GOTO(luaManager = ysNewManager(NULL, t), NULL, error,
		    "lua init failed");
  CHECK_AND_GOTO(yesLuaRegister(luaManager), -1, error, "lua init failed");

  CHECK_AND_GOTO(t = ysTccInit(), -1, error, "tcc init failed");
  CHECK_AND_GOTO(tccManager = ysNewManager(NULL, t), NULL, error,
		    "tcc init failed");

  /* Init widgets */
  baseMod = yeCreateArray(NULL, NULL);
  addNativeFuncToBaseMod();

  if (cfg->win_name)
	  ywidSetWindowName(cfg->win_name);
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
  ywContenerEnd();
#ifdef WITH_CURSES
  ycursDestroy();
#endif
#ifdef WITH_SDL
  ysdl2Destroy();
#endif
  yeDestroy(modList);
  modList = NULL;
  ysNativeEnd();
  ysDestroyManager(tccManager);
  ysTccEnd();
  ysDestroyManager(luaManager);
  ysLuaEnd();
  yeDestroy(globalsFunctions);
  globalsFunctions = NULL;
  yeDestroy(baseMod);
  baseMod = NULL;
  yeEnd();
  init = 0;
}

int ygRegistreFuncInternal(void *manager, int nbArgs, const char *name,
			   const char *toRegistre)
{
  Entity *func = yeGet(globalsFunctions, name);

  if (!func) {
    func = yeCreateFunctionExt(name, manager, globalsFunctions, name,
			       YE_FUNC_NO_FASTPATH_INIT);
  }

  if (!toRegistre || yuiStrEqual0(name, toRegistre)) {
    if (manager != tccManager)
      ysAddFuncSymbole(tccManager, NULL, nbArgs, func);
    if (manager != luaManager)
      ysAddFuncSymbole(luaManager, NULL, nbArgs, func);
  } else {
    ysAddFuncSymbole(tccManager, toRegistre, nbArgs, func);
    ysAddFuncSymbole(luaManager, toRegistre, nbArgs, func);
  }
  return 0;
}

void *ygGetManager(const char *name)
{
  if (yuiStrEqual0(name, "tcc"))
    return tccManager;
  else if (yuiStrEqual0(name, "lua"))
    return luaManager;
  return NULL;
}

Entity *ygFileToEnt(YFileType t, const char *path, Entity *father)
{
  if (t == YJSON)
    return ydFromFile(jsonManager, path, father);
  return NULL;
}

int ygEntToFile(YFileType t, const char *path, Entity *ent)
{
  if (t == YJSON)
    return ydToFile(jsonManager, path, ent);
  return -1;
}

Entity *ygLoadMod(const char *path)
{
  char *tmp;
  Entity *mod = NULL;
  Entity *type;
  Entity *file;
  Entity *starting_widget;
  Entity *preLoad;
  Entity *initScripts;
  Entity *name;

  if (unlikely(!modList)) {
    modList = yeCreateArray(NULL, NULL);
  }

  tmp = g_strconcat(path, "/start.json", NULL);
  if (!tmp) {
    DPRINT_ERR("cannot allocated path(like something went really wrong)");
    return NULL;
  }
  mod = ydFromFile(jsonManager, tmp, NULL);
  if (!mod)
    goto failure;

  name = yeGet(mod, "name");
  if (!name) {
    DPRINT_ERR("name not found in %s\n", tmp);
    goto failure;
  }

  yePushBack(modList, mod, yeGetString(name));
  yeDestroy(mod);
  yeCreateString(path, mod, "$path");
  type = yeGet(mod, "type");
  file = yeGet(mod, "file");
  starting_widget = yeGet(mod, "starting widget");
  preLoad = yeGet(mod, "pre-load");
  initScripts = yeGet(mod, "init-scripts");

  YE_ARRAY_FOREACH(preLoad, var) {
    Entity *tmpType = yeGet(var, "type");
    Entity *tmpFile = yeGet(var, "file");

    if (yuiStrEqual0(yeGetString(tmpType), "lua")) {
      char *fileStr;

      fileStr = g_strconcat(path, "/",
			    yeGetString(tmpFile), NULL);
      if (ysLoadFile(luaManager, fileStr) < 0) {
	DPRINT_ERR("Error when loading '%s': %s\n",
		   yeGetString(tmpFile), ysGetError(luaManager));
	goto failure;
      }
      g_free(fileStr);

    } else if (yuiStrEqual0(yeGetString(tmpType), "tcc")) {
      char *fileStr;

      fileStr = g_strconcat(path, "/",
			    yeGetString(tmpFile), NULL);
      if (ysLoadFile(tccManager, fileStr) < 0) {
	DPRINT_ERR("Error when loading '%s': %s\n",
		   yeGetString(tmpFile), ysGetError(tccManager));
	goto failure;
      }
      g_free(fileStr);

    } else if (yuiStrEqual0(yeGetString(tmpType), "json")) {
      char *fileStr;
      Entity *as = yeGet(var, "as");

      fileStr = g_strconcat(path, "/",
			    yeGetString(tmpFile), NULL);
      tmpFile = ydFromFile(jsonManager, fileStr, mod);
      g_free(fileStr);

      yeRenamePtrStr(mod, tmpFile, yeGetString(as));
    } else if (yuiStrEqual0(yeGetString(tmpType), "module")) {
      char *fileStr;

      fileStr = g_strconcat(path, "/",
			    yeGetString(tmpFile), NULL);
      if (!ygLoadMod(fileStr)) {
	DPRINT_ERR("fail to load module: %s");
      }
      g_free(fileStr);
    }
  }

  YE_ARRAY_FOREACH(initScripts, var2) {
    if (yeType(var2) == YSTRING) {
      void *fastPath = ysGetFastPath(tccManager, yeGetString(var2));

      if (fastPath)
	ysFCall(tccManager, fastPath, mod);
      else
	ysCall(luaManager, yeGetString(var2), mod);
    } else if (yeType(var2) == YARRAY) {
      ysCall(ygGetManager(yeGetString(yeGet(var2, 0))),
	     yeGetString(yeGet(var2, 1)), mod);
    }
  }

  if (type) {
    if (yuiStrEqual(yeGetString(type), "json")) {
      char *fileStr;
      const char *mod_name = "$main file";

      fileStr = g_strconcat(path, "/",
			    yeGetString(file), NULL);
      file = ydFromFile(jsonManager, fileStr, mod);
      g_free(fileStr);
      if (!file) {
	goto failure;
      }
      if (yeGet(mod, "as"))
	mod_name = yeGetString(yeGet(mod, "as"));
      yeRenamePtrStr(mod, file, mod_name);

      starting_widget = yeGet(file, yeGetString(starting_widget));
    } else {
      DPRINT_ERR("start does not suport loader of type %s", yeGetString(type));
    }
  } else {
    starting_widget = yeGet(mod, yeGetString(starting_widget));
  }
  yePushBack(mod, starting_widget, "$starting widget");
  printf(" === add module: \"%s\" === \n", yeGetString(name));

  goto exit;
 failure:
  yeRemoveChild(modList, mod);
 exit:
  g_free(tmp);
  return mod;
}

Entity *ygGetMod(const char *path)
{
  if (!path)
    return NULL;
  return yeGet(modList, path);
}

int ygLoadScript(Entity *mod, void *manager, const char *path)
{
  int ret;
  char *tmp = g_strconcat(yeGetString(yeGet(mod, "$path")), path, NULL);

  ret = ysLoadFile(manager, tmp);
  g_free(tmp);
  return ret;
}

static Entity *ygGetFunc(Entity *mod, const char *funcName)
{
  Entity *ret;

  if (mod)
    return yeGet(mod, funcName);

  ret = yeGet(globFunctions, funcName);
  if (!ret) {
    ret = yeCreateFunctionSimple(funcName, ysNativeManager(), globFunctions);
  }
  return ret;
}

static int isNestedEntity(const char *str, char *modName, const char **entName)
{
  uint32_t cpLen;

  *entName = g_strrstr(str, ":");
  if (!(*entName)) {
    *entName = str;
    return 0;
  }
  *entName = *entName + 1;

  cpLen = (*entName) - str - 1;
  strncpy(modName, str, cpLen);
  modName[cpLen] = '\0';
  return 1;
}

Entity *ygGetFuncExt(const char *func)
{
  char modName[254];
  const char *funcName;
  char *tmp;
  intptr_t ret;
  Entity *tmpMod;

  if (!func)
    return NULL;
  ret = isNestedEntity(func, modName, &funcName);
  if (ret)
    tmp = modName;
  else
    return ygGetFunc(NULL, funcName);

  tmpMod = ygGetMod(tmp);
  if (!tmpMod) {
    DPRINT_WARN("%s module desn't existe", tmp);
    return NULL;
  }
  return ygGetFunc(tmpMod, funcName);
}

Entity *ygGet(const char *toFind)
{
  char modName[254];
  const char *funcName;
  char *tmp;
  int ret;

  if (!toFind)
    return NULL;
  ret = isNestedEntity(toFind, modName, &funcName);
  tmp = modName;

  // if no module give in toFind, use modList as base
  if (!ret) {
    Entity *retVal = yeGetByStr(modList, funcName);

    if (!retVal)
      return ygGetFunc(NULL, funcName);
    return retVal;
  }
  return yeGetByStr(ygGetMod(tmp), funcName);
}

void ygSetInt(const char *toSet, int val)
{
  int len = strlen(toSet);
  Entity *father;
  Entity *e;
  const char *end;

  for (end = toSet + len - 1; end != toSet && *end != '.' &&  *end != ':';
       --end, --len);
  if (end == toSet) {
    DPRINT_ERR("can't set '%s' because variable is in the global scope");
    return;
  }
  father = yeNGetByStr(modList, toSet, len - 1);
  toSet += len;
  if ((e = yeGet(father, toSet)) == NULL)
    yeCreateInt(val, father, toSet);
  else
    yeSetInt(e, val);
}

int ygBindBySinIdx(YWidgetState *wid, int idx, const char *callback)
{
  return ywidBindBySinIdx(wid, idx, ygGetFuncExt(callback));
}

int ygBind(YWidgetState *wid, const char *signal, const char *callback)
{
  Entity *callbackEnt = ygGetFuncExt(callback);
  if (!callbackEnt) {
    DPRINT_WARN("unable to find '%s'\n", callback);
  }
  return ywidBind(wid, signal, callbackEnt);
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

  return ygDoLoop();
}

int ygDoLoop(void)
{
  YWidgetState *wid;

  do {
    wid = ywidGetMainWid();
    if (unlikely(!wid)) {
      return -1;
    }
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

int ygInitGameConfigByRenderType(GameConfig *cfg, const char *path,
				 RenderType t)
{

  cfg->rConf = NULL;
  cfg->win_name = NULL;
  cfg->startingMod = g_new(ModuleConf, 1);
  cfg->startingMod->path = path;

  if (!t) {
    return 0;
  }

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

int ygInitGameConfigByStr(GameConfig *cfg, const char *path, const char *render)
{
  RenderConf *rConf = g_new(RenderConf, 1);

  cfg->rConf = NULL;
  cfg->startingMod = g_new(ModuleConf, 1);
  cfg->startingMod->path = path;

  cfg->win_name = NULL;


  rConf->name = render;
  cfg->rConf = g_list_append(cfg->rConf,
			     rConf);
  return 0;
}

void ygCleanGameConfig(GameConfig *cfg)
{
  g_free(cfg->startingMod);
  g_list_free(cfg->rConf);
}

void *ygCallInt(const char *mod, const char *callName, ...)
{
  void *ret;
  va_list ap;
  Entity *modEntity;

  if (!callName)
    return NULL;
  modEntity = ygGetMod(mod);
  va_start(ap, callName);
  ret = yesVCall(ygGetFunc(modEntity, callName), ap);
  va_end(ap);
  return ret;

}

int ygAddDefine(const char *name, char *val)
{
  return ysAddDefine(tccManager, name, val);
}

#undef CHECK_AND_GOTO
#undef CHECK_AND_RET
