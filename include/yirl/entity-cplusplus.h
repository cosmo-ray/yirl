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

#ifndef YIRL_ENTITY_CPLUSPLUS_H
#define YIRL_ENTITY_CPLUSPLUS_H

Entity *yeGetByIdx(Entity *entity, size_t index);
Entity *yeGetByStrFast(Entity *entity, const char *name);


extern "C++" {

  void *operator new(size_t l);
#if defined(__APPLE__)
  void operator delete(void *ptr);
  void operator delete(void*, long unsigned int);
#endif
  static inline Entity *yeGet(Entity *e, int idx)
  {
    return yeGetByIdx(e, idx);
  }

  static inline Entity *yeGet(Entity *e, const char *idx)
  {
    return yeGetByStrFast(e, idx);
  }

  Entity *yeCreateArray(Entity *fathers, const char *name);
}


#endif
