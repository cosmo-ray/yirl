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

#include "entity.h"
#include "widget.h"
#include "widget-callback.h"
#include "utils.h"
#include "map.h"
#include "sound.h"
#include "native-script.h"

typedef enum {
  SDL2   = 1,
  CURSES = 2,
  ALL    = CURSES | SDL2
} RenderType;

typedef struct {
  const char *path;
} ModuleConf;

typedef struct RenderConf_ {
  const char *name;
} RenderConf;

#define GList void

typedef struct {
  ModuleConf *startingMod;
  GList *rConf;
} GameConfig;

#undef GList

int ygInitGameConfig(GameConfig *cfg, const char *path, RenderType t);

void *ygCallInt(const char *mod, const char *callName, ...);

#define ygCall(mod, callName, args...)					\
	ygCallInt(mod, callName,					\
		  YUI_VA_ARGS_HANDELER(Y_END_VA_LIST,			\
				       args))

void ygCleanGameConfig(GameConfig *cfg);

int ygInit(GameConfig *config);

int ygStartLoop(GameConfig *config);

Entity *ygLoadMod(const char *path);
Entity *ygGetMod(const char *path);
Entity *ygGetFuncExt(const char *func);

void *ygGetManager(const char *name);

void ygEnd(void);

void *ygTerminateCallback(va_list a);

/* scrits managers */
void *ygGetLuaManager(void);
void *ygGetTccManager(void);

int ygBindBySinIdx(YWidgetState *wid, int idx, const char *callback);

int ygBind(YWidgetState *wid, const char *signal, const char *callback);

Entity *ygGet(const char *toFind);

int ygAddDefine(void *opac, const char *name);

#endif
