#include <glib.h>
#include "s7-script.h"
#include "s7.h"

static int t = -1;

#define GET_OPS(sm) (((YScriptS7 *)sm)->ops)
#define GET_S7(sm) (((YScriptS7 *)sm)->s7)

typedef struct {
	YScriptOps ops;
	s7_scheme *s7;
} YScriptS7;

static int init(void *sm, void *args)
{
	s7_scheme *s7;

	(void)args;
	s7 = s7_init();
	if (s7 == NULL)
		return -1;
	GET_S7(sm) = s7;
	return 0;
}

static int destroy(void *sm)
{
	s7_quit(GET_S7(sm));
	free((YScriptS7 *)sm);
	return 0;
}

int loadFile(void *s, const char *file)
{
	s7_load(GET_S7(s), file);
	return 0;
}

int loadString(void *s, const char *str)
{
	s7_eval_c_string(GET_S7(s), str);
	return 0;
}

#define CPTR(idx) s7_make_c_pointer(s, args[idx])

static void *call(void *sm, const char *name, va_list ap)
{
	s7_scheme *s = GET_S7(sm);
	s7_pointer f = s7_name_to_value(s, name);
	s7_pointer r;
	s7_pointer a;
	int nbArg = 0;
	static void *args[16];

	if (!f)
		return NULL;

	for (void *tmp = va_arg(ap, void *); tmp != Y_END_VA_LIST;
	     tmp = va_arg(ap, void *)) {
		args[nbArg] = tmp;
		++nbArg;
	}

	switch (nbArg) {
	case 1:
		a = s7_cons(s, CPTR(0), s7_nil(s));
		break;
	case 2:
		a = s7_cons(s, CPTR(0), CPTR(1));
		break;
	default:
		a = s7_list(s, nbArg, CPTR(0), CPTR(1), CPTR(2),
			    CPTR(3), CPTR(4), CPTR(5),
			    CPTR(6), CPTR(7), CPTR(8),
			    CPTR(9), CPTR(10), CPTR(11));
		break;
	}
	r = s7_call(s, f, a);
	/* s7_integer(s7_call(s7, */
	/* 		   s7_name_to_value(s7, "add1"), */
	/* 		   s7_cons(s7, s7_make_integer(s7, 2), s7_nil(s7)))); */
	return r;
}

static void *allocator(void)
{
	YScriptS7 *ret;

	ret = g_new0(YScriptS7, 1);
	if (ret == NULL)
		return NULL;
	ret->s7 = NULL;
	ret->ops.init = init;
	ret->ops.destroy = destroy;
	ret->ops.loadFile = loadFile;
	ret->ops.loadString = loadString;
	ret->ops.call = call;
	ret->ops.getError = NULL;
	ret->ops.registreFunc = NULL;
	ret->ops.addFuncSymbole = NULL;
	return (void *)ret;
}

int ysS7Init(void)
{
	t = ysRegister(allocator);
	return t;
}

int ysS7End(void)
{
	return ysUnregiste(t);
}

int ysS7GetType(void)
{
	return t;
}

