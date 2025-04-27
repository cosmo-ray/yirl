/*
**Copyright (C) 2020-2025 Matthias Gatto
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

#include <kuroko/kuroko.h>
#include <kuroko/vm.h>
#include <kuroko/util.h>
#ifdef likely
#undef likely
#endif
#ifdef unlikely
#undef unlikely
#endif
#include <yirl/all.h>

static int t = -1;
static void *cur_manager;

struct YScriptKrk {
	YScriptOps ops;
};

static int init(void *sm, void *args)
{
	printf("krk init\n");
	/*
	  int main(int argc, char *argv[]) {
	  krk_initVM(0);
	  krk_startModule("__main__");
	  krk_interpret("print('hello, world')", "<stdin>");
	  krk_freeVM();
	  return 0;
	  }
	*/
	return 0;
}


void *call(void *sm, const char *name, int nb, union ycall_arg *args,
	   int *types)
{
	printf("call\n");
	return NULL;
}

static struct ys_ret call2(void *sm, const char *name, int nb, union ycall_arg *args,
			   int *types)
{
	printf("call 2\n");
	return (struct ys_ret){.t=YS_VPTR, .v.vptr=0};
}

static int loadFile(void *s, const char *file)
{
	printf("loadFile\n");
	return 0;
}

static int loadString(void *s, const char *str)
{
	printf("loadString\n");
	return 0;
}

static int destroy(void *sm)
{
	printf("destroy\n");
	return 0;
}

static void addFuncSymbole(void *sm, const char *name, int nbArgs, Entity *func)
{
	printf("addFuncSymbole\n");
}

static void *allocator(void)
{
	struct YScriptKrk *ret;

	ret = calloc(1, sizeof(*ret));
	ret->ops.init = init;
	ret->ops.destroy = destroy;
	ret->ops.loadFile = loadFile;
	ret->ops.loadString = loadString;
	ret->ops.call = call;
	ret->ops.call2 = call2;
	ret->ops.addFuncSymbole = addFuncSymbole;
	return (void *)ret;
}

int ysKrkInit(void)
{
	t = ysRegister(allocator);
	return t;
}

int ysKrkEnd(void)
{
	return ysUnregiste(t);
}

int ysKrkGetType(void)
{
	return t;
}
