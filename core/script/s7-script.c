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

#include <glib.h>
#include "s7-script.h"
#include "s7.h"
#include "game.h"
#include "canvas.h"
#include "widget.h"
#include "pos.h"
#include "entity-script.h"
#include "events.h"
#include "texture.h"

static int t = -1;

#define GET_OPS(sm) (((YScriptS7 *)sm)->ops)
#define GET_S7(sm) (((YScriptS7 *)sm)->s7)
#define GET_ET(sm) (((YScriptS7 *)sm)->et)
#define GET_GET(sm) (((YScriptS7 *)sm)->get)

typedef struct {
	YScriptOps ops;
	s7_scheme *s7;
	s7_int et;
	s7_int get;
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

#define E_AT(s, a, idx) (s7_list_ref(s, a, idx) != s7_nil(s) ?		\
			 (Entity *)s7_c_object_value(s7_list_ref(s, a, idx)) : \
			 NULL)

#define S_AT(s, a, idx) (s7_list_ref(s, a, idx) != s7_nil(s) ?	\
			 s7_string(s7_list_ref(s, a, idx)) :	\
			 NULL)

#define I_AT(s, a, idx) (s7_list_ref(s, a, idx) != s7_nil(s) ?	\
			 s7_integer(s7_list_ref(s, a, idx)) :	\
			 0)

static s7_pointer s7ywidNewWidget(s7_scheme *sc, s7_pointer a)
{
	return s7_make_c_pointer(sc, ywidNewWidget(s7_c_object_value(s7_car(a)),
						   S_AT(sc, a, 1)));
}

#define E_YE_GET(e)						\
	Entity *ar__ = s7_c_object_value(s7_car(a));		\
	s7_pointer k__ = s7_list_ref(s, a, 1);			\
	if (s7_is_string(k__))					\
		e = yeGet(ar__, s7_string(k__));		\
	else							\
		e = yeGet(ar__, s7_integer(k__));		\

static s7_pointer s7yeIncrAt(s7_scheme *s, s7_pointer a)
{
	Entity *e = NULL;

	E_YE_GET(e);
	yeAdd(e, 1);
	return s7_nil(s);
}

static s7_pointer s7yeAddAt(s7_scheme *s, s7_pointer a)
{
	Entity *e = NULL;

	E_YE_GET(e);
	yeAdd(e, I_AT(s, a, 2));
	return s7_nil(s);
}

static s7_pointer s7yeGet(s7_scheme *s, s7_pointer a)
{
	Entity *e = NULL;

	E_YE_GET(e);
	if (!e)
		return s7_nil(s);
	return s7_make_c_object(s, s7m->et, e);
}

static s7_pointer s7yeIsChild(s7_scheme *s, s7_pointer a)
{
	Entity *e = NULL;

	E_YE_GET(e);
	return s7_make_boolean(s, !!e);
}


static s7_pointer s7yeGetIntAt(s7_scheme *s, s7_pointer a)
{
	Entity *e = NULL;

	E_YE_GET(e);
	return s7_make_integer(s, yeGetInt(e));
}

#define S7_END_CREATOR(X)						\
	s7_pointer f = s7_list_ref(s, a, X);				\
	s7_pointer r = s7_make_c_object(s, s7m->get, ne);		\
	if (f != s7_nil(s))						\
		yePushBack(s7_c_object_value(f), ne,			\
			   s7_string(s7_list_ref(s, a, X + 1)));	\
	return r;							\


#define S7_IMPLE_CREATOR(t, tf)						\
	static s7_pointer s7yeCreate##t(s7_scheme *s, s7_pointer a)	\
	{								\
		Entity *ne = yeCreate##t(tf(s7_list_ref(s, a, 0)),	\
					 NULL, NULL);		\
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
#define S7MB s7_make_boolean

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


#define BIND_E_EIIEE(f, u0, u1)						\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a) {		\
		return S7ME(s, s7m->et,					\
			    f(s7_c_object_value(s7_list_ref(s, a, 0)), \
			      s7_integer(s7_list_ref(s, a, 1)),		\
			      s7_integer(s7_list_ref(s, a, 2)),		\
			      E_AT(s, a, 3),				\
			      E_AT(s, a, 4)				\
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
				    E_AT(s, a, 3)			\
				    ));					\
	}

#define S7_IMPLEMENT_B_EES(func)					\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7MB(s,						\
			    !!func(					\
				    s7_c_object_value(s7_list_ref(s, a, 0)), \
				    s7_c_object_value(s7_list_ref(s, a, 1)), \
				    s7_string(s7_list_ref(s, a, 2))	\
				    ));					\
	}

#define S7_IMPLEMENT_I_E(func)						\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7MI(s,						\
			    func(s7_c_object_value(s7_car(a))));	\
	}

#define BIND_I_EE(f, u0, u1)						\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7MI(s,						\
			    f(s7_c_object_value(s7_car(a)),		\
			      E_AT(s, a, 1)				\
				    ));					\
	}

#define S7_IMPLEMENT_V_E(func)						\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		func(s7_c_object_value(s7_car(a)));			\
		return s7_nil(s);					\
	}

#define BIND_E_EIIS(func, u0, u1)					\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7ME(s, s7m->et,					\
			    func(					\
				    s7_c_object_value(s7_list_ref(s, a, 0)), \
				    s7_integer(s7_list_ref(s, a, 1)),	\
				    s7_integer(s7_list_ref(s, a, 2)),	\
				    s7_string(s7_list_ref(s, a, 3))	\
				    ));					\
	}

#define BIND_E_SEES(func, u0, u1)					\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7ME(s, s7m->et,					\
			    func(					\
				    s7_string(s7_list_ref(s, a, 0)),	\
				    E_AT(s, a, 1),			\
				    E_AT(s, a, 2),			\
				    s7_string(s7_list_ref(s, a, 3))	\
				    ));					\
	}

#define BIND_B_EEE(f, u0, u1)						\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7MB(s,						\
			    !!f( E_AT(s, a, 0), E_AT(s, a, 1),		\
				 E_AT(s, a, 2)));			\
	}


#define BIND_B_EEEE(f, u0, u1)						\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7MB(s, !!f( E_AT(s, a, 0), E_AT(s, a, 1),	\
				   E_AT(s, a, 2), E_AT(s, a, 3)));	\
	}

#define BIND_V_EII(f, useless0, useless01)			\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		f(s7_c_object_value(s7_car(a)),			\
		  s7_integer(s7_list_ref(s, a, 1)),		\
		  s7_integer(s7_list_ref(s, a, 2)));		\
		return s7_nil(s);				\
	}

#define BIND_V_EI(f, useless0, useless01)			\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		f(s7_c_object_value(s7_car(a)),			\
		  s7_integer(s7_list_ref(s, a, 1)));		\
		return s7_nil(s);				\
	}


#define BIND_I_S(f, u0, u1)						\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7MI(s, f(s7_string(s7_car(a))));		\
	}

#define BIND_I_I(f, u0, u1)						\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7MI(s, f(s7_integer(s7_car(a))));		\
	}

#define BIND_V_EE(f, useless0, useless01)			\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		f(E_AT(s, a, 0), E_AT(s, a, 1));		\
		return s7_nil(s);				\
	}


#define BIND_B_EE(func, useless0, useless01)				\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7MB(s,						\
			    !!func(s7_c_object_value(s7_car(a)),	\
				   s7_c_object_value(s7_list_ref(s, a, 1)))); \
	}

#define BIND_V_I(func, u0, u1)						\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		func(s7_integer(s7_car(a)));				\
		return s7_nil(s);					\
	}

#define BIND_I_V(func)							\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		(void)a;						\
		return S7MI(s, func());					\
	}

#define BIND_V_vaE(f, useless0)						\
	static s7_pointer s7##func(s7_scheme *s, s7_pointer a)		\
	{								\
		(void)a;						\
		f(s7_c_object_value(s7_list_ref(s, a, 0)),		\
		  s7_c_object_value(s7_list_ref(s, a, 1)),		\
		  s7_c_object_value(s7_list_ref(s, a, 2)),		\
		  s7_c_object_value(s7_list_ref(s, a, 3)),		\
		  s7_c_object_value(s7_list_ref(s, a, 4)),		\
		  s7_c_object_value(s7_list_ref(s, a, 5)),		\
		  s7_c_object_value(s7_list_ref(s, a, 6)),		\
		  s7_c_object_value(s7_list_ref(s, a, 7)),		\
		  s7_c_object_value(s7_list_ref(s, a, 8)),		\
		  s7_c_object_value(s7_list_ref(s, a, 9)));		\
		return s7_nil(s);					\
	}

#define BIND_S_E(f, useless0, usless1)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		const char *str = f(s7_c_object_value(s7_car(a)));	\
		s7_pointer r = s7_make_string(s, str);			\
		return r;						\
	}


#define BIND_E_E(f, useless0, uesless1)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7ME(s, s7m->et,					\
			    f(s7_c_object_value(s7_list_ref(s, a, 0)))); \
	}

#define BIND_E_S(f, useless0, uesless1)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7ME(s, s7m->et,					\
			    f(s7_string(s7_list_ref(s, a, 0))));	\
	}

#define BIND_E_EE(f, useless0, uesless1)				\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7ME(s, s7m->et, f(E_AT(s, a, 0), E_AT(s, a, 1))); \
	}

#define BIND_E_EI(f, useless0, uesless1)				\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7ME(s, s7m->et, f(E_AT(s, a, 0), I_AT(s, a, 1))); \
	}

#define BIND_E_ES(f, useless0, uesless1)				\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		return S7ME(s, s7m->et, f(E_AT(s, a, 0), S_AT(s, a, 1))); \
	}


static s7_pointer s7yesCall(s7_scheme *s, s7_pointer a)
{
	void *args[17];
	void *r;

	for (int i = 0; i < 16; ++i) {
		args[i] = E_AT(s, a, i);
		if (!args[i]) {
			args[i] = Y_END_VA_LIST;
			break;
		}
		args[16] = Y_END_VA_LIST;
	}

	r = yesCallInt(args[0], args[1], args[2], args[3], args[4],
		       args[5], args[6], args[7], args[8], args[9],
		       args[10], args[11], args[12], args[13], args[14],
		       args[15]);
	if (yeIsPtrAnEntity(r))
		return s7_make_c_object(s, s7m->et, r);
	return s7_make_integer(s, (intptr_t) r);
}

static s7_pointer s7yevCreateGrp(s7_scheme *s, s7_pointer a)
{
	int nb_args = 1;
	Entity *r;

	if (s7_is_integer(s7_list_ref(s, a, 9)))
		nb_args = 10;
	else if (s7_is_integer(s7_list_ref(s, a, 8)))
		nb_args = 9;
	else if (s7_is_integer(s7_list_ref(s, a, 7)))
		nb_args = 8;
	else if (s7_is_integer(s7_list_ref(s, a, 6)))
		nb_args = 7;
	else if (s7_is_integer(s7_list_ref(s, a, 5)))
		nb_args = 6;
	else if (s7_is_integer(s7_list_ref(s, a, 4)))
		nb_args = 5;
	else if (s7_is_integer(s7_list_ref(s, a, 3)))
		nb_args = 4;
	else if (s7_is_integer(s7_list_ref(s, a, 2)))
		nb_args = 3;
	else if (s7_is_integer(s7_list_ref(s, a, 1)))
		nb_args = 2;

	r = yeCreateInts_(NULL,
		      nb_args,
		      s7_integer(s7_list_ref(s, a, 0)),
		      s7_integer(s7_list_ref(s, a, 1)),
		      s7_integer(s7_list_ref(s, a, 2)),
		      s7_integer(s7_list_ref(s, a, 3)),
		      s7_integer(s7_list_ref(s, a, 4)),
		      s7_integer(s7_list_ref(s, a, 5)),
		      s7_integer(s7_list_ref(s, a, 6)),
		      s7_integer(s7_list_ref(s, a, 7)),
		      s7_integer(s7_list_ref(s, a, 8)),
		      s7_integer(s7_list_ref(s, a, 9)),
		      s7_integer(s7_list_ref(s, a, 10)));
	return s7_make_c_object(s, s7m->et, r);
}

#define BIND_V_E(f, a, b) S7_IMPLEMENT_V_E(f)
#define BIND_I_E(f, a, b) S7_IMPLEMENT_I_E(f)
#define BIND_B_EES(f, a, b) S7_IMPLEMENT_B_EES(f)
#define BIND_E_EIIE(f, a, b) S7_IMPLEMENT_E_EIIE(f)
#include "binding.c"

static s7_pointer s7yeSetIntAt(s7_scheme *s, s7_pointer a)
{
	int val = s7_integer(s7_list_ref(s, a, 2));

	yeSetInt(s7_c_object_value(s7yeGet(s, a)), val);
	return s7_nil(s);
}

void destroyer(void *e)
{
	yeDestroy(e);
}

static s7_pointer stringifier(s7_scheme *s7, s7_pointer args)
{
	char *s;
	s7_pointer r;

	s = yeToCStr(s7_c_object_value(s7_car(args)), 2, 0);
	r = s7_make_string(s7, s);
	free(s);
	return r;
}

static int init(void *sm, void *args)
{
	s7_scheme *s7;
	s7_int et;
	s7_int get;

	(void)args;
	s7 = s7_init();
	if (s7 == NULL)
		return -1;
	s7_gc_on(s7, true);
	GET_S7(sm) = s7;
	s7m = sm;
	et = s7_make_c_type(s7, "Entity");
	s7_c_type_set_to_string(s7, et, stringifier);

	get = s7_make_c_type(s7, "Entity");
	s7_c_type_set_to_string(s7, get, stringifier);
	s7_c_type_set_free(s7, get, destroyer);
	GET_ET(sm) = et;
	GET_GET(sm) = get;

	s7_define_safe_function(s7, "yeIsChild", s7yeIsChild, 2, 0, false, "");
	s7_define_safe_function(s7, "int_cast", int_cast, 1, 0, false, "");
	s7_define_safe_function(s7, "string_cast", string_cast, 1, 0, false, "");
	s7_define_safe_function(s7, "str_cast", string_cast, 1, 0, false, "");
	s7_define_safe_function(s7, "ptr_cast", ptr_cast, 1, 0, false, "");
	s7_define_variable(s7, "YEVE_ACTION",
			   s7_make_c_pointer(s7, YEVE_ACTION));
	s7_define_variable(s7, "YEVE_NOTHANDLE",
			   s7_make_c_pointer(s7, YEVE_NOTHANDLE));


#define PUSH_I_GLOBAL(g) s7_define_variable(s7, #g, s7_make_integer(s7, g))
#define BIND(name, nargs, optargs)					\
	s7_define_safe_function(s7, #name, s7##name, nargs, optargs, false, "")

	BIND(ywidNewWidget, 2, 0);
	BIND(yeCreateString, 1, 2);
	BIND(yeCreateInt, 1, 2);
	BIND(yeCreateFunction, 1, 2);
	BIND(yeCreateArray, 0, 2);
#define IN_CALL 1
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
		s7_make_c_object(s, s7m->et, args[idx]) :		\
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

	switch (nbArg) {
	case 1:
		a = s7_cons(s, CPTR(0), s7_nil(s));
		break;
	default:
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
