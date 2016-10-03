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

#include "yscript.h"

#define inst_compille(ascii_code, label_name, nb_args)	\
	case ascii_code:				\
	script[i] = (uint64_t) &&label_name;		\
	i += (nb_args + 1);				\
	break

Entity *yscript_exec(Entity *stack, int64_t *script)
{
  Entity *ret = NULL;
  int64_t *origin = script;

  if (unlikely(!*script)) { // compille time :p
	  int i;

	  script[0] = 1;
	  for (i = 1; script[i] != 'e' && script[i] != 'E';) {
		  switch (script[i]) {
			  inst_compille('+', add, 3);
			  inst_compille('-', sub, 3);
			  inst_compille('/', div, 3);
			  inst_compille('*', mult, 3);
			  inst_compille('<', inf_comp, 3);
			  inst_compille('j', jmp, 1);
			  inst_compille('i', create_int, 1);
			  inst_compille('u', unstack, 0);
		  default:
			  abort();
		  }
	  }
	  if (script[i] == 'e')
		  script[i] = (uint64_t)&&end;
	  else if (script[i] == 'E')
		  script[i] = (uint64_t)&&end_ret;
	  else
		  abort();
  }
#undef inst_compille

  ++script;
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
inf_comp:
  if (yeGetIntDirect(yeGetByIdxDirect(stack, script[1])) <
      yeGetIntDirect(yeGetByIdxDirect(stack, script[2]))) {
	  script = origin + script[3];
	  goto *((void *)*script);
  }
  script += 4;
  goto *((void *)*script);
		 
jmp:
  script = origin + script[1];
  goto *((void *)*script);

create_int:
  ++script;
  yeCreateInt(*script, stack, NULL);
  ++script;
  goto *((void *)*script);
unstack:
  yePopBack(stack);

  /* return */

end_ret:
  ++script;
  ret = yeGet(stack, *script);
  yeIncrRef(ret);
end:
  yeClearArray(stack);
  return ret;
}
