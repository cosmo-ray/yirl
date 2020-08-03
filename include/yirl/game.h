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
  NONE   = 0,
  SDL2   = 1,
  CURSES = 2,
  ALL    = CURSES | SDL2
} RenderType;

typedef enum {
  YJSON = 0,
  YRAW_FILE,
  END_YFILETYPE
} YFileType;

typedef struct {
	const char *path;
} ModuleConf;

typedef struct {
	ModuleConf *startingMod;
	const char *win_name;
	int w;
	int h;
} GameConfig;

#undef GList

extern char *yProgramArg;

extern char *ygBinaryRootPath;

static inline void ygBinaryRootPathFree(void)
{
  free(ygBinaryRootPath);
  ygBinaryRootPath = "./";
}

#define ygInitGameConfig(cfg, path, render)				\
  _Generic((render),							\
	   const char *: ygInitGameConfigByStr,				\
	   char *: ygInitGameConfigByStr,				\
	   Y_GEN_CLANG_ARRAY(char, ygInitGameConfigByStr),		\
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

Entity *ygFileToEnt(YFileType t, const char *path, Entity *father);
int ygEntToFile(YFileType t, const char *path, Entity *ent);

void ygCleanGameConfig(GameConfig *cfg);

int ygInit(GameConfig *config);
int ygIsInit(void);

/**
 * @return true is the game loop is looping
 */
int ygIsAlive(void);

int ygStartLoop(GameConfig *config);
int ygDoLoop(void);

Entity *ygLoadMod(const char *path);
Entity *ygGetMod(const char *path);
Entity *ygGetFuncExt(const char *func);

int ygLoadScript(Entity *mod, void *manager, const char *path);

void *ygGetManager(const char *name);

void ygTerminate(void);
void ygEnd(void);

/* scrits managers */
void *ygS7Manager(void);
void *ygGetLuaManager(void);
void *ygGetTccManager(void);

int ygBind(YWidgetState *wid, const char *callback);

Entity *ygGet(const char *toFind);
void ygSetInt(const char *toSet, int val);
void ygReCreateInt(const char *toSet, int val);
void ygIncreaseInt(const char *toInc, int val);

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

static inline int ygEqual(const char *path, Entity *o)
{
  return yeEqual(ygGet(path), o);
}

/**
 * @brief do a chdir(yeGetStringAt(ygGet(mod), "$path"));
 */
int ygModDir(const char * restrict const mod);
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

#endif
