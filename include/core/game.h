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

/* For now this is in game and should move after */
typedef struct {
  const char *path;
} Module;

typedef struct RenderConf_ {
  const char *name;
  struct RenderConf_ *next;
} RenderConf;

typedef struct {
  Module *startingMod;
  RenderConf *rConf;
} GameConfig;


int ygInit(void);

int ygStartLoop(GameConfig *config);

void ygEnd(void);

#endif
