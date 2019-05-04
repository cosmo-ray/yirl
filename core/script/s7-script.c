#include <glib.h>
#include "s7-script.h"
#include "s7.h"
#include "game.h"
#include "widget.h"

static int t = -1;

#define GET_OPS(sm) (((YScriptS7 *)sm)->ops)
#define GET_S7(sm) (((YScriptS7 *)sm)->s7)

typedef struct {
	YScriptOps ops;
	s7_scheme *s7;
} YScriptS7;

static s7_pointer int_cast(s7_scheme *sc, s7_pointer args)
{
	return s7_make_integer(sc, (intptr_t)s7_c_pointer(s7_car(args)));
}

static s7_pointer ptr_cast(s7_scheme *sc, s7_pointer args)
{
	return s7_make_c_pointer(sc, (void *)s7_integer(s7_car(args)));
}

static s7_pointer string_cast(s7_scheme *sc, s7_pointer args)
{
	return s7_make_string(sc, (void *)s7_c_pointer(s7_car(args)));
}

static s7_pointer s7ywidNewWidget(s7_scheme *sc, s7_pointer a)
{
	return s7_make_c_pointer(sc, ywidNewWidget(s7_c_pointer(s7_car(a)),
						   s7_string(s7_cdr(a))));
}

#define S7MP s7_make_c_pointer

#define S7_IMPLEMENT_E_SES(func)					\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7MP(s,						\
			    func(					\
				    s7_string(s7_list_ref(s, a, 0)),	\
				    s7_c_pointer(s7_list_ref(s, a, 1)),	\
				    s7_string(s7_list_ref(s, a, 2))	\
				    ));					\
	}

S7_IMPLEMENT_E_SES(yeCreateString);

#define S7_IMPLEMENT_I_E(func)						\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		return s7_make_integer(s,				\
				       func(s7_c_pointer(s7_car(a))));	\
	}

S7_IMPLEMENT_I_E(yeGetInt);

#define S7_DEF_F(name, nargs, optargs)					\
	s7_define_safe_function(s7, #name, s7##name, nargs, optargs, false, "")

static int init(void *sm, void *args)
{
	s7_scheme *s7;

	(void)args;
	s7 = s7_init();
	if (s7 == NULL)
		return -1;
	GET_S7(sm) = s7;
	s7_define_safe_function(s7, "int_cast", int_cast, 1, 0, false, "");
	s7_define_safe_function(s7, "string_cast", string_cast, 1, 0, false, "");
	s7_define_safe_function(s7, "str_cast", string_cast, 1, 0, false, "");
	s7_define_safe_function(s7, "ptr_cast", ptr_cast, 1, 0, false, "");
	S7_DEF_F(yeGetInt, 1, 0);
	S7_DEF_F(ywidNewWidget, 2, 0);
	S7_DEF_F(yeCreateString, 1, 2);
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

	printf("nb: %d\n", nbArg);
	switch (nbArg) {
	case 1:
		a = s7_cons(s, CPTR(0), s7_nil(s));
		break;
	default:
		printf("2 args %p %p\n", args[0], args[1]);
		a = s7_list(s, nbArg, CPTR(0), CPTR(1), CPTR(2),
			    CPTR(3), CPTR(4), CPTR(5),
			    CPTR(6), CPTR(7), CPTR(8),
			    CPTR(9), CPTR(10), CPTR(11));
		break;
	}
	r = s7_call(s, f, a);
	if (s7_is_c_pointer(r))
		return s7_c_pointer(r);
	if (s7_is_integer(r))
		return (void *)s7_integer(r);
	return NULL;
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

