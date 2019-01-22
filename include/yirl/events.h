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

int yevIsKeyDown(Entity *events, int k);
int yevIsKeyUp(Entity *events, int k);

#define yevCreateGrp yeCreateInts

/**
 * @return 1 if a key of grp is press down
 */
int yevIsGrpDown(Entity *events, Entity *grp);
/**
 * @return 1 if a key of grp is press up
 */
int yevIsGrpUp(Entity *events, Entity *grp);

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

#endif
