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

#include "events.h"

_Bool yevIsKeyDown(Entity *events, int k)
{
	Entity *eve = events;

	YEVE_FOREACH(eve, events) {
		if (ywidEveType(eve) == YKEY_DOWN &&
		    ywidEveKey(eve) == k)
			return 1;
	}
	return 0;
}

_Bool yevIsKeyUp(Entity *events, int k)
{
	Entity *eve = events;

	YEVE_FOREACH(eve, events) {
		if (ywidEveType(eve) == YKEY_UP &&
		    ywidEveKey(eve) == k)
			return 1;
	}
	return 0;
}

Entity *yevMousePos(Entity *events)
{
	Entity *eve = events;

	YEVE_FOREACH(eve, events) {
		int t = ywidEveType(eve);
		if (t == YKEY_MOUSEDOWN || t == YKEY_MOUSEMOTION)
			return ywidEveMousePos(eve);
	}
	return NULL;
}

_Bool yevIsGrpDown(Entity *events, Entity *grp)
{
	Entity *eve = events;

	YEVE_FOREACH(eve, events) {
		if (ywidEveType(eve) != YKEY_DOWN)
			continue;
		int ck = ywidEveKey(eve);
		Entity *key;

		YE_FOREACH(grp, key) {
			int k = yeGetInt(key);

			if (ck == k)
				return 1;
		}
	}
	return 0;
}

_Bool yevIsGrpUp(Entity *events, Entity *grp)
{
	Entity *eve = events;

	YEVE_FOREACH(eve, events) {
		if (ywidEveType(eve) != YKEY_UP)
			continue;
		int ck = ywidEveKey(eve);
		Entity *key;

		YE_FOREACH(grp, key) {
			int k = yeGetInt(key);

			if (ck == k)
				return 1;
		}
	}
	return 0;
}

int yevMouseDown(Entity *events, int *button)
{
	Entity *eve = events;

	YEVE_FOREACH(eve, events) {
		int t = ywidEveType(eve);
		if (t == YKEY_MOUSEDOWN) {
			*button =  ywidEveKey(eve);
			return 1;
		}
	}
	return 0;
}

int yevMouseUp(Entity *events, int *button)
{
	Entity *eve = events;

	YEVE_FOREACH(eve, events) {
		int t = ywidEveType(eve);
		if (t == YKEY_MOUSEUP) {
			*button =  ywidEveKey(eve);
			return 1;
		}
	}
	return 0;
}

int yeveMouseX(void)
{
	return ywidXMouseGlobalPos;
}

int yeveMouseY(void)
{
	return ywidYMouseGlobalPos;
}
