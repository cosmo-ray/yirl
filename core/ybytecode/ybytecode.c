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
#include <inttypes.h>
#include "entity-script.h"
#include "ybytecode-script.h"
#include "ybytecode.h"
#include "game.h"

#define inst_compille(ascii_code, label_name, nb_args)	\
  case ascii_code:					\
  script[i] = (int_ptr_t) &&label_name;			\
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
	inst_compille(YB_ADD, add, 3);
	inst_compille(YB_INCR, yb_incr, 1);
	inst_compille(YB_SUB, sub, 3);
	inst_compille(YB_DIV, div, 3);
	inst_compille(YB_MULT, mult, 3);
	inst_compille(YB_INF, inf_comp, 3);
	inst_compille(YB_NOT_EQUAL_NBR, yb_not_equal_comp_nbr, 3);
	inst_compille(YB_EQUAL_NBR, yb_equal_comp_nbr, 3);
	inst_compille(YB_EQUAL, qeual_comp, 3);
	inst_compille(YB_INF_COMP_NBR, yb_inf_comp_nbr, 3);
	inst_compille(YB_SUP_COMP_NBR, yb_sup_comp_nbr, 3);
	inst_compille(YB_SUP, sup_comp, 3);
	inst_compille(YB_JMP, jmp, 1);
	inst_compille(YB_JMP_IF_0, jmp_if_0, 1);
	inst_compille(YB_CREATE_STRING, create_string, 1);
	inst_compille(YB_CREATE_INT, create_int, 1);
	inst_compille(YB_CREATE_ARRAY, create_array, 0);
	inst_compille(YB_SET_INT, set_int, 2);
	inst_compille(YB_STRING_ADD_CH, string_add_ch, 2);
	inst_compille(YB_STRING_ADD_CH_ENT, string_add_ch_ent, 2);
	inst_compille(YB_NEW_WIDGET, new_widget, 2);
	inst_compille(YB_REGISTRE_WIDGET_SUBTYPE, wid_add_subtype, 1);
	inst_compille(YB_PUSH_BACK, push_back, 3);
	inst_compille(YB_BRUTAL_CAST, brutal_cast, 2);
	inst_compille(YB_PRINT_POS, print_pos, 0);
	inst_compille(YB_PRINT_IRET, print_iret, 0);
	inst_compille(YB_PRINT_ENTITY, print_entity, 1);
	inst_compille(YB_STACK_POP, stack_pop, 0);
	inst_compille(YB_LEAVE, end, 0);
	inst_compille(YB_RETURN, end_ret, 1);
	inst_compille(YB_YG_GET_PUSH, yg_get_push, 1);
	inst_compille(YB_YG_GET_PUSH_INT, yg_get_push_int, 1);
	inst_compille(YB_GET_AT_IDX, get_at_idx, 2);
	inst_compille(YB_GET_AT_STR, get_at_str, 2);
      case YB_CALL:
	script[i] = (int_ptr_t) &&call_entity;
	i += (script[i + 1] + 3);
	break;
      case YB_COMPILLE_FUNC:
	script[i] = (int_ptr_t) &&compille_func;
	++neested;
	i += 3;
	break;
      case YB_END_FUNC:
	script[i] = (int_ptr_t)&&end;
	if (!neested)
	  goto out_loop;
	--neested;
	++i;
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
	    g_strdup_printf("instruction '" PRIint64 "' at %d is not valide",
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

 string_add_ch_ent:
  iret = yeStringAddChByEntity(yeGetByIdxDirect(stack, script[1]), yeGetByIdxDirect(stack, script[2]));
  script += 3;
  goto *((void *)*script);

 string_add_ch:
  iret = yeStringAddCh(yeGetByIdxDirect(stack, script[1]), script[2]);
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

 yb_not_equal_comp_nbr:
  if (yeGetIntDirect(yeGetByIdxDirect(stack, script[1])) != script[2]) {
    script = origin + script[3];
    goto *((void *)*script);
  }
  script += 4;
  goto *((void *)*script);

 yb_equal_comp_nbr:
  if (yeGetIntDirect(yeGetByIdxDirect(stack, script[1])) == script[2]) {
    script = origin + script[3];
    goto *((void *)*script);
  }
  script += 4;
  goto *((void *)*script);

 qeual_comp:
  if (yeGetIntDirect(yeGetByIdxDirect(stack, script[1])) ==
      yeGetIntDirect(yeGetByIdxDirect(stack, script[2]))) {
    script = origin + script[3];
    goto *((void *)*script);
  }
  script += 4;
  goto *((void *)*script);

 yb_sup_comp_nbr:
  if (yeGetIntDirect(yeGetByIdxDirect(stack, script[1])) > script[2]) {
    script = origin + script[3];
    goto *((void *)*script);
  }
  script += 4;
  goto *((void *)*script);

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
  /* No goto on purpose here */
  /* fall-though */
 jmp:
  script = origin + script[1];
  goto *((void *)*script);

 call_entity:
  ++YBytecodeScriptDirectReturn;
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
  if (yeIsPtrAnEntity(ret)) {
    if (yeType(ret) == YINT) {
      iret = yeGetIntDirect(ret);
    }
    yePushBack(stack, ret, NULL);
    yeDestroy(ret);
  } else {
    iret = (intptr_t)ret;
  }
  script += script[1] + 3;
  --YBytecodeScriptDirectReturn;
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
  script[0] = (int_ptr_t)&&create_func;
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

 stack_pop:
  yePopBack(stack);
  ++script;
  goto *((void *)*script);

 create_array:
  yeCreateArray(stack, NULL);
  ++script;
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

 get_at_idx:
  yePushBack(stack, yeGet(yeGetByIdxDirect(stack, script[1]), script[2]), NULL);
  script += 3;
  goto *((void *)*script);

 get_at_str:
  yePushBack(stack, yeGet(yeGetByIdxDirect(stack, script[1]),
			  (const char *)script[2]), NULL);
  script += 3;
  goto *((void *)*script);

 yg_get_push_int:
  {
    ++script;
    Entity *tmp = ygGet((const char *)*script);
    ++script;
    if (yeType(tmp) != YINT) {
      yeCreateInt(0, stack, NULL);
      iret = -1;
    }
    iret = yePushBack(stack, tmp, NULL);
    goto *((void *)*script);
  }
 yg_get_push:
  ++script;
  iret = yePushBack(stack, ygGet((const char *)*script), NULL);
  ++script;
  goto *((void *)*script);

 new_widget:
  yeCreateData(ywidNewWidget(yeGetByIdxDirect(stack, script[1]),
			     (const char *)script[2]),
	       stack, NULL);
  script += 3;

 wid_add_subtype:
  ywidAddSubType(yeGetByIdxDirect(stack, script[1]));
  yeIncrRef(yeGetByIdxDirect(stack, script[1]));
  script += 2;
  goto *((void *)*script);

 print_iret:
  printf("iret: %d\n", iret);
  ++script;
  goto *((void *)*script);

 print_pos:
  printf("script instruction pos: "PRIiptr"\n",
	 script - origin);
  ++script;
  goto *((void *)*script);
 print_entity:
  {
    char *ctmp = yeToCStr(yeGetByIdxDirect(stack, script[1]), 1, 0);

    printf("%s\n", ctmp);
    g_free(ctmp);
    script += 2;
  }
  goto *((void *)*script);

 push_back:
  yePushBack(yeGetByIdxDirect(stack, script[1]),
	     yeGetByIdxDirect(stack, script[2]),
	     (char *)script[3]);
  script += 4;
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
