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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include <glib.h>

#include <src/duktape.h>
#include <extras/print-alert/duk_print_alert.h>
#include <extras/console/duk_console.h>

#include "game.h"
#include "canvas.h"
#include "keydef.h"
#include "widget.h"
#include "texture.h"
#include "pos.h"
#include "entity-script.h"
#include "events.h"

static int t = -1;

#define OPS(s) ((YScriptDuk *)s)->ops
#define CTX(s) ((YScriptDuk *)s)->ctx

static duk_ret_t dukyeDestroy(duk_context *ctx);

static inline int is_ent(duk_context *ctx, int i)
{
	return duk_is_object(ctx, i);
}

static inline Entity *GET_E(duk_context *ctx, int i)
{
	Entity *r;

	if (!duk_is_object(ctx, i))
		return NULL;
	duk_get_prop_index(ctx, i, 0);
 	r = duk_get_pointer(ctx, -1);
 	duk_pop(ctx);
	return r;
}

static inline void PUSH_E(duk_context *c, Entity *e)
{
	if (unlikely(!e)) {
		duk_push_null(c);
		return;
	}
	/* [stack...] */
	duk_push_object(c);
	/* [stack...] [obj] */
	duk_push_pointer(c, e);
	/* [stack...] [obj] [ptr] */
	duk_put_prop_index(c, -2, 0);
}

static inline void mk_ent(duk_context *c, Entity *e, int add_destroy)
{
	PUSH_E(c, e);
	if (!add_destroy || unlikely(!e))
		return;
	/* [stack...] [obj] */
	duk_push_c_function(c, dukyeDestroy, 1);
	/* [stack...] [obj] [c func] */
	duk_set_finalizer(c, -1);
	/* [stack...] [obj] */
}

typedef struct {
	YScriptOps ops;
	duk_context *ctx;
} YScriptDuk;

YScriptDuk *cur_sc;

static void e_destroy(void *s, Entity *e)
{
	if (yeIData(e)) {
		duk_context *ctx = CTX(s);

		duk_push_heap_stash(ctx);
		/* [stack...]  [stash] */
		duk_del_prop_string(ctx, -1, yeGetFunction(e));
		duk_pop(ctx);
	}
}

static duk_ret_t dukyjsFunc(duk_context *ctx)
{
	Entity *e = GET_E(ctx, 0);
	const char *fname;

	if (yeIData(e)) {
		/* [stack...] */
		duk_push_heap_stash(ctx);
		/* [stack...]  [stash] */
		duk_get_prop_string(ctx, -1, yeGetFunction(e));
		/* [stack...]  [stash] [function/undefine ?] */
		duk_replace(ctx, -2);
		/* [stack] [function/undefine ?] */

	} else {
		fname = yeGetString(GET_E(ctx, 0));
		duk_push_string(ctx, fname);
	}
	return 1;
}

static duk_ret_t dukyjsCall(duk_context *ctx)
{
	int i = duk_get_top(ctx) - 1;

	if (is_ent(ctx, 0)) {
		/* [entity func] [argv...] */
		dukyjsFunc(ctx);
		/* [entity func] [argv...] [func] */
		duk_replace(ctx, 0);
		/* [func] [argv...] */
	}
	duk_call(ctx, i);
	return duk_get_top(ctx);
}

static void *get_arg(duk_context *ctx, int i)
{
	if (is_ent(ctx, i))
		return GET_E(ctx, i);
	if (duk_is_string(ctx, i))
		return (void *)duk_get_string(ctx, i);
	if (duk_is_number(ctx, i))
		return (void *)duk_get_int(ctx, i);
	return duk_get_pointer(ctx, i);
}

static duk_ret_t dukyesCall(duk_context *ctx)
{
	Entity *e = GET_E(ctx, 0);

	switch (duk_get_top(ctx)) {
	case 1:
		PUSH_E(ctx, yesCall(e));
		return 1;
	case 2:
		PUSH_E(ctx, yesCall(e, get_arg(ctx, 1)));
		return 1;
	case 3:
		PUSH_E(ctx, yesCall(e, get_arg(ctx, 1), get_arg(ctx, 2)));
		return 1;
	case 4:
		PUSH_E(ctx, yesCall(e, get_arg(ctx, 1),
				    get_arg(ctx, 2), get_arg(ctx, 3)));
		return 1;
	case 5:
		PUSH_E(ctx, yesCall(e, get_arg(ctx, 1), get_arg(ctx, 2),
				    get_arg(ctx, 3), get_arg(ctx, 4)));
		return 1;
	default:
		DPRINT_ERR("internal error: too much argument");
		return -1;
	}

	return -1;
}

static duk_ret_t dukyeDestroy(duk_context *ctx)
{
	Entity *e;

	if (!is_ent(ctx, 0))
		return 0;
	duk_get_prop_index(ctx, 0, 0);
 	e = duk_get_pointer(ctx, 1);
	duk_push_null(ctx);
	duk_put_prop_index(ctx, 0, 0);
	yeDestroy(e);
 	duk_pop(ctx);
	return 0;
}

static duk_ret_t dukto_str(duk_context *ctx)
{
	if (duk_is_pointer(ctx, 0)) {
		void *p = duk_get_pointer(ctx, 0);
		duk_push_string(ctx, (char *)p);
	} else {
		duk_push_null(ctx);
	}
	return 1;
}

static duk_ret_t dukygGetManager(duk_context *ctx)
{
	duk_push_pointer(ctx, ygGetManager(duk_get_string(ctx, 0)));
	return 1;
}

static duk_ret_t dukygLoadScript(duk_context *ctx)
{
	duk_push_int(ctx,
		     ygLoadScript(GET_E(ctx, 0),
				  duk_get_pointer(ctx, 1),
				  duk_get_string(ctx, 2)
		       ));
	return 1;
}

static duk_ret_t dukywidNewWidget(duk_context *ctx)
{
	duk_push_pointer(ctx, ywidNewWidget(GET_E(ctx, 0),
					    duk_get_string(ctx, 1)));
	return 1;
}

static duk_ret_t dukyeSetIntAt(duk_context *ctx)
{
	yeSetIntAt(GET_E(ctx, 0), duk_get_int(ctx, 1),
		   duk_get_int(ctx, 2));
	return 0;
}

static duk_ret_t dukyeGet(duk_context *ctx)
{
	if (duk_is_string(ctx, 1))
		PUSH_E(ctx, yeGet(GET_E(ctx, 0), duk_get_string(ctx, 1)));
	else
		PUSH_E(ctx, yeGet(GET_E(ctx, 0), duk_get_int(ctx, 1)));
	return 1;
}

static duk_ret_t dukyeCreateString(duk_context *ctx)
{
	mk_ent(ctx, yeCreateString(duk_get_string(ctx, 0),
				   GET_E(ctx, 1),
				   duk_get_string(ctx, 2)),
	       !GET_E(ctx, 1));
	return 1;
}

static duk_ret_t dukywRectCreateInts(duk_context *ctx)
{
	mk_ent(ctx, ywRectCreateInts(duk_get_int(ctx, 0), duk_get_int(ctx, 1),
				     duk_get_int(ctx, 2), duk_get_int(ctx, 3),
				     GET_E(ctx, 4), duk_get_string(ctx, 5)),
	       !GET_E(ctx, 4));
	return 1;
}

static duk_ret_t dukywPosCreate(duk_context *ctx)
{
	mk_ent(ctx, ywPosCreate(duk_get_int(ctx, 0), duk_get_int(ctx, 1),
				GET_E(ctx, 2), duk_get_string(ctx, 3)),
	       !GET_E(ctx, 2));
	return 1;
}

static duk_ret_t dukyeCreateInt(duk_context *ctx)
{
	mk_ent(ctx, yeCreateInt(duk_get_int(ctx, 0),
				GET_E(ctx, 1),
				duk_get_string(ctx, 2)), !GET_E(ctx, 1));
	return 1;
}

static duk_ret_t dukyeCreateFunction(duk_context *ctx)
{
	static int glob_cnt;
	const char *str = NULL;
	if (duk_is_function(ctx, 0)) {
		Entity *e;
		char *rname = NULL;

		asprintf(&rname, "_r%d", glob_cnt++);
		/* [stack...] */
		duk_push_heap_stash(ctx);
		/* [stack...]  [stash] */
		duk_dup(ctx, 0);
		/* [stack...]  [stash] [function] */
		duk_put_prop_string(ctx, -2, rname);
		/* [stack...] [stash] */
		e = yeCreateFunction(rname, ygGetManager("js"),
				     GET_E(ctx, 1), duk_get_string(ctx, 2));
		YE_TO_FUNC(e)->idata = 0x1;
		duk_pop(ctx);
		/* [stack...] */
		free(rname);
		mk_ent(ctx, e, !GET_E(ctx, 1));
		return 1;
	}
	str = duk_get_string(ctx, 0);
	mk_ent(ctx, yeCreateFunction(str,
				     ygGetManager("js"),
				     GET_E(ctx, 1),
				     duk_get_string(ctx, 2)),
	       !GET_E(ctx, 1));
	return 1;
}

static duk_ret_t dukyeCreateArray(duk_context *ctx)
{
	mk_ent(ctx, yeCreateArray(GET_E(ctx, 0),
				  duk_get_string(ctx, 1)),
	       !GET_E(ctx, 0));
	return 1;
}

static duk_ret_t dukyeReCreateArray(duk_context *ctx)
{
	mk_ent(ctx, yeReCreateArray(GET_E(ctx, 0),
				    duk_get_string(ctx, 1),
				    GET_E(ctx, 2)),
		!GET_E(ctx, 0));
	return 1;
}

#define BIND_E_EIIEE(f, u0, u1)						\
	static duk_ret_t duk##f(duk_context *ctx) {			\
		PUSH_E(ctx, f(GET_E(ctx, 0),				\
			      duk_get_int(ctx, 1),			\
			      duk_get_int(ctx, 2),			\
			      GET_E(ctx, 3),				\
			      GET_E(ctx, 4)));				\
		return 1;						\
	}

#define BIND_E_EIIE(f, u0, u1)						\
	static duk_ret_t duk##f(duk_context *ctx) {			\
		PUSH_E(ctx, f(GET_E(ctx, 0),				\
			      duk_get_int(ctx, 1),			\
			      duk_get_int(ctx, 2),			\
			      GET_E(ctx, 3)));				\
		return 1;						\
	}


#define BIND_E_EIIS(f, u0, u1)			\
	static duk_ret_t duk##f(duk_context *ctx) {			\
		PUSH_E(ctx, f(GET_E(ctx, 0),				\
			      duk_get_int(ctx, 1),			\
			      duk_get_int(ctx, 2),			\
			      duk_get_string(ctx, 3)));			\
		return 1;						\
	}

#define BIND_E_EIII(f, u0, u1)						\
	static duk_ret_t duk##f(duk_context *ctx) {			\
		PUSH_E(ctx, f(GET_E(ctx, 0),				\
			      duk_get_int(ctx, 1),			\
			      duk_get_int(ctx, 2),			\
			      duk_get_int(ctx, 3)));			\
		return 1;						\
	}

#define BIND_E_SEES(f, u0, u1)						\
	static duk_ret_t duk##f(duk_context *ctx) {			\
		PUSH_E(ctx, f(						\
			       duk_get_string(ctx, 0),			\
			       GET_E(ctx, 1),				\
			       GET_E(ctx, 2),				\
			       duk_get_string(ctx, 3)			\
			       ));					\
		return 1;						\
	}

#define BIND_E_S(x, u0, u1)						\
	static duk_ret_t duk##x(duk_context *ctx) {			\
		PUSH_E(ctx, x(duk_get_string(ctx, 0)));			\
		return 1;						\
	}

#define BIND_V_V(x)						\
	static duk_ret_t duk##x(duk_context *ctx) {		\
		x();						\
		return 0;					\
	}

#define BIND_V_I(x, u0, u1)					\
	static duk_ret_t duk##x(duk_context *ctx) {		\
		x(duk_get_int(ctx, 0));				\
		return 0;					\
	}

#define BIND_I_V(x)						\
	static duk_ret_t duk##x(duk_context *ctx) {		\
		duk_push_int(ctx, x());				\
		return 1;					\
	}

#define BIND_V_E(x, u0, u1)			\
	static duk_ret_t duk##x(duk_context *ctx) {		\
		x(GET_E(ctx, 0));				\
		return 0;					\
	}

#define BIND_V_EE(x, u0, u1)					\
	static duk_ret_t duk##x(duk_context *ctx) {		\
		x(GET_E(ctx, 0), GET_E(ctx, 1));		\
		return 0;					\
	}

#define BIND_V_EI(x, u0, u1)					\
	static duk_ret_t duk##x(duk_context *ctx) {		\
		x(GET_E(ctx, 0), duk_get_int(ctx, 1));		\
		return 0;					\
	}

#define BIND_V_EII(x, u0, u1)						\
	static duk_ret_t duk##x(duk_context *ctx) {			\
		x(GET_E(ctx, 0), duk_get_int(ctx, 1), duk_get_int(ctx, 2)); \
		return 0;						\
	}

#define BIND_I_E(x, u0, u1)					\
	static duk_ret_t duk##x(duk_context *ctx) {		\
		duk_push_int(ctx, x(GET_E(ctx, 0)));		\
		return 1;					\
	}

#define BIND_I_EE(x, u0, u1)						\
	static duk_ret_t duk##x(duk_context *ctx) {			\
		duk_push_int(ctx, x(GET_E(ctx, 0), GET_E(ctx, 1)));	\
		return 1;						\
	}

#define BIND_I_I(x, u0, u1)					\
	static duk_ret_t duk##x(duk_context *ctx) {		\
		duk_push_int(ctx, x(duk_get_int(ctx, 0)));	\
		return 1;					\
	}

#define BIND_I_S(x, u0, u1)					\
	static duk_ret_t duk##x(duk_context *ctx) {		\
		duk_push_int(ctx, x(duk_get_string(ctx, 0)));	\
		return 1;					\
	}

#define BIND_S_E(x, u0, u1)					\
	static duk_ret_t duk##x(duk_context *ctx) {		\
		duk_push_string(ctx, x(GET_E(ctx, 0)));		\
		return 1;					\
	}

#define BIND_E_E(x, u0, u1)			\
	static duk_ret_t duk##x(duk_context *ctx) {	\
		PUSH_E(ctx, x(GET_E(ctx, 0)));		\
		return 1;				\
	}

#define DUMB_FUNC(x)						\
	static duk_ret_t duk##x(duk_context *ctx) {		\
		printf("BIND not yet implemented %s\n", #x);	\
		return 0;					\
	}


DUMB_FUNC(yeGetIntAt);
DUMB_FUNC(yevCreateGrp);
DUMB_FUNC(yeIncrAt);
DUMB_FUNC(yeAddAt);

#define BIND_E_EE(a, b, c) DUMB_FUNC(a)
#define BIND_E_ES(a, b, c) DUMB_FUNC(a)
#define BIND_E_EI(a, b, c) DUMB_FUNC(a)
#define BIND_B_EE(a, b, c) DUMB_FUNC(a)
#define BIND_I_EES(a, b, c) DUMB_FUNC(a)
#define BIND_E_EES(a, b, c) DUMB_FUNC(a)
#define BIND_B_EES(a, b, c) DUMB_FUNC(a)
#define BIND_B_EEE(a, b, c) DUMB_FUNC(a)
#define BIND_B_EEEE(a, b, c) DUMB_FUNC(a)
#include "binding.c"

static void *call(void *sm, const char *name, va_list ap)
{
	duk_context *ctx = CTX(sm);
	int i = 0;
	void *r;

	duk_get_global_string(ctx, name);
	for (void *tmp = va_arg(ap, void *); tmp != Y_END_VA_LIST;
	     tmp = va_arg(ap, void *)) {
		if (yeIsPtrAnEntity(tmp))
			PUSH_E(ctx, tmp);
		else if ((intptr_t)tmp < 65536)
			duk_push_int(ctx, (intptr_t)tmp);
		else
			duk_push_pointer(ctx, tmp);
		++i;
	}
	duk_call(ctx, i);
	i = duk_get_top(ctx) - 1;
	if (is_ent(ctx, i))
		r = GET_E(ctx, i);
	else if (duk_is_pointer(ctx, i))
		r = duk_get_pointer(ctx, i);
	else if (duk_is_boolean(ctx, i))
		r = (void *)duk_get_boolean(ctx, i);
	else
		r = (void *)duk_get_int(ctx, i);
	duk_pop(ctx);
	return r;
}

static int destroy(void *sm)
{
	duk_destroy_heap(CTX(sm));
	free(sm);
	return 0;
}

static void fatal_handler(void *udata, const char *msg)
{
	DPRINT_ERR("BAZOKA: %p - %s\n",
		   udata, msg ? msg : "(nil)");
	abort();
}

static int init(void *sm, void *args)
{
	duk_context *ctx = duk_create_heap(NULL, NULL, NULL, NULL, fatal_handler);

	duk_print_alert_init(ctx, 0);
	duk_console_init(ctx, 0);
	CTX(sm) = ctx;
	duk_push_global_object(ctx);

#define BIND(x, args, oargs)						\
	duk_push_string(ctx, #x);					\
	duk_push_c_function(ctx,					\
			    duk##x, args + oargs ?			\
			    args + oargs : DUK_VARARGS);		\
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE |			\
		     DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

#define PUSH_I_GLOBAL_VAL(x, val)					\
	duk_push_string(ctx, #x);					\
	duk_push_int(ctx, val);						\
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE |			\
		     DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

#define PUSH_I_GLOBAL(x)						\
	duk_push_string(ctx, #x);					\
	duk_push_int(ctx, x);						\
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE |			\
		     DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);


	PUSH_I_GLOBAL_VAL(is_yirl, 1);
	PUSH_I_GLOBAL_VAL(YEVE_NOTHANDLE, NOTHANDLE);
	PUSH_I_GLOBAL_VAL(YEVE_ACTION, ACTION);
	BIND(ywRectCreateInts, 4, 2);
	BIND(to_str, 1, 0);
	BIND(yeCreateString, 1, 2);
	BIND(yeCreateInt, 1, 2);
	BIND(yeCreateArray, 0, 2);
	BIND(yeReCreateArray, 2, 1);
	BIND(yeCreateFunction, 1, 2);
	BIND(ywidNewWidget, 1, 2);
	BIND(ygLoadScript, 3, 0);
	BIND(ygGetManager, 1, 0);
	BIND(yjsFunc, 0, 0);
	BIND(yjsCall, 0, 0);
	BIND(yesCall, 0, 0);
	BIND(yeDestroy, 1, 0);
#define IN_CALL 1
#include "binding.c"
#undef IN_CALL
	duk_pop(ctx);
	duk_eval_string(ctx,
			"function yeGetIntAt(e, at) "
			"{ return yeGetInt(yeGet(e, at)) }"

			"function yeGetStringAt(e, at) "
			"{ return yeGetString(yeGet(e, at)) }"

			"function yeTryCreateArray(e, at) {"
			"var r = yeGet(e, at);"
			"if (r) return r;"
			"return yeCreateArray(e, at);"
			"}"

			"function yeTryCreateInt(i, e, at) {"
			"var r = yeGet(e, at);"
			"if (r) return r;"
			"return yeCreateInt(i, e, at);"
			"}"

			"function ywSizeCreate(w, h, f, n)"
			"{return ywPosCreate(w, h, f, n);}"
		);
	return 0;
}

static int loadString(void *s, const char *str)
{
	duk_eval_string(CTX(s), str);
	return 0;
}

static int loadFile(void *s, const char *file)
{
	int fd = open(file, O_RDONLY);
	yeAutoFree Entity *fstr = yeCreateString(NULL, NULL, NULL);
	struct stat st;

	if (stat(file, &st) < 0 || fd < 0) {
		DPRINT_ERR("cannot open/stat '%s'", file);
		return -1;
	}
	yeAddStrFromFd(fstr, fd, st.st_size);
	loadString(s, yeGetString(fstr));
	close(fd);
	return 0;
}

static void *allocator(void)
{
	YScriptDuk *ret;

	ret = g_new0(YScriptDuk, 1);
	ret->ops.init = init;
	ret->ops.e_destroy = e_destroy;
	ret->ops.destroy = destroy;
	ret->ops.loadFile = loadFile;
	ret->ops.loadString = loadString;
	ret->ops.call = call;
	return (void *)ret;
}

int ysDukInit(void)
{
	t = ysRegister(allocator);
	return t;
}

int ysDukEnd(void)
{
	return ysUnregiste(t);
}

int ysDukGetType(void)
{
	return t;
}
