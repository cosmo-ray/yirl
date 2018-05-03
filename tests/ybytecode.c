#include <glib.h>

#include "game.h"
#include "entity-script.h"
#include "tests.h"
#include "ybytecode.h"
#include "ybytecode-script.h"
#include "condition.h"

void ysciptAdd(void)
{
  yeInitMem();
  Entity *args = yeCreateArrayExt(NULL, NULL,
				  YBLOCK_ARRAY_NOINIT |
				  YBLOCK_ARRAY_NOMIDFREE);

  int64_t test0[] = {0,
		     YB_CREATE_INT, 4, //stack 4
		     YB_CREATE_INT, 3, //stack 3
		     //stack 0 will be use to store resulte
		     YB_CREATE_INT, 0,
		     // add stack 0 to stack 1
		     // and store resulte in stack 2
		     YB_ADD, 0, 1, 2,
		     YB_RETURN, 2, // return 2nd elem
		     YB_END_FUNC
  };

  for (int i = 0; i < 5000000; ++i) {
    yeDestroy(ybytecode_exec(args, test0));
  }
  yeDestroy(args);
  yeEnd();
}

void yscriptBenchLoop(void)
{
  yeInitMem();
  Entity *args = yeCreateArrayExt(NULL, NULL,
				  YBLOCK_ARRAY_NOINIT |
				  YBLOCK_ARRAY_NOMIDFREE);
  Entity *tmp;

  // this is a simple for(int i = 0; i < 500 000; ++i)
  int64_t test1[] = {0,
		     YB_CREATE_INT, 0, //stack[0] = 2 // 2
		     YB_JMP, 7, // jmp to loop comparaison // 4
		     YB_INCR, 0, // 6
		     // if stack 0 is iferior to stack[2], goto 5
		     YB_INF_COMP_NBR, 0, 50000000, 5,
		     YB_CREATE_INT, 2, // stack[1] = 2
		     YB_MULT, 0, 1, 0, // mult stack[0] by 2
		     YB_RETURN, 0, // return 2nd elem
		     YB_END_FUNC
  };

  tmp = ybytecode_exec(args, test1);
  g_assert(yeGetIntDirect(tmp) == (50000000 * 2));
  yeDestroy(tmp);
  yeDestroy(args);
  yeEnd();
}

void yscriptLoop(void)
{
  yeInitMem();
  Entity *args = yeCreateArrayExt(NULL, NULL,
				  YBLOCK_ARRAY_NOINIT |
				  YBLOCK_ARRAY_NOMIDFREE);
  Entity *tmp;

  // this is a simple for(int i = 0; i < 500 000; ++i)
  int64_t test1[] = {0,
		     YB_CREATE_INT, 0, //stack 1 - 2
		     YB_CREATE_INT, 1, //stack 0 - 4
		     YB_CREATE_INT, 5000, // 6
		     YB_JMP, 13, // jmp to loop - 8
		     YB_ADD, 0, 1, 0,
		     // if stack 0 is iferior to stack 2, goto 9
		     YB_INF, 0, 2, 9,
		     YB_CREATE_INT, 2, // stack 2 in 3
		     YB_MULT, 0, 3, 0, // mult 0 by 2
		     YB_RETURN, 0, // return 2nd elem
		     YB_END_FUNC
  };

  tmp = ybytecode_exec(args, test1);
  g_assert(yeGetIntDirect(tmp) == (5000 * 2));
  yeDestroy(tmp);
  yeDestroy(args);
  yeEnd();
}

void ybytecodeScript(void)
{
  void *sm = NULL;
  int64_t test1[] = {0,
  		     YB_CREATE_INT, 0, //stack 1 - 2
  		     YB_CREATE_INT, 1, //stack 0 - 4
  		     YB_CREATE_INT, 50000000, // 6
  		     YB_ADD, 1, 2, 0,
  		     YB_RETURN, 0,
  		     YB_END_FUNC
  };
  int64_t testWithArgs[] = {0,
  			    YB_BRUTAL_CAST, 0, YINT,
  			    YB_BRUTAL_CAST, 1, YINT,
  			    YB_ADD, 0, 1, 0,
  			    YB_RETURN, 0,
  			    YB_END_FUNC
  };

  int64_t testWithEntArgs[] = {0,
			       YB_CREATE_INT, 0,
			       YB_ADD, 0, 1, 2,
			       YB_RETURN, 2,
			       YB_END_FUNC
  };
  Entity *gc, *i0, *i1;
  yeInitMem();
  gc = yeCreateArray(NULL, NULL);
  i0 = yeCreateInt(7, gc, NULL);
  i1 = yeCreateInt(8, gc, NULL);

  sm = ysYBytecodeManager();
  g_assert(sm);

  g_assert(!ysRegistreFunc(sm, "add", test1));
  g_assert(!ysRegistreFunc(sm, "argsadd", testWithArgs));
  g_assert(!ysRegistreFunc(sm, "entargsadd", testWithEntArgs));
  g_assert((uint64_t)ysCall(sm, "add") == 50000001);
  g_assert((uint64_t)ysCall(sm, "argsadd", 50000000, 1) == 50000001);
  g_assert((uint64_t)ysCall(sm, "add") == 50000001);
  g_assert((uint64_t)ysCall(sm, "argsadd", 1, 2) == 3);
  g_assert((uint64_t)ysCall(sm, "entargsadd", i0, i1) == (8 + 7));
  g_assert((uint64_t)ysCall(sm, "entargsadd", i0, i1) == (8 + 7));

  g_assert(!ysYBytecodeEnd());
  yeDestroy(gc);
  yeEnd();
}


void ybytecodeLoopCallFunction(void)
{
  int64_t test1[] = {0,
		     YB_CREATE_STRING, (uint64_t)"testFunc", //stack "testFunc"
		     YB_COMPILLE_FUNC, 0, 1, // function with name at stack 0
		     YB_ADD, 0, 1, 0, // add stack 0 to stack 0
		     // store result in stack 0
		     YB_END_FUNC,
		     YB_CREATE_INT, 0,
		     YB_CREATE_INT, 1,
		     YB_CREATE_INT, 5000000,
		     YB_JMP, 25,
		     YB_CALL, 2, 1, 2, 3, // 2 arguments, call stack 1,
				      //with arguments at stack 2 and 3
		     YB_STACK_POP,
		     YB_INF, 2, 4, 19,
		     YB_RETURN, 2, // return 2nd elem, so 15 , 52
		     YB_END_FUNC
  };
  void *sm = NULL;

  yeInitMem();
  sm = ysYBytecodeManager();
  g_assert(sm);

  g_assert(!ysRegistreFunc(sm, "test", test1));
  g_assert((uint64_t)ysCall(sm, "test") == 5000000);

  g_assert(!ysYBytecodeEnd());
  yeEnd();
}

void ybytecodeAddFunction(void)
{
  int64_t test1[] = {0,
		     YB_CREATE_STRING, (uint64_t)"testFunc", //stack "testFunc"
		     YB_COMPILLE_FUNC, 0, 1, // function with name at stack 0
		     YB_ADD, 0, 1, 0, // add stack 0 to stack 0
		     // store result in stack 0
		     YB_END_FUNC, // end function
		     YB_CREATE_INT, 15,
		     YB_CREATE_INT, 52,
		     YB_CALL, 2, 1, 2, 3, // 2 arguments, call stack 1,
				      //with arguments at stack 2 and 3
		     YB_RETURN, 2, // return 2nd elem, so 15 , 52
		     YB_END_FUNC
  };
  void *sm = NULL;

  yeInitMem();
  sm = ysYBytecodeManager();
  g_assert(sm);

  g_assert(!ysRegistreFunc(sm, "test", test1));
  for (int i = 0; i < 5000000; ++i)
    g_assert((uint64_t)ysCall(sm, "test") == (52 + 15));

  g_assert(!ysYBytecodeEnd());
  yeEnd();
}

void ybytecodeConditions(void)
{
  void *sm;
  Entity *condition;

  yeInitMem();
  sm = ysYBytecodeManager();
  g_assert(sm);
  condition = yeCreateArray(NULL, NULL);
  yeCreateString(">", condition, NULL);
  yeCreateInt(1, condition, NULL);
  yeCreateInt(0, condition, NULL);
  g_assert(yeCheckCondition(condition));
  g_assert(!ysYBytecodeEnd());
  yeDestroy(condition);
  yeEnd();
}


void ybytecodeReadFile(void)
{
  void *sm;
  GameConfig cfg;
  Entity *gc;
  Entity *num;
  Entity *func;

  ygInit( ({ ygInitGameConfig(&cfg, "./", NONE); &cfg; }) );
  sm = ysYBytecodeManager();
  g_assert(sm);

  ysLoadFile(sm, TESTS_PATH"/add.yb");
  gc = yeCreateArray(NULL, NULL);
  func = yeCreateFunction("add", ysYBytecodeManager(), gc, NULL);
  g_assert(func);
  num = yeCreateInt(1, gc, NULL);

  yesCall(func, num, num);
  g_assert(yeGetInt(num) == 2);

  func = yeCreateFunction("add1", ysYBytecodeManager(), gc, NULL);
  yesCall(func, num);
  g_assert(yeGetInt(num) == 3);

  func = yeCreateFunction("sub", ysYBytecodeManager(), gc, NULL);
  yesCall(func, num, num);
  g_assert(yeGetInt(num) == 0);

  func = yeCreateFunction("add2", ysYBytecodeManager(), gc, NULL);
  yesCall(func, num);
  g_assert(yeGetInt(num) == 2);

  g_assert(!ysYBytecodeEnd());
  yeDestroy(gc);
  ygEnd();
  ygCleanGameConfig(&cfg);
}
