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

Entity *yscript_exec(Entity *stack, uint64_t *script)
{
  Entity *ret = NULL;

  if (!*script) { // compille time :p
	  int i;

	  script[0] = 1;
	  for (i = 1; script[i] != 'e' && script[i] != 'E';) {
		  switch (script[i]) {
			  inst_compille('+', add, 3);
			  inst_compille('-', sub, 3);
			  inst_compille('/', div, 3);
			  inst_compille('*', mult, 3);
			  inst_compille('s', stack, 1);
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
  }
#undef inst_compille

  ++script;
  goto *((void *)*script);

  /* Arithemetic operations */

add:
  yeSetInt(yeGet(stack, script[3]),
	   yeGetInt(yeGet(stack, script[1])) +
	   yeGetInt(yeGet(stack, script[2]))
	  );
  script += 4;
  goto *((void *)*script);
sub:
  yeSetInt(yeGet(stack, script[3]),
	   yeGetInt(yeGet(stack, script[1])) -
	   yeGetInt(yeGet(stack, script[2]))
	  );
  script += 4;
  goto *((void *)*script);
div:
  yeSetInt(yeGet(stack, script[3]),
	   yeGetInt(yeGet(stack, script[1])) /
	   yeGetInt(yeGet(stack, script[2]))
	  );
  script += 4;
  goto *((void *)*script);
mult:
  yeSetInt(yeGet(stack, script[3]),
	   yeGetInt(yeGet(stack, script[1])) *
	   yeGetInt(yeGet(stack, script[2]))
	  );
  script += 4;
  goto *((void *)*script);

  /* stack manipulations */

stack:
  ++script;
  yePushBack(stack, YE_TO_ENTITY(*script), NULL);
  ++script;
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
  YE_DESTROY(stack);
  return ret;
}

int main(void)
{
	Entity *args = yeCreateArray(NULL, NULL);
	// return (4 + 3)
	uint64_t test0[] = {0,
			    'i', 4, //stack 4
			    'i', 3, //stack 3
			    'i', 0, //stack 0 will be use to store resulte
			    '+', 0, 1, 2, // add stack 0 to stack 1
					  // and store resulte in stack 2
			    'E', 2 // return 2nd elem
	};

	printf("%d\n", yeGetInt(yscript_exec(args, test0)));
	args = yeCreateArray(NULL, NULL);
	printf("%d\n", yeGetInt(yscript_exec(args, test0)));
	for (int i = 0; i < 500000; ++i) {
		args = yeCreateArray(NULL, NULL);
		yscript_exec(args, test0);
	}
}
