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

#ifndef ENTITY_MACRO_H_
#define ENTITY_MACRO_H_

#include "utils.h"

#define YET_TYPE_MIX_int Int
#define YET_TYPE_MIX_INT Int
#define YET_TYPE_MIX_Int Int

#define YET_TYPE_MIX_Float Float
#define YET_TYPE_MIX_float Float
#define YET_TYPE_MIX_FLOAT Float

#define YET_TYPE_MIX_String String
#define YET_TYPE_MIX_string String
#define YET_TYPE_MIX_STRING String

#define YET_TYPE_MIX_Array Array
#define YET_TYPE_MIX_array Array
#define YET_TYPE_MIX_ARRAY Array

#define YET_TO_CASE_MIX(t) YUI_CAT(YET_TYPE_MIX_, t)

#define yeAutoFree __attribute__ ((cleanup(yeAutoFreeDestroy)))


#define YE_NEW__Int(val...) yeCreateInt(val, NULL, NULL)
#define YE_NEW__Float(val...) yeCreateFloat(val, NULL, NULL)
#define YE_NEW__String(val...) yeCreateString(val, NULL, NULL)

#define YE_NEW__Array(val...)			\
  yeCreateArray(NULL, NULL);

#define YE_NEW_(type, var, val...)				\
  yeAutoFree Entity *var = YUI_CAT(YE_NEW__, type)(val);

#define YE_NEW(type, variable, val...)		\
  YE_NEW_(YET_TO_CASE_MIX(type), variable, val)


#endif
