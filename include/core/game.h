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


#ifndef GAME_H
#define GAME_H

#include <glib.h>
#include "entity.h"
#include "widget.h"
#include "widget-callback.h"
#include "utils.h"

typedef enum {
  SDL2 = 1,
  CURSES = 2,
  ALL = CURSES | SDL2
} RenderType;

/* For now this is in game and should move after */
typedef struct {
  const char *path;
} ModuleConf;

typedef struct RenderConf_ {
  const char *name;
} RenderConf;

typedef struct {
  ModuleConf *startingMod;
  GList *rConf;
} GameConfig;

int ygInitGameConfig(GameConfig *cfg, const char *path, RenderType t);

void ygCleanGameConfig(GameConfig *cfg);

int ygInit(GameConfig *config);

int ygStartLoop(GameConfig *config);

void ygEnd(void);

int ygTerminateCallback(YWidgetState *wid, YEvent *eve, Entity *arg);

#endif
