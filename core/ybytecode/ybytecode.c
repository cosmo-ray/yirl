/*
**Copyright (C) 2016 Matthias Gatto
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

#include <glib.h>
#include <ctype.h>
#include "entity-script.h"
#include "ybytecode-script.h"
#include "ybytecode.h"
#include "game.h"

#define inst_compille(ascii_code, label_name, nb_args)	\
  case ascii_code:					\
  script[i] = (uint64_t) &&label_name;			\
  i += (nb_args + 1);					\
  break

char *ybytecode_error;

Entity *ybytecode_exec(Entity *stack, int64_t *script)
{
  Entity *ret = NULL;
  int64_t *origin = script;
  int64_t *tmp;
  int iret = 0;

  if (unlikely((!*script))) { // compille time :p
    script[0] = 1;

    for (int i = 1, neested = 0 ;;) {
      switch (script[i]) {
	inst_compille('+', add, 3);
	inst_compille(YB_INCR, yb_incr, 1);
	inst_compille('-', sub, 3);
	inst_compille('/', div, 3);
	inst_compille('*', mult, 3);
	inst_compille('<', inf_comp, 3);
	inst_compille(YB_INF_COMP_NBR, yb_inf_comp_nbr, 3);
	inst_compille('>', sup_comp, 3);
	inst_compille('j', jmp, 1);
	inst_compille(JMP_IF_0, jmp_if_0, 1);
	inst_compille('s', create_string, 1);
	inst_compille('i', create_int, 1);
	inst_compille('I', set_int, 2);
	inst_compille(YB_BRUTAL_CAST, brutal_cast, 2);
	inst_compille(YB_YG_GET_PUSH, yg_get_push, 1);
      case 'c':
	script[i] = (uint64_t) &&call_entity;
	i += (script[i + 1] + 3);
	break;
      case 'F':
	script[i] = (uint64_t) &&compille_func;
	++neested;
	i += 3;
	break;
      case 'e':
	script[i] = (uint64_t)&&end;
	if (!neested)
	  goto out_loop;
	--neested;
	++i;
	break;
      case 'E':
	script[i] = (uint64_t)&&end_ret;
	if (!neested)
	  goto out_loop;
	--neested;
	i += 2;
	break;

      default:
	if (ybytecode_error) {
	  g_free(ybytecode_error);
	  ybytecode_error = NULL;
	}
	if (isprint(script[i])) {
	  ybytecode_error =
	    g_strdup_printf("instruction '%c' at %d is not valide",
			    (char)script[i], i);
	} else {
	  ybytecode_error =
	    g_strdup_printf("instruction '%ld' at %d is not valide",
			    script[i], i);
	}
	return NULL;
      }
    }
  }
 out_loop:

  ++script;
  goto *((void *)*script);

 brutal_cast:

  yeBrutalCast(yeGetByIdxDirect(stack, script[1]),
	       (size_t)script[2]);
  script += 3;
  goto *((void *)*script);

 set_int:
  yeSetIntDirect(yeGetByIdxDirect(stack, script[1]), script[2]);
  script += 3;
  goto *((void *)*script);

 yb_incr:
  yeIncrementIntDirect(yeGetByIdxDirect(stack, script[1]));
  script +=2;
  goto *((void *)*script);
 add:
  yeSetIntDirect(yeGetByIdxDirect(stack, script[3]),
		 yeGetIntDirect(yeGetByIdxDirect(stack, script[1])) +
		 yeGetIntDirect(yeGetByIdxDirect(stack, script[2]))
		 );
  script += 4;
  goto *((void *)*script);

 sub:
  yeSetIntDirect(yeGetByIdxDirect(stack, script[3]),
		 yeGetIntDirect(yeGetByIdxDirect(stack, script[1])) -
		 yeGetIntDirect(yeGetByIdxDirect(stack, script[2]))
		 );
  script += 4;
  goto *((void *)*script);

 div:
  yeSetIntDirect(yeGetByIdxDirect(stack, script[3]),
		 yeGetIntDirect(yeGetByIdxDirect(stack, script[1])) /
		 yeGetIntDirect(yeGetByIdxDirect(stack, script[2]))
		 );
  script += 4;
  goto *((void *)*script);

 mult:
  yeSetIntDirect(yeGetByIdxDirect(stack, script[3]),
		 yeGetIntDirect(yeGetByIdxDirect(stack, script[1])) *
		 yeGetIntDirect(yeGetByIdxDirect(stack, script[2]))
		 );
  script += 4;
  goto *((void *)*script);

  /* stack manipulations */
 yb_inf_comp_nbr:
  if (yeGetIntDirect(yeGetByIdxDirect(stack, script[1])) < script[2]) {
    script = origin + script[3];
    goto *((void *)*script);
  }
  script += 4;
  goto *((void *)*script);

 inf_comp:
  if (yeGetIntDirect(yeGetByIdxDirect(stack, script[1])) <
      yeGetIntDirect(yeGetByIdxDirect(stack, script[2]))) {
    script = origin + script[3];
    goto *((void *)*script);
  }
  script += 4;
  goto *((void *)*script);

 sup_comp:
  if (yeGetIntDirect(yeGetByIdxDirect(stack, script[1])) >
      yeGetIntDirect(yeGetByIdxDirect(stack, script[2]))) {
    script = origin + script[3];
    goto *((void *)*script);
  }
  script += 4;
  goto *((void *)*script);

 jmp_if_0:
  if (iret) {
    iret = 0;
    script += 2;
    goto *((void *)*script);
  }
 jmp:
  script = origin + script[1];
  goto *((void *)*script);

 call_entity:
  YBytecodeScriptDirectReturn = 1;
  switch (script[1]) {
  case 0:
    ret = yesCall(yeGetByIdxDirect(stack, script[2]));
    break;
  case 1:
    ret = yesCall(yeGetByIdxDirect(stack, script[2]),
		  yeGetByIdxDirect(stack, script[3]));
    break;
  case 2:
    ret = yesCall(yeGetByIdxDirect(stack, script[2]),
		  yeGetByIdxDirect(stack, script[3]),
		  yeGetByIdxDirect(stack, script[4]));
    break;
  case 3:
    ret = yesCall(yeGetByIdxDirect(stack, script[2]),
		  yeGetByIdxDirect(stack, script[3]),
		  yeGetByIdxDirect(stack, script[4]),
		  yeGetByIdxDirect(stack, script[5]));
    break;
  }
  script += script[1] + 3;
  if (ret) {
    yePushBack(stack, ret, NULL);
    yeDestroy(ret);
  }
  YBytecodeScriptDirectReturn = 0;
  goto *((void *)*script);

 create_func:
  yeCreateFunction(yeGetString(yeGetByIdxDirect(stack, script[1])),
		   ysYBytecodeManager(), stack, NULL);
  script += script[2];
  goto *((void *)*script);
 compille_func:

  ysRegistreFunc(ysYBytecodeManager(),
		 yeGetString(yeGetByIdxDirect(stack, script[1])),
		 &script[2]);
  yeCreateFunction(yeGetString(yeGetByIdxDirect(stack, script[1])),
		   ysYBytecodeManager(), stack, NULL);
  script[0] = (uint64_t)&&create_func;
  script += 2;
  tmp = script;
  ++script;
  for (int nested = 0; nested || ((void *)*script != &&end_ret &&
				  (void *)*script != &&end);
       ++script) {
    if ((void *)*script == &&create_func)
      ++nested;
    else if (((void *)*script == &&end_ret || (void *)*script == &&end))
      --nested;
  }
  if ((void *)*script == &&end_ret)
    ++script;
  ++script;
  *tmp = script - tmp + 2;
  goto *((void *)*script);

 create_int:
  ++script;
  yeCreateInt(*script, stack, NULL);
  ++script;
  goto *((void *)*script);

 create_string:
  ++script;
  yeCreateString((const char *)*script, stack, NULL);
  ++script;
  goto *((void *)*script);

 yg_get_push:
  ++script;
  iret = yePushBack(stack, ygGet((const char *)*script), NULL);
  ++script;
  goto *((void *)*script);

end_ret:
  ++script;
  ret = yeGet(stack, *script);
  yeIncrRef(ret);
 end:
  yeClearArray(stack);
  return ret;
}

#undef inst_compille
