/*
**Copyright (C) 2019 Matthias Gatto
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

#ifndef _YIRL_EVENTS_H_
#define _YIRL_EVENTS_H_

#include "yirl/entity.h"
#include "yirl/widget.h"

_Bool yevIsKeyDown(Entity *events, int k);
_Bool yevIsKeyUp(Entity *events, int k);

/* use as yevCreateGrp(parent, keys...) */
#define yevCreateGrp yeCreateInts

/**
 * @return 1 if a key of grp is press down
 */
_Bool yevIsGrpDown(Entity *events, Entity *grp);
/**
 * @return 1 if a key of grp is press up
 */
_Bool yevIsGrpUp(Entity *events, Entity *grp);

static inline void yeveDirFromDirGrp(Entity *events,
				     Entity *up_g, Entity *down_g,
				     Entity *left_g, Entity *right_g,
				     int *up_down, int *right_left,
				     void (*callback)(Entity *, int key, int is_up),
				     Entity *arg)
{
	int ud_push = 0, lr_push = 0;

	if (yevIsGrpDown(events, up_g)) {
		*up_down = -1;
		ud_push = 1;
		if (callback)
			callback(arg, Y_UP_KEY, 0);
	}
	if (yevIsGrpDown(events, down_g)) {
		ud_push = 1;
		*up_down = 1;
		if (callback)
			callback(arg, Y_DOWN_KEY, 0);
	}
	if (yevIsGrpDown(events, right_g)) {
		lr_push = 1;
		*right_left = 1;
		if (callback)
			callback(arg, Y_RIGHT_KEY, 0);
	}
	if (yevIsGrpDown(events, left_g)) {
		lr_push = 1;
		*right_left = -1;
		if (callback)
			callback(arg, Y_LEFT_KEY, 0);
	}

	if (yevIsGrpUp(events, up_g) && !ud_push) {
		*up_down = 0;
		if (callback)
			callback(arg, Y_UP_KEY, 1);
	}
	if (yevIsGrpUp(events, down_g) && !ud_push) {
		*up_down = 0;
		if (callback)
			callback(arg, Y_DOWN_KEY, 1);
	}
	if (yevIsGrpUp(events, right_g) && !lr_push) {
		*right_left = 0;
		if (callback)
			callback(arg, Y_RIGHT_KEY, 1);
	}
	if (yevIsGrpUp(events, left_g) && !lr_push) {
		*right_left = 0;
		if (callback)
			callback(arg, Y_LEFT_KEY, 1);
	}
}

static inline int yevCheckKeysInt(Entity *events, EventType type, int *keys)
{
  Entity *eve = events;

  YEVE_FOREACH(eve, events) {
    if (ywidEveType(eve) != type)
      continue;
    int ck = ywidEveKey(eve);

    for (int k = *keys; k >= 0; ({ ++keys; k = *keys; })) {
      if (ck == k)
	return 1;
    }
  }
  return 0;
}

/**
 * usage exemple: yevCheckKeys(events, YKEY_DOWN, 'a', 'b')
 */
#define yevCheckKeys(events, type, keys...)	\
  ({ int k[] = { keys, -1}; int r = yevCheckKeysInt(events, type, k);  r; })

Entity *yevMousePos(Entity *events);

int yevMouseDown(Entity *events, int *button);
int yevMouseUp(Entity *events, int *button);

#endif
