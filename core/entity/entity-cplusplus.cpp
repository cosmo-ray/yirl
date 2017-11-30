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

extern "C" {
#include "entity.h"
#include <stdlib.h>
}

#if defined(__APPLE__)
void *__cxa_call_unexpected = 0;
void *__cxa_pure_virtual = 0;
void *__gxx_personality_v0 = 0;

namespace __cxxabiv1 {
  class __class_type_info {
    virtual void dummy();
  };
  void __class_type_info::dummy() { }  // causes the vtable to get created here
  class __si_class_type_info {
    virtual void dummy();
  };
  void __si_class_type_info::dummy() { }
}


void operator delete(void *ptr) {
  free(ptr);
}

void operator delete(void *ptr, long unsigned int end)
{
  free(ptr);
}

#endif

void *operator new(size_t l)
{
  void *ret = malloc(l);
  return ret;
}

Entity *yeCreateArray(Entity *fathers, const char *name)
{
  return yeCreateArrayByCStr(fathers, name);
}
