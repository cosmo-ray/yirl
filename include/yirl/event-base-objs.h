/*
**Copyright (C) 2024 Matthias Gatto
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

#include "yirl/widget.h"


int ywSetGroupeSpeed(Entity *wid, int grp, int speed);
/* set groupe dir in degree */
int ywSetGroupeDir(Entity *wid, int grp, double radiant);
int ywEBSSwapGroup(Entity *wid, unsigned int target);
void ywEBSWrapperPush(Entity *wid, Entity *wrapper);

int ywEBSInit(void);
int ywEBSEnd(void);
int ysdl2RegistreEBS(void);
