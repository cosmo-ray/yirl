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

/**
 * this widgets try to handle colision detection, and objects movements for you.
 * you have a defined number of groups, that can be set with 'nb_grp', in widget entity
 * each groups have printable objects, and have some optional callbacks,
 *	and options that can be set, to define objects behaviors.
 *
 * when creating objects, you can use canvas fucntion, but need to call ywEBSSwapGroup
 * to define in which group you want your canvasObj to be created.
 *
 * avaible callbackss are:
 * 'on-down'/'on-up':
 *	need to be set like this: {"on-up": [KEY, callback]}
 *	param: WIDGET_ENTITY.
 * 'grps-oob-callbacks':
 *	call everytime an objects from a group OOB.
 *	set like this: {"grps-oob-callbacks": [GROUPE_0_CALLBACK, GROUPE_1_CALLBACK]}
 *	param: WIDGET_ENTITY, OOB_OBJECT
 *
 * avaible options are:
 * 'grps-allow-oob':
 *	enable an object to OOB,
 *	set like this: { "grps-allow-oob": [0, 1, ...] }
 *
 * each groups have a speed, and direction, that can be set, either
 * using ywSetGroupeSpeed/ywSetGroupeDir, or {"grps-dir": [1.4, 2.4], "grps-spd": [10, 5]}
 */

#include "yirl/widget.h"

void ywEBSRemoveObj(Entity *wid, int grp, Entity *obj);

int ywSetGroupeSpeed(Entity *wid, int grp, int speed);
/* set groupe dir in degree */
int ywSetGroupeDir(Entity *wid, int grp, double radiant);
int ywEBSSwapGroup(Entity *wid, unsigned int target);
void ywEBSWrapperPush(Entity *wid, Entity *wrapper);

static inline double ywEBSGroupeDir(Entity *wid, int grp)
{
	Entity *groups_dir = yeGet(wid, "grps-dir");

	return yeGetFloatAt(groups_dir, grp);
}

int ywEBSInit(void);
int ywEBSEnd(void);
int ysdl2RegistreEBS(void);
