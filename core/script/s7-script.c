#include <glib.h>
#include "s7-script.h"
#include "s7.h"
#include "game.h"
#include "canvas.h"
#include "widget.h"
#include "pos.h"

static int t = -1;

#define GET_OPS(sm) (((YScriptS7 *)sm)->ops)
#define GET_S7(sm) (((YScriptS7 *)sm)->s7)
#define GET_ET(sm) (((YScriptS7 *)sm)->et)

typedef struct {
	YScriptOps ops;
	s7_scheme *s7;
	s7_int et;
} YScriptS7;

YScriptS7 *s7m = NULL;

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
	return s7_make_c_pointer(sc, ywidNewWidget(s7_c_object_value(s7_car(a)),
						   s7_string(s7_cdr(a))));
}

static s7_pointer s7yeGet(s7_scheme *s, s7_pointer a)
{
	Entity *e = NULL;
	s7_pointer k = s7_list_ref(s, a, 1);
	Entity *ar = s7_c_object_value(s7_car(a));

	if (s7_is_string(k))
		e = yeGet(ar, s7_string(k));
	else
		e = yeGet(ar, s7_integer(k));
	return s7_make_c_object(s, s7m->et, e);
}

#define S7_END_CREATOR(X)							\
	Entity *father = s7_c_object_value(s7_list_ref(s, a, X));	\
	s7_pointer r = s7_make_c_object(s, s7m->et, ne);		\
	if (father)							\
		yePushBack(father, ne,					\
			   s7_string(s7_list_ref(s, a, X + 1)));	\
	return r;							\


#define S7_IMPLE_CREATOR(t, tf)						\
	static s7_pointer s7yeCreate##t(s7_scheme *s, s7_pointer a)	\
	{								\
		Entity *ne = yeCreate##t(tf(s7_list_ref(s, a, 0)),	\
					 NULL, NULL);			\
		S7_END_CREATOR(1);					\
	}

static s7_pointer s7yeCreateFunction(s7_scheme *s, s7_pointer a)
{
	Entity *ne = yeCreateFunction(s7_string(s7_list_ref(s, a, 0)),
				      s7m, NULL, NULL);
	S7_END_CREATOR(1);
}

static s7_pointer s7yeCreateArray(s7_scheme *s, s7_pointer a)
{
	Entity *ne = yeCreateArray (NULL, NULL);
	S7_END_CREATOR(0);
}

static s7_pointer s7ywPosCreate(s7_scheme *s, s7_pointer a)
{
	s7_int x = s7_integer(s7_list_ref(s, a, 0));
	s7_int y = s7_integer(s7_list_ref(s, a, 1));
	Entity *ne = ywPosCreate(x, y, NULL, NULL);

	S7_END_CREATOR(2);
}


S7_IMPLE_CREATOR(Int, s7_integer);
S7_IMPLE_CREATOR(String, s7_string);


#define S7ME s7_make_c_object
#define S7MI s7_make_integer

#define S7_IMPLEMENT_E_SES(func)					\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7ME(s, s7m->et,					\
			    func(					\
				    s7_string(s7_list_ref(s, a, 0)),	\
				    s7_c_object_value(s7_list_ref(s, a, 1)), \
				    s7_string(s7_list_ref(s, a, 2))	\
				    ));					\
	}

#define S7_IMPLEMENT_E_EIIE(func)					\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7ME(s, s7m->et,					\
			    func(					\
				    s7_c_object_value(s7_list_ref(s, a, 0)), \
				    s7_integer(s7_list_ref(s, a, 1)),	\
				    s7_integer(s7_list_ref(s, a, 2)),	\
				    s7_c_object_value(s7_list_ref(s, a, 3)) \
				    ));					\
	}

#define S7_IMPLEMENT_B_EES(func)					\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7MI(s,						\
			    func(					\
				    s7_c_object_value(s7_list_ref(s, a, 0)),	\
				    s7_c_object_value(s7_list_ref(s, a, 1)), \
				    s7_string(s7_list_ref(s, a, 2))	\
				    ));					\
	}

#define BIND_B_EES(f, a, b) S7_IMPLEMENT_B_EES(f)
#define BIND_E_EIIE(f, a, b) S7_IMPLEMENT_E_EIIE(f)
#include "binding.c"

#define S7_IMPLEMENT_I_E(func)						\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7MI(s,						\
			    func(s7_c_object_value(s7_car(a))));	\
	}

S7_IMPLEMENT_I_E(yeGetInt);

#define S7_DEF_F(name, nargs, optargs)					\
	s7_define_safe_function(s7, #name, s7##name, nargs, optargs, false, "")

static int init(void *sm, void *args)
{
	s7_scheme *s7;
	s7_int et;

	(void)args;
	s7 = s7_init();
	if (s7 == NULL)
		return -1;
	GET_S7(sm) = s7;
	s7m = sm;
	et =  s7_make_c_type(s7, "Entity");
	s7_c_type_set_free(s7, et, (void (*)(void *))yeDestroy);
	GET_ET(sm) = et;

	s7_define_safe_function(s7, "int_cast", int_cast, 1, 0, false, "");
	s7_define_safe_function(s7, "string_cast", string_cast, 1, 0, false, "");
	s7_define_safe_function(s7, "str_cast", string_cast, 1, 0, false, "");
	s7_define_safe_function(s7, "ptr_cast", ptr_cast, 1, 0, false, "");
	s7_define_variable(s7, "YEVE_ACTION",
			   s7_make_c_pointer(s7, (void *)ACTION));
	s7_define_variable(s7, "YEVE_NOTHANDLE",
			   s7_make_c_pointer(s7, (void *)NOTHANDLE));

	S7_DEF_F(yeGetInt, 1, 0);
	S7_DEF_F(ywidNewWidget, 2, 0);

	S7_DEF_F(yeCreateString, 1, 2);
	S7_DEF_F(yeCreateInt, 1, 2);
	S7_DEF_F(yeCreateFunction, 1, 2);
	S7_DEF_F(yeCreateArray, 0, 2);

#define BIND(f, nb_args, opt_args) S7_DEF_F(f, nb_args, opt_args)
#define IN_CALL 1
#undef BIND_NONE
#include "binding.c"
#undef IN_CALL

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

#define CPTR(idx) yeIsPtrAnEntity(args[idx]) ?				\
		s7_make_c_object(s, s7m->et, yeIncrRef(args[idx])) :	\
		s7_make_c_pointer(s, args[idx])


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
	if (s7_is_c_object(r))
		return s7_c_object_value(r);
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

