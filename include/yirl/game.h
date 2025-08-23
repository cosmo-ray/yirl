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

#ifndef _YIRL_GAME_H_
#define _YIRL_GAME_H_

#include "yirl/entity.h"
#include "yirl/widget.h"
#include "yirl/utils.h"
#include "yirl/map.h"
#include "yirl/sound.h"

#ifndef Y_INSIDE_TCC
#include "yirl/native-script.h"
#include "yirl/script.h"
#else
#define Y_END_VA_LIST ((void *)0xDEAD0000)
#endif

typedef enum {
  YNONE   = 0,
  YSDL2   = 1,
  YALL    = YSDL2
} RenderType;

typedef enum {
  YJSON = 0,
  YRAW_FILE,
  YRAW_FILE_DATA,
  END_YFILETYPE
} YFileType;

typedef struct {
	const char *path;
} ModuleConf;

typedef struct {
	ModuleConf *startingMod;
	const char *win_name;
	RenderType render_type;
	int w;
	int h;
} GameConfig;

#undef GList

extern char *yProgramArg;

extern const char *ygBinaryRootPath;

static inline Entity *ygInitWidgetModule(Entity *mod, const char *name, Entity *call)
{
	Entity *wid_type = yeCreateArray(NULL, NULL);

	yeCreateString(name, wid_type, "name");
	yePushBack(wid_type, call, "callback");

	Entity *start = yeCreateArray(mod, "test_wid");
	yeCreateString(name, start, "<type>");

	yeCreateString("test_wid", mod, "starting widget");

	ywidAddSubType(wid_type);

	return start;
}

static inline Entity *ygInitWidgetModule2(Entity *mod, Entity *call, const char *name, const char *subtype)
{
	Entity *wid_type = yeCreateArray(NULL, NULL);

	yeCreateString(name, wid_type, "name");
	yePushBack(wid_type, call, "callback-ent");
	yeCreateString(subtype, wid_type, "sub-type");

	Entity *start = yeCreateHash(mod, "test_wid");
	yeCreateString(name, start, "<type>");

	yeCreateString("test_wid", mod, "starting widget");

	ywidAddSubType(wid_type);

	return start;
}

enum {
	Y_MOD_LOCAL,
	Y_MOD_YIRL
};

/**
 * Must be call in mod init
 * @param type either Y_MOD_YIRL or Y_MOD_LOCAL
 * @param mod the mod
 * @param path entity path
 */
static inline void ygAddModule(int type, Entity *mod, const char *path)
{
	Entity *pre_load = yeTryCreateArray(mod, "pre-load");
	Entity *mod_to_load = yeCreateArray(pre_load, 0);

	if (type == Y_MOD_YIRL) {
		Entity *path_ent;

		path_ent = yeCreateString("YIRL_MODULES_PATH", mod_to_load, "path");
		yeAddStr(path_ent, path);
	} else {
		yeCreateString(path, mod_to_load, "file");
	}
	yeCreateString("module", mod_to_load, "type");
}

const char *ygGetBinaryRootPath(void);

const char *ygGetProgramArg(void);

#define ygInitGameConfig(cfg, path, render)				\
  _Generic((render),							\
	   const char *: ygInitGameConfigByStr,				\
	   char *: ygInitGameConfigByStr,				\
	   void *: ygInitGameConfigByStr,				\
	   RenderType : ygInitGameConfigByRenderType,			\
	   int : ygInitGameConfigByRenderType) (cfg, path, render)

int ygInitGameConfigByRenderType(GameConfig *cfg,
				 const char *path, RenderType t);
int ygInitGameConfigByStr(GameConfig *cfg, const char *path,
			  const char *render);

void *ygCallInt(const char *mod, const char *callName, int nb,
		union ycall_arg *args, int *types);

/* because tcc bug with 0 args */
#define ygVCall(mod, callName) ygCallInt(mod, callName, 0, NULL, NULL)

#define ygCall(mod, callName, args...)			\
	ygCallInt(mod, callName,			\
		  YUI_GET_ARG_COUNT(args),		\
		  YS_ARGS(args),			\
		   YS_ATYPES(args))

int ygRegistreFuncInternal(void *manager, int nbArgs, const char *name,
			   const char *toRegistre);

#ifndef Y_INSIDE_TCC
static inline int ygRegistreFunc(void *manager, int nbArgs, const char *name,
				 const char *toRegistre)
{
	return ygRegistreFuncInternal(manager, nbArgs, name, toRegistre);
}
#else
static inline int ygRegistreFunc(int nbArgs, const char *name,
				 const char *toRegistre)
{
	return ygRegistreFuncInternal(ygGetTccManager(), nbArgs,
				      name, toRegistre);
}
#endif

/**
 * get time in ms since game have init
 */
uint32_t ygGetTick(void);

Entity *ygFileToEnt(YFileType t, const char *path, Entity *ent);
int ygEntToFile(YFileType t, const char *path, Entity *ent);
static inline int ygEntToFile2(YFileType t, Entity *ent, const char *path)
{
  return ygEntToFile(t, path, ent);
}

void ygCleanGameConfig(GameConfig *cfg);

int ygInit(GameConfig *config);
int ygIsInit(void);

/**
 * @return true is the game loop is looping
 */
int ygIsAlive(void);

int ygStartLoop(GameConfig *config);
int ygDoLoop(void);

const char *ygModDirPath(const char * restrict const mod);

Entity *ygLoadMod(const char *path);
Entity *ygGetMod(const char *path);
Entity *ygGetFuncExt(const char *func);

void *ygGetManager(const char *name);

/**
 * try to call wid[callback_key] is it exist, or exit game
 */
void *ygCallFuncOrQuit(Entity *wid, const char *callback_key);

void ygTerminate(void);
void ygEnd(void);

/* scrits managers */
void *ygPH7Manager(void);
void *ygS7Manager(void);
void *ygPerlManager(void);
void *ygGetLuaManager(void);
void *ygGetTccManager(void);

/* When I'll have time I should make a _Generic function */
int ygLoadScript(Entity *mod, void *manager, const char *path);

static inline void *ygScriptManager(int manager) {
	if (manager == YLUA)
		return ygGetLuaManager();
	else if (manager == YS7)
		return ygS7Manager();
	else if (manager == YPH7)
		return ygPH7Manager();
	else if (manager == YTCC)
		return ygGetTccManager();
	else if (manager == YPERL)
		return ygPerlManager();
	return NULL;
}

#define ygScriptCall(manager, func, args...)		\
	ysCall(ygScriptManager(manager), (func), args)

static inline int ygLoadScript2(int manager, Entity *mod, const char *path) {
	return ygLoadScript(mod, ygScriptManager(manager), path);
}

int ygBind(YWidgetState *wid, const char *callback);

Entity *ygGet(const char *toFind);
void ygSetInt(const char *toSet, int val);
void ygReCreateInt(const char *toSet, int val);
void ygIncreaseInt(const char *toInc, int val);
void ygIncreaseIntMax(const char *toInc, int val, int max);
void ygDecreaseIntMin(const char *toInc, int val, int min);

void ygReCreateString(const char *toSet, const char *str);

#define ygGetInt(path) yeGetInt(ygGet(path))
#define ygGetString(path) yeGetString(ygGet(path))

static inline void ygIntAdd(const char *toSet, int toAdd)
{
	int i = ygGetInt(toSet);
	ygSetInt(toSet, i + toAdd);
}

int ygAddDefine(const char *name, char *val);

/**
 * Create a copy of an entity of type float, int or string
 * which is obtain by calling ygGet(entityPath)
 * check at each turn if the copied and stalked entities are the same, if not, call
 * callback(originalEntity, copiedEntity, arg), then copy the original again
 */
int ygStalk(const char *entityPath, Entity *callback, Entity *arg);

int ygUnstalk(const char *entityPath);

const char *ygUserDir(void);

static inline int ygEqual(const char *path, Entity *o)
{
  return yeEqual(ygGet(path), o);
}

/**
 * @brief do a chdir(yeGetStringAt(ygGet(mod), "$path"));
 */
int ygModDir(const char * restrict const mod);

/**
 * @brief change directory to mod directory
 */
int ygModDirByEntity(Entity *mod);

/**
 * @brief chdir to "main_dir", the one specified by -d or
 * where the yirl has been started
 */
int ygModDirOut(void);

/**
 * @brief   push @ent to yirl global scope
 * @param   ent entity to push
 * @param   name mandatory name
 */
int yePushToGlobalScope(Entity *ent, const char *name);
static inline int ygPushToGlobalScope(Entity *ent, const char *name)
{
	return yePushToGlobalScope(ent, name);
}

void ygRemoveFromGlobalScope(const char *name);

static inline void ygUpdateScreen(void)
{
	ywidRend(ywidGetMainWid());
}

#if defined(NDEBUG) || defined(YIRL_WITHOUT_ASSERT)
#define ygAssert(expr)
#else
#define ygAssert(expr)							\
	if (!(expr)) {							\
		fprintf(stderr, "'" #expr "' assertion fail at %s:%d\n", \
			__FILE__, __LINE__);				\
		ygDgbAbort();						\
	};
#endif

#ifdef NDEBUG
#define ygDgbAbort() do {} while (0)
#else
void ygDgbAbort(void);
#endif

#endif
