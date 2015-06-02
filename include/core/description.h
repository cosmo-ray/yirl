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

/*
 * This file contain the function used to serealize and deserealize entity 
 */


#ifndef _DESCRIPTION_H_
#define _DESCRIPTION_H_

#include "utils.h"
#include "entity.h"

typedef struct {
  Entity *(*fromFile)(void *opac, const char *fileName);
  int (*toFile)(void *opac, const char *fileName, Entity *entity);
  int (*destroy)(void *opac);
} YDescriptionOps;

static inline int ydToFile(void *opac, char *name, Entity *entity)
{
  return ((YDescriptionOps *)opac)->toFile(opac, name, entity);
}

static inline Entity *ydFromFile(void *opac, char *name)
{
  return ((YDescriptionOps *)opac)->fromFile(opac, name);
}

int ydRegister(void *(*allocator)(void));

int ydUnregiste(int t);


/**
 * @args the arguments 
 * @type the type of script
 * @return the new dm
 */
void *ydNewManager(int type);

/**
 * @scr the opaque scriptionManager object 
 */
int ydDestroyManager(void *dm);


#endif
