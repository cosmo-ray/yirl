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
#include "container.h"
#include "menu.h"
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

int loadString(void *s, const char *str);

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

static s7_pointer s7yeGetStringAt(s7_scheme *s, s7_pointer a)
{
	Entity *e = NULL;

	E_YE_GET(e);
	return s7_make_string(s, yeGetString(e));
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

static s7_pointer s7ywPosIsSame(s7_scheme *s, s7_pointer a)
{
	if (s7_is_integer(s7_list_ref(s, a, 1)))
		return s7_make_boolean(s, ywPosIsSame(E_AT(s, a, 0),
						      I_AT(s, a, 1),
						      I_AT(s, a, 2)));
	return s7_make_boolean(s, ywPosIsSame(E_AT(s, a, 0), E_AT(s, a, 1), 0));

}

static s7_pointer s7ywPosSet(s7_scheme *s, s7_pointer a)
{
	Entity *e;

	if (s7_is_integer(s7_list_ref(s, a, 1)))
		e = ywPosSet(E_AT(s, a, 0), I_AT(s, a, 1), I_AT(s, a, 2));
	else
		e = ywPosSetEnt(E_AT(s, a, 0), E_AT(s, a, 1), 0);
	return s7_make_c_object(s, s7m->et, e);
}

static s7_pointer s7yeCreateCopy(s7_scheme *s, s7_pointer a)
{
	Entity *ne = yeCreateCopy(E_AT(s, a, 0), NULL, NULL);
	S7_END_CREATOR(1);
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

static s7_pointer s7ywRectCreate(s7_scheme *s, s7_pointer a)
{
	s7_int x = s7_integer(s7_list_ref(s, a, 0));
	s7_int y = s7_integer(s7_list_ref(s, a, 1));
	s7_int w = s7_integer(s7_list_ref(s, a, 2));
	s7_int h = s7_integer(s7_list_ref(s, a, 3));
	Entity *ne = ywRectReCreateInts(x, y, w, h, NULL, NULL);

	S7_END_CREATOR(4);
}


static s7_pointer s7ywPosCreate(s7_scheme *s, s7_pointer a)
{
	s7_int x = s7_integer(s7_list_ref(s, a, 0));
	s7_int y = s7_integer(s7_list_ref(s, a, 1));
	Entity *ne = ywPosCreate(x, y, NULL, NULL);

	S7_END_CREATOR(2);
}

static s7_pointer s7ywSizeCreate(s7_scheme *s, s7_pointer a)
{
	return s7ywPosCreate(s, a);
}

static s7_pointer s7yeForeach(s7_scheme *s, s7_pointer a)
{
	Entity *array = E_AT(s, a, 0);
	s7_pointer func = s7_list_ref(s, a, 1);
	s7_pointer arg = s7_list_ref(s, a, 2);

	YE_FOREACH(array, el) {
		s7_pointer args = s7_list(s, 2, s7_make_c_object(s, s7m->et, el),
					  arg);
		arg = s7_call(s, func, args);
	}
	return arg;
}

static s7_pointer s7yeRevForeach(s7_scheme *s, s7_pointer a)
{
	Entity *array = E_AT(s, a, 0);
	s7_pointer func = s7_list_ref(s, a, 1);
	s7_pointer arg = s7_list_ref(s, a, 2);

	YE_REVFOREACH(array, el) {
		s7_pointer args = s7_list(s, 2, s7_make_c_object(s, s7m->et, el),
					  arg);
		arg = s7_call(s, func, args);
	}
	return arg;
}

S7_IMPLE_CREATOR(Int, s7_integer);
S7_IMPLE_CREATOR(String, s7_string);


static s7_pointer make_nothing(s7_scheme *s, ...)
{
	/* this should never be call, this is here so _Generic work */
	abort();
	return s7_nil(s);
}

#define S7A2(call, ...)					\
	_Generic(call,					\
		 default: make_nothing,			\
		 char *: s7_make_string,		\
		 const char *: s7_make_string,		\
		 int: s7_make_integer,			\
		 long: s7_make_integer,			\
		 long long: s7_make_integer,		\
		 _Bool: s7_make_boolean,		\
		 unsigned long: s7_make_integer,	\
		 unsigned int: s7_make_integer)		\
		(__VA_ARGS__, call)

#define S7T(call)				\
	_Generic(call,				\
		 default: 0,			\
		 char *: 2,			\
		 const char *: 2,		\
		 Entity *: 1,			\
		 const Entity *: 1,		\
		 int: 2,			\
		 _Bool: 2,			\
		 long: 2,			\
		 long long int: 2,			\
		 double: 2,			\
		 float: 2,			\
		 unsigned long: 2,		\
		 unsigned int: 2)

#define VOID_CALL(call)				\
	_Generic(call,				\
		 default: NULL,			\
		 char *: call,			\
		 const char *: call,		\
		 Entity *: call,		\
		 const Entity *: call,		\
		 int: call,			\
		 _Bool: call,			\
		 long long: call,			\
		 long: call,			\
		 double: call,			\
		 float: call,			\
		 unsigned long: call,		\
		 unsigned int: call)


#define S7ME s7_make_c_object
#define S7MI s7_make_integer
#define S7MB s7_make_boolean

#define BIND_AUTORET(call)						\
	int t = S7T(call);						\
	switch (t) {							\
	case 0:								\
		call;							\
		return s7_nil(s);					\
	case 1:								\
		return S7ME(s, s7m->et, (void *)VOID_CALL(call));	\
	case 2:								\
		return S7A2(VOID_CALL(call), s);			\
	}								\
	return s7_nil(s);

#define BIND_SES(f, ...)						\
	static s7_pointer						\
	s7##func(s7_scheme *s, s7_pointer a)				\
	{								\
		BIND_AUTORET(f(						\
			s7_string(s7_list_ref(s, a, 0)),		\
			s7_c_object_value(s7_list_ref(s, a, 1)),	\
			s7_string(s7_list_ref(s, a, 2))			\
			));						\
	}

#define BIND_EIIIIS(f, u0, u1)						\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a) {		\
		BIND_AUTORET(f(s7_c_object_value(s7_list_ref(s, a, 0)),	\
		       I_AT(s, a, 1),					\
		       I_AT(s, a, 2),					\
		       I_AT(s, a, 3),					\
		       I_AT(s, a, 4),					\
		       S_AT(s, a, 5)					\
				     ));				\
	}

#define BIND_EIIEE(f, u0, u1)						\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a) {		\
		BIND_AUTORET(f(s7_c_object_value(s7_list_ref(s, a, 0)), \
			       s7_integer(s7_list_ref(s, a, 1)),	\
			       s7_integer(s7_list_ref(s, a, 2)),	\
			       E_AT(s, a, 3),				\
			       E_AT(s, a, 4)				\
				     ));				\
	}

#define BIND_EEIII(f, ...)					    \
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	    \
	{							    \
		BIND_AUTORET(f(E_AT(s, a, 0), E_AT(s, a, 1),	    \
			       I_AT(s, a, 2), I_AT(s, a, 3),	    \
			       I_AT(s, a, 4)));			    \
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


#define BIND_I(f, useless0, usless1)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		BIND_AUTORET(f(s7_integer(s7_list_ref(s, a, 0))));	\
	}

#define BIND_V(f, ...)							\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		BIND_AUTORET(f());					\
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

#define BIND_ESE(f, ...)			\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(E_AT(s, a, 0), S_AT(s, a, 1),	\
			       E_AT(s, a, 2)));			\
	}


#define BIND_EES(f, ...)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(E_AT(s, a, 0), E_AT(s, a, 1),	\
			       S_AT(s, a, 2)));			\
	}

#define BIND_EIIS(f, ...)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(E_AT(s, a, 0), I_AT(s, a, 1),	\
			       I_AT(s, a, 2), S_AT(s, a, 3)));	\
	}

#define BIND_EIIE(f, ...)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(E_AT(s, a, 0), I_AT(s, a, 1),	\
			       I_AT(s, a, 2), E_AT(s, a, 3)));	\
	}

#define BIND_EEEE(f, ...)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(E_AT(s, a, 0), E_AT(s, a, 1),	\
			       E_AT(s, a, 2), E_AT(s, a, 3)));	\
	}

#define BIND_EEES(f, ...)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(E_AT(s, a, 0), E_AT(s, a, 1),	\
			       E_AT(s, a, 2), S_AT(s, a, 3)));	\
	}

#define BIND_SEES(f, ...)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(S_AT(s, a, 0), E_AT(s, a, 1),	\
			       E_AT(s, a, 2), S_AT(s, a, 3)));	\
	}

#define BIND_EES(f, ...)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(E_AT(s, a, 0), E_AT(s, a, 1),	\
			       S_AT(s, a, 2)));			\
	}

#define BIND_EEE(f, ...)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(E_AT(s, a, 0), E_AT(s, a, 1),	\
			       E_AT(s, a, 2)));			\
	}

#define BIND_EII(f, ...)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(E_AT(s, a, 0), I_AT(s, a, 1),	\
			       I_AT(s, a, 2)));			\
	}

#define BIND_EEI(f, ...)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(E_AT(s, a, 0), E_AT(s, a, 1),	\
			       I_AT(s, a, 2)));			\
	}

#define BIND_EE(f, ...)						\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(E_AT(s, a, 0), E_AT(s, a, 1)));	\
	}

#define BIND_ES(f, ...)						\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(E_AT(s, a, 0), S_AT(s, a, 1)));	\
	}

#define BIND_SI(f, ...)						\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(S_AT(s, a, 0), I_AT(s, a, 1)));	\
	}

#define BIND_ISS(f, ...)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(I_AT(s, a, 0), S_AT(s, a, 1),	\
			       S_AT(s, a, 2)));			\
	}

#define BIND_IIS(f, ...)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)	\
	{							\
		BIND_AUTORET(f(I_AT(s, a, 0), I_AT(s, a, 1),	\
			       S_AT(s, a, 2)));			\
	}

#define BIND_E(f, useless0, usless1)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		BIND_AUTORET(f(E_AT(s, a, 0)));				\
	}

#define BIND_S(f, useless0, usless1)					\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		BIND_AUTORET(f(s7_string(s7_car(a))));			\
	}

#define BIND_EI(f, ...)				\
	static s7_pointer s7##f(s7_scheme *s, s7_pointer a)		\
	{								\
		BIND_AUTORET(f(E_AT(s, a, 0), I_AT(s, a, 1)));		\
	}

static s7_pointer s7yesCall(s7_scheme *s, s7_pointer a)
{
	union ycall_arg args[17];
	int types[17];
	void *r;
	int i = 1;

	for (; i < 16; ++i) {
		args[i - 1].e = E_AT(s, a, i);
		types[i - 1] = YS_ENTITY;
		if (!args[i - 1].e) {
			break;
		}
	}

	r = yesCallInt(E_AT(s, a, 0), i, args, types);
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

BIND_ESE(yeReCreateArray, 2, 1);
BIND_EII(ywCanvasObjSetPos, 3, 0);
BIND_EIIE(ywCanvasNewText, 2, 2);
BIND_EEIII(ywMapInitEntity, 5, 0);

#include "binding.c"

static s7_pointer s7yeSetStringAt(s7_scheme *s, s7_pointer a)
{
	const char *val = s7_string(s7_list_ref(s, a, 2));

	yeSetString(s7_c_object_value(s7yeGet(s, a)), val);
	return s7_nil(s);
}

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

	s = yeToCStr(s7_c_object_value(s7_car(args)), 3, YE_FORMAT_PRETTY);
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

	s7_define_safe_function(s7, "yeForeach", s7yeForeach, 2, 1, false, "");
	s7_define_safe_function(s7, "yeRevForeach", s7yeRevForeach, 2, 1,
				false, "");
	s7_define_safe_function(s7, "yeIsChild", s7yeIsChild, 2, 0, false, "");
	s7_define_safe_function(s7, "int_cast", int_cast, 1, 0, false, "");
	s7_define_safe_function(s7, "string_cast", string_cast, 1, 0, false, "");
	s7_define_safe_function(s7, "str_cast", string_cast, 1, 0, false, "");
	s7_define_safe_function(s7, "ptr_cast", ptr_cast, 1, 0, false, "");
	s7_define_variable(s7, "YEVE_ACTION",
			   s7_make_c_pointer(s7, YEVE_ACTION));
	s7_define_variable(s7, "YEVE_NOTHANDLE",
			   s7_make_c_pointer(s7, YEVE_NOTHANDLE));


#define PUSH_I_GLOBAL_VAL(g, VAL) s7_define_variable(s7, #g, \
						     s7_make_integer(s7, VAL))

#define PUSH_I_GLOBAL(g) s7_define_variable(s7, #g, s7_make_integer(s7, g))

#define BIND(name, nargs, optargs)					\
	s7_define_safe_function(s7, #name, s7##name, nargs, optargs, false, "")

	BIND(yeGetStringAt, 2, 0);
	BIND(yeSetStringAt, 3, 0);
	BIND(ywRectCreate, 4, 2);
	BIND(ywCanvasObjSetPos, 3, 0);
	BIND(ywidNewWidget, 2, 0);
	BIND(yeCreateString, 1, 2);
	BIND(yeCreateInt, 1, 2);
	BIND(yeCreateFunction, 1, 2);
	BIND(yeCreateArray, 0, 2);
	BIND(yeCreateCopy, 1, 2);
	BIND(yesCall, 1, 15);
	BIND(ywCanvasNewText, 2, 2);
	BIND(ywMapInitEntity, 5, 0);
	BIND(ywPosIsSame, 2, 1);
	BIND(ywPosSet, 2, 1);
	BIND(yeReCreateArray, 2, 1);
	BIND(ywSizeCreate, 2, 2);

#define IN_CALL 1
#include "binding.c"
#undef IN_CALL

	/* ugly bindings here */
	loadString(sm,
		   "(define ywMapPushNbr"
		   "(lambda (map nb pos str)"
		   "(ywMapPushElem map (yeCreateInt nb) pos str)"
		   "))");
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

#define CPTR(idx) idx < nb ? (yeIsPtrAnEntity(args[idx].vptr) ?		\
			      s7_make_c_object(s, s7m->et, args[idx].vptr) : \
			      s7_make_c_pointer(s, args[idx].vptr)) : s7_nil(s)


static void *call(void *sm, const char *name, int nb, union ycall_arg *args,
		  int *t_array)
{
	s7_scheme *s = GET_S7(sm);
	s7_pointer f = s7_name_to_value(s, name);
	s7_pointer r;
	s7_pointer a;

	if (!f)
		return NULL;

	switch (nb) {
	case 1:
		a = s7_cons(s, CPTR(0), s7_nil(s));
		break;
	default:
		a = s7_list(s, nb, CPTR(0), CPTR(1), CPTR(2),
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

