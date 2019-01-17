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
#include "rawfile-desc.h"
#include "description.h"

/* scripting */
#include "entity-script.h"
#include "lua-script.h"
#include "lua-binding.h"
#include "tcc-script.h"
#include "native-script.h"
#include "ybytecode-script.h"

/* widgets */
#include "utils.h"
#include "entity.h"
#if WITH_CURSES == 1
#include "curses-driver.h"
#endif
#include "sdl-driver.h"
#include "menu.h"
#include "map.h"
#include "text-screen.h"
#include "canvas.h"
#include "container.h"
#include "native-script.h"
#include "condition.h"

static int init;
static void *jsonManager;
static void *rawfileManager;

static void *luaManager;
static void *tccManager;

static Entity *mainMod;
static Entity *modList;
static Entity *baseMod;
static Entity *globalsFunctions;
static Entity *stalked_array;

static int alive = 1;

static const char *sdl2 = "sdl2";
static const char *curses = "curses";

char *yProgramArg;

char *ygBinaryRootPath = "./";

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

void ygTerminate(void)
{
  ysdl2WindowMode();
  alive = 0;
}

static void *ygTerminateCallback(va_list va)
{
  (void)va;
  ygTerminate();
  return (void *)ACTION;
}

static void *quitOnKeyDown(va_list ap)
{
  va_list tmp_ap;
  va_copy(tmp_ap, ap);
  va_arg(tmp_ap, Entity *);
  Entity *events = va_arg(tmp_ap, Entity *);
  Entity *eve = events;
  void *ret = (void *)NOTHANDLE;

  YEVE_FOREACH(eve, events) {
    if (ywidEveType(eve) == YKEY_DOWN) {
      if (ywidEveKey(eve) == 'q' ||
	  ywidEveKey(eve) == Y_ESC_KEY) {
	alive = 0;
	ret = (void *)ACTION;
	goto exit;
      }
    }
  }
 exit:
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
  // presently toSet is set here to events
  Entity *toSet = va_arg(ap, Entity *);
  Entity *value;

  // here we really set to it's value
  toSet = va_arg(ap, Entity *);
  value = va_arg(ap, Entity *);
  (void)wid;
  ygSetInt(yeGetString(toSet), yeGetInt(value));
  return (void *)NOACTION;
}

static void *recreateInt(va_list ap)
{
  Entity *wid = va_arg(ap, Entity *);
  // presently toSet is set here to events
  Entity *toSet = va_arg(ap, Entity *);
  Entity *value;

  // here we really set to it's value
  toSet = va_arg(ap, Entity *);
  value = va_arg(ap, Entity *);
  (void)wid;
  ygReCreateInt(yeGetString(toSet), yeGetInt(value));
  return (void *)NOACTION;
}

static void *nextOnKeyDown(va_list ap)
{
  va_list tmp_ap;
  va_copy(tmp_ap, ap);
  va_arg(tmp_ap, Entity *);
  Entity *events = va_arg(tmp_ap, Entity *);
  Entity *eve = events;
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
  ysRegistreCreateNativeEntity(recreateInt, "recreateInt", baseMod, NULL);
  yeCreateFunctionSimple("menuMove", ysNativeManager(), baseMod);
  yeCreateFunctionSimple("menuNext", ysNativeManager(), baseMod);
  yeCreateFunctionSimple("menuActions", ysNativeManager(), baseMod);
  ygRegistreFunc(ysNativeManager(), 0, "FinishGame", "yFinishGame");
  ygRegistreFunc(ysNativeManager(), 1, "callNext", "yCallNextWidget");
}

int ygIsAlive(void)
{
  return alive;
}

int ygIsInit(void)
{
  return init;
}

int ygInit(GameConfig *cfg)
{
  static int t;
  char *path;

  /* trick use in case of failure in this function to free all */
  init = 1;

  yuiDebugInit();

  /* Init parseurs */
  yeInitMem();

  globalsFunctions = yeCreateArray(NULL, NULL);
  CHECK_AND_RET(t = ydJsonInit(), -1, -1,
		    "json init failed");
  CHECK_AND_GOTO(jsonManager = ydNewManager(t), NULL, error,
		    "json init failed");
  CHECK_AND_GOTO(rawfileManager = ydNewManager(ydRawFileInit()), NULL, error,
		    "raw-file init failed");

  /* Init scripting */
  /* TODO init internal lua function */
  CHECK_AND_GOTO(t = ysLuaInit(), -1, error, "lua init failed");
  CHECK_AND_GOTO(luaManager = ysNewManager(NULL, t), NULL, error,
		    "lua init failed");
  CHECK_AND_GOTO(yesLuaRegister(luaManager), -1, error, "lua init failed");

  path = g_strdup_printf("%s%s", ygBinaryRootPath, "/scripts-dependancies/object-wrapper.lua");
  if (ysLoadFile(luaManager, path) < 0) {
    DPRINT_WARN("can't load %s: %s", path,
		ysGetError(luaManager));
  }
  g_free(path);
  CHECK_AND_GOTO(t = ysTccInit(), -1, error, "tcc init failed");
  CHECK_AND_GOTO(tccManager = ysNewManager(NULL, t), NULL, error,
		    "tcc init failed");

  /* Init widgets */
  baseMod = yeCreateArray(NULL, NULL);
  addNativeFuncToBaseMod();

  if (cfg->win_name)
    ywidSetWindowName(cfg->win_name);
  ywidChangeResolution(cfg->w, cfg->h);

  for (GList *tmp = cfg->rConf; tmp; tmp = tmp->next) {
    //TODO check which render to use :)
    if (yuiStrEqual(TO_RC(tmp->data)->name, "curses")) {
#if WITH_CURSES == 1
      ycursInit();
#endif
    } else if (yuiStrEqual(TO_RC(tmp->data)->name, "sdl2")) {
#if WITH_SDL == 1
      ysdl2Init();
      ysound_init();
#endif
    }
  }

  CHECK_AND_GOTO(ywMenuInit(), -1, error, "Menu init failed");
  CHECK_AND_GOTO(ywMapInit(), -1, error, "Map init failed");
  CHECK_AND_GOTO(ywTextScreenInit(), -1, error, "Text Screen init failed");
  CHECK_AND_GOTO(ywContainerInit(), -1, error, "Container init failed");
  CHECK_AND_GOTO(ywCanvasInit(), -1, error, "Canvas init failed");

  for (GList *tmp = cfg->rConf; tmp; tmp = tmp->next) {
    //TODO check which render to use :)
    if (yuiStrEqual(TO_RC(tmp->data)->name, "curses")) {
#if WITH_CURSES == 1
      CHECK_AND_GOTO(ycursRegistreMenu(), -1, error, "Menu init failed");
      CHECK_AND_GOTO(ycursRegistreTextScreen(), -1, error,
			"Text Screen init failed");
      CHECK_AND_GOTO(ycursRegistreMap(), -1, error, "Map init failed");
#else
      DPRINT_ERR("yirl is not compille with curses support");
#endif

    } else if (yuiStrEqual(TO_RC(tmp->data)->name, "sdl2")) {
#ifdef WITH_SDL
      CHECK_AND_GOTO(ysdl2RegistreTextScreen(), -1, error,
			"Text Screen init failed");
      CHECK_AND_GOTO(ysdl2RegistreMenu(), -1, error, "Menu init failed");
      CHECK_AND_GOTO(ysdl2RegistreMap(), -1, error, "Map init failed");
      CHECK_AND_GOTO(ysdl2RegistreCanvas(), -1, error, "Canvas SDL2 init failed");
#else
      DPRINT_ERR("yirl is not compille with SDL2 support");
#endif

    }
  }
  modList = yeCreateArray(NULL, NULL);
  stalked_array = yeCreateArray(NULL, NULL);

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
  ydDestroyManager(rawfileManager);
  ydRawFileEnd();
  ywTextScreenEnd();
  ywMapEnd();
  ywMenuEnd();
  ywCanvasEnd();
  ywContainerEnd();
#if WITH_CURSES == 1
  ycursDestroy();
#endif
#ifdef WITH_SDL
  ysound_end();
  ysdl2Destroy();
#endif
  yeDestroy(modList);
  modList = NULL;
  ysNativeEnd();
  ysYBytecodeEnd();
  ysDestroyManager(tccManager);
  ysTccEnd();
  ysDestroyManager(luaManager);
  ysLuaEnd();
  yeDestroy(globalsFunctions);
  globalsFunctions = NULL;
  yeDestroy(baseMod);
  baseMod = NULL;
  yeDestroy(stalked_array);
  stalked_array = NULL;
  yeEnd();
  free(yProgramArg);
  yProgramArg = NULL;
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
  else if (yuiStrEqual0(name, "yb"))
    return ysYBytecodeManager();
  return NULL;
}

Entity *ygFileToEnt(YFileType t, const char *path, Entity *father)
{
  if (t == YJSON)
    return ydFromFile(jsonManager, path, father);
  else if (t == YRAW_FILE)
    return ydFromFile(rawfileManager, path, father);
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

  for (int i = 0; i < 2; ++i) {
    const char * const starts[] = {"/start.c", "/start.lua"};
    void * const managers[] = {tccManager, luaManager};

    tmp = g_strconcat(path, starts[i], NULL);
    CHECK_AND_RET(tmp, NULL, NULL,
		  "cannot allocated path(like something went really wrong)");
    if (!access(tmp, F_OK) && !ysLoadFile(managers[i], tmp)) {
      mod = yeCreateArray(NULL, NULL);
      yeCreateString(path, mod, "$path");
      if (ysCall(managers[i], "mod_init", mod))
	break;
    }
    yeDestroy(mod);
    g_free(tmp);
  }

  if (!mod) {
    tmp = g_strconcat(path, "/start.json", NULL);
    CHECK_AND_RET(tmp, NULL, NULL,
		  "cannot allocated path(like something went really wrong)");
    mod = ydFromFile(jsonManager, tmp, NULL);
    yeCreateString(path, mod, "$path");
  }
  if (!mod)
    goto failure;
  name = yeGet(mod, "name");
  CHECK_AND_GOTO(name, NULL, failure, "name not found in %s\n", tmp);

  if (yeGet(modList, yeGetString(name))) {
    yeDestroy(mod);
    goto exit;
  }
  yePushBack(modList, mod, yeGetString(name));
  yeDestroy(mod);
  type = yeGet(mod, "type");
  file = yeGet(mod, "file");
  preLoad = yeGet(mod, "pre-load");
  initScripts = yeGet(mod, "init-scripts");

  YE_ARRAY_FOREACH(preLoad, var) {
    Entity *tmpType = yeGet(var, "type");
    Entity *tmpFile = yeGet(var, "file");
    Entity *pathEnt = yeGet(var, "path");
    char *fileStr = g_strconcat(path, "/", yeGetString(tmpFile), NULL);
    const char *pathCstr = "no path set";


    if (tmpFile) {
      pathCstr = fileStr;
    } else if (pathEnt) {
      char *mod_path = g_strdup_printf("%s%s", ygBinaryRootPath, "/modules/");

      yeStringReplace(pathEnt, "YIRL_MODULES_PATH", mod_path);
      g_free(mod_path);
      pathCstr = yeGetString(pathEnt);
    }
    if (yuiStrEqual0(yeGetString(tmpType), "lua")) {
      if (ysLoadFile(luaManager, pathCstr) < 0) {
	DPRINT_ERR("Error when loading '%s': %s\n",
		   pathCstr, ysGetError(luaManager));
	goto fail_preload;
      }

    } else if (yuiStrEqual0(yeGetString(tmpType), "tcc")) {
      if (ysLoadFile(tccManager, pathCstr) < 0) {
	DPRINT_ERR("Error when loading '%s': %s\n",
		   pathCstr, ysGetError(tccManager));
	goto fail_preload;
      }

    } else if (yuiStrEqual0(yeGetString(tmpType), "yb")) {
      if (ysLoadFile(ysYBytecodeManager(), pathCstr) < 0) {
	DPRINT_ERR("Error when loading '%s': %s\n",
		   pathCstr, ysGetError(tccManager));
	goto fail_preload;
      }

    } else if (yuiStrEqual0(yeGetString(tmpType), "json")) {
      Entity *as = yeGet(var, "as");
      tmpFile = ydFromFile(jsonManager, pathCstr, mod);

      yeRenamePtrStr(mod, tmpFile, yeGetString(as));
    } else if (yuiStrEqual0(yeGetString(tmpType), "module")) {
      if (!ygLoadMod(pathCstr)) {
	DPRINT_ERR("fail to load module: %s", pathCstr);
      fail_preload:
	g_free(fileStr);
	goto failure;
      }
    }
    g_free(fileStr);
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

  starting_widget = yeGet(mod, "starting widget");
  if (!starting_widget)
    starting_widget = yeGet(mod, "starting_widget");
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
    starting_widget = yeGetByStr(mod, yeGetString(starting_widget));
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

  ret = yeGet(globalsFunctions, funcName);
  if (!ret && ysGetFastPath(ysNativeManager(), funcName)) {
    ret = yeCreateFunctionSimple(funcName, ysNativeManager(), globalsFunctions);
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

void ygReCreateInt(const char *toSet, int val)
{
  Entity *father = modList;
  char buf[1024];

  for (uint32_t i = 0, b_len = 0;
       ({ buf[b_len] = toSet[i]; ++b_len; toSet[i] ;});
       ++i) {
    if (unlikely(b_len > 1024))
      return;
    if (toSet[i] == '.' || toSet[i] == ':') {
      Entity *nfather;
      buf[b_len -1] = 0;
      nfather = yeGet(father, buf);
      if (!nfather)
	nfather = yeCreateArray(father, buf);
      father = nfather;
      b_len = 0;
      continue;
    }
  }

  yeReCreateInt(val, father, buf);
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
    father = modList;
  } else {
    father = yeNGetByStr(modList, toSet, len - 1);
    toSet += len;
  }

  if ((e = yeGet(father, toSet)) == NULL)
    yeCreateInt(val, father, toSet);
  else
    yeSetInt(e, val);
}

int ygBind(YWidgetState *wid, const char *callback)
{
  Entity *callbackEnt = ygGetFuncExt(callback);
  if (!callbackEnt) {
    DPRINT_WARN("unable to find '%s'\n", callback);
  }
  if (!yeReplaceBack(wid->entity, callbackEnt, "action"))
    return -1;
  return 0;
}

static int ygParseStartAndGame(GameConfig *config)
{
  Entity *starting_widget;
  Entity *ws;
  Entity *wn;

  mainMod = ygLoadMod(config->startingMod->path);
  ws = yeGet(mainMod, "window size");
  wn = yeGet(mainMod, "window name");

  starting_widget = yeGet(mainMod, "$starting widget");

  if (starting_widget) {
    YWidgetState *wid = ywidNewWidget(starting_widget, NULL);

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

  if (ws)
    ywidChangeResolution(yeGetIntAt(ws, 0), yeGetIntAt(ws, 1));
  if (wn)
    ywidSetWindowName(yeGetString(wn));
  return ygDoLoop();
}

int ygStalk(const char *entityPath, Entity *callback, Entity *arg)
{
  Entity *stalked = yeReCreateArray(stalked_array, entityPath, NULL);
  Entity *target = ygGet(entityPath);

  if (unlikely(!target || !callback))
    goto error;

  yeCreateString(entityPath, stalked, NULL);
  if (!yeCreateCopy(target, stalked, NULL))
    goto error;

  yePushBack(stalked, callback, NULL);
  yePushBack(stalked, arg, NULL);
  return 0;
 error:
  yeRemoveChild(stalked_array, stalked);
  return -1;
}

int ygUnstalk(const char *entityPath)
{
  if (!yeGet(stalked_array, entityPath))
    return -1;
  yeRemoveChild(stalked_array, entityPath);
  return 0;
}

static void checkSlakedEntity(void)
{
  YE_ARRAY_FOREACH(stalked_array, elem) {
    Entity *original = ygGet(yeGetStringAt(elem, 0));
    Entity *copy = yeGet(elem, 1);
    if (!yeEqual(original, copy)) {
      yesCall(yeGet(elem, 2), original, copy, yeGet(elem, 3));
      yeCopy(original, copy);
    }
  }
}

int ygDoLoop(void)
{
  alive = 1;

  do {
    YWidgetState *wid = ywidGetMainWid();

    if (unlikely(!wid)) {
      return -1;
    }
    g_assert(ywidRend(wid) != -1);
    checkSlakedEntity();
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
  cfg->w = ywidWindowWidth;
  cfg->h = ywidWindowHight;
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
    cfg->rConf = g_list_append(cfg->rConf, rConf);
  }
  return 0;
}

int ygInitGameConfigByStr(GameConfig *cfg, const char *path, const char *render)
{
  RenderConf *rConf = g_new(RenderConf, 1);

  cfg->rConf = NULL;
  cfg->startingMod = g_new(ModuleConf, 1);
  cfg->startingMod->path = path;
  cfg->w = ywidWindowWidth;
  cfg->h = ywidWindowHight;
  cfg->win_name = NULL;


  rConf->name = render;
  cfg->rConf = g_list_append(cfg->rConf,
			     rConf);
  return 0;
}

void ygCleanGameConfig(GameConfig *cfg)
{
  g_free(cfg->startingMod);
  g_list_free_full(cfg->rConf, g_free);
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

int yePushToGlobalScope(Entity *entity, const char *name)
{
  if (yeGet(modList, name))
    return -1;
  return yePushBack(modList, entity, name);
}


#undef CHECK_AND_GOTO
#undef CHECK_AND_RET
