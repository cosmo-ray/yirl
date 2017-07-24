/*
**Copyright (C) 2017 Matthias Gatto
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

/* !!!!!! ATTENTION !!!!!!
 * DO NOT INCLUDE THIS FILE, include entity.h instead
 */

/**
 * @brief Add @str to the string entity @Ent
 */
int yeStringAdd(Entity *ent, const char *str);

/**
 * @brief same as yeStringAdd, but add a new line
 */
int yeStringAddNl(Entity *ent, const char *str);

/**
 * @brief Count the number of @carac in @ent
 *
 * @ent a string entity
 * @lineLimit hard to explain, use -1, or go read the code...
 * @return the number a @carac in @ent
 * @examples yeCountCharacters(str, '\n', -1), will return the number of
 *	     lines in str
 */
int yeCountCharacters(Entity *ent, char carac, int lineLimit);

int yeAddStrFromFd(Entity *e, int fd, int len);
int yeStringAddInt(Entity *ent, int i);
int yeStringAddLong(Entity *ent, long i);

