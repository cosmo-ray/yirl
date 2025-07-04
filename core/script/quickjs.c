/*
**Copyright (C) 2020-2023 Matthias Gatto
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
#include <string.h>

#include <quickjs.h>
#include <quickjs-libc.h>

#include "quickjs-script.h"
#include "game.h"
#include "canvas.h"
#include "container.h"
#include "keydef.h"
#include "menu.h"
#include "widget.h"
#include "script.h"
#include "texture.h"
#include "pos.h"
#include "entity-script.h"
#include "entity-array.h"
#include "events.h"
#include "condition.h"
#include "event-base-objs.h"

static int t = -1;

#define OPS(s) ((YScriptQjs *)s)->ops
#define RUNTIME(s) ((YScriptQjs *)s)->runtime
#define CTX(s) ((YScriptQjs *)s)->ctx


static JSClassID widget_class_id;
static JSClassID entity_class_id;
static JSClassID it_class_id;
static JSClassID freeable_entity_class_id;
static JSClassID script_manager_class_id;


static void *cur_manager;

static inline Entity *GET_E_(JSValueConst v)
{
	Entity *e = JS_GetOpaque(v, entity_class_id);
	if (e)
		return e;
	return JS_GetOpaque(v, freeable_entity_class_id);
}

#define GET_E(ctx, idx)				\
	(idx < argc ? GET_E_(argv[idx]) : NULL)

#define GET_S(ctx, idx)							\
	(idx < argc ? JS_ToCString(ctx, argv[idx]) : NULL)


#define GET_I(ctx, idx)				\
	GET_I_(ctx, idx, argc, argv)

static inline int GET_I_(JSContext *ctx, int idx,
			 int argc, JSValueConst *argv)
{
	int64_t res;

	if (idx >= argc)
		return 0;
	JS_ToInt64(ctx, &res, argv[idx]);
	return res;
}

#define GET_D(ctx, idx)				\
	GET_D_(ctx, idx, argc, argv)

static JSValue call_exept(JSContext *ctx, JSValueConst func_obj, JSValueConst this_obj,
		  int argc, JSValueConst *argv) {
	JSValue r = JS_Call(ctx, func_obj, this_obj, argc, argv);
	if (JS_IsException(r)) {
		js_std_dump_error(ctx);
	}
	return r;
}

static inline double GET_D_(JSContext *ctx, int idx,
			 int argc, JSValueConst *argv)
{
	double res;

	if (idx >= argc)
		return 0;
	JS_ToFloat64(ctx, &res, argv[idx]);
	return res;
}

#define LUAT(call)				\
	_Generic(call,				\
		 default: 0,			\
		 char *: 1,			\
		 const char *: 1,		\
		 Entity *: 1,			\
		 const Entity *: 1,		\
		 int: 1,			\
		 long: 1,			\
		 long long: 1,			\
		 unsigned long long: 1,		\
		 _Bool: 1,			\
		 double: 1,			\
		 float: 1,			\
		 unsigned long: 1,		\
		 unsigned int: 1)

#define VOID_CALL(call)				\
	_Generic(call,				\
		 default: NULL,			\
		 char *: call,			\
		 const char *: call,		\
		 Entity *: call,		\
		 const Entity *: call,		\
		 _Bool : call,			\
		 int: call,			\
		 long: call,			\
		 long long: call,		\
		 unsigned long long: call,	\
		 double: call,			\
		 float: call,			\
		 unsigned long: call,		\
		 unsigned int: call)

#define BIND_AUTORET(call)						\
	int t = LUAT(call);						\
	switch (t) {							\
	case 0:								\
		_Pragma("GCC diagnostic ignored \"-Wunused-value\"");	\
		call;							\
		_Pragma("GCC diagnostic pop");				\
		return JS_NULL;						\
	case 1:								\
		return AUTOPUSH(VOID_CALL(call), ctx);			\
	}								\
	return JS_NULL;

static JSValue make_abort(JSContext *ctx, ...)
{
	abort();
	return JS_NULL;
}

#define AUTOPUSH(call, ...)				\
	_Generic(call,					\
		 default: make_abort,			\
		 char *: new_str,			\
		 const char *: new_str,			\
		 char: JS_NewInt32,			\
		 short: JS_NewInt32,			\
		 int: JS_NewInt32,			\
		 long: JS_NewInt64,			\
		 long long: JS_NewInt64,		\
		 float: JS_NewFloat64,			\
		 double: JS_NewFloat64,			\
		 _Bool: JS_NewBool,			\
		 unsigned char: JS_NewInt32,		\
		 unsigned short: JS_NewInt32,		\
		 unsigned long: JS_NewInt64,		\
		 unsigned long long: JS_NewInt64,	\
		 Entity *: new_ent,			\
		 const Entity *: new_ent,		\
		 unsigned int: JS_NewInt32)		\
	(__VA_ARGS__, call)


#define BIND_V(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f());					\
	}

#define BIND_E(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0)));				\
	}

#define BIND_S(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_S(ctx, 0)));				\
	}

#define BIND_I(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_I(ctx, 0)));				\
	}

#define BIND_EE(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1)));				\
	}

#define BIND_EI(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1)));				\
	}

#define BIND_ED(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_D(ctx, 1)));				\
	}

#define BIND_SI(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_S(ctx, 0),				\
			       GET_I(ctx, 1)));				\
	}

#define BIND_ISS(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_I(ctx, 0),				\
			       GET_S(ctx, 1),GET_S(ctx, 2)));		\
	}

#define BIND_IES(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_I(ctx, 0),				\
			       GET_E(ctx, 1),GET_S(ctx, 2)));		\
	}

#define BIND_IESI(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_I(ctx, 0),				\
			       GET_E(ctx, 1),GET_S(ctx, 2),		\
			       GET_I(ctx, 3)));				\
	}

#define BIND_DESI(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_D(ctx, 0),				\
			       GET_E(ctx, 1),GET_S(ctx, 2),		\
			       GET_I(ctx, 3)));				\
	}

#define BIND_IIS(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_I(ctx, 0),				\
			       GET_I(ctx, 1),GET_S(ctx, 2)));		\
	}

#define BIND_III(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_I(ctx, 0),				\
			       GET_I(ctx, 1),GET_I(ctx, 2)));		\
	}

#define BIND_IIE(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_I(ctx, 0),				\
			       GET_I(ctx, 1),GET_E(ctx, 2)));		\
	}

#define BIND_II(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_I(ctx, 0), GET_I(ctx, 1)));		\
	}

#define BIND_SS(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_S(ctx, 0), GET_S(ctx, 1)));		\
	}

#define BIND_ES(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_S(ctx, 1)));				\
	}

#define BIND_EID(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_D(ctx, 2)));		\
	}

#define BIND_EII(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2)));		\
	}

#define BIND_ESS(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_S(ctx, 1), GET_S(ctx, 2)));		\
	}

#define BIND_EEI(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_I(ctx, 2)));		\
	}

#define BIND_ESI(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_S(ctx, 1), GET_I(ctx, 2)));		\
	}

#define BIND_ESE(f, ...)			\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_S(ctx, 1), GET_E(ctx, 2)));		\
	}


#define BIND_EES(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_S(ctx, 2)));		\
	}

#define BIND_SES(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_S(ctx, 0),				\
			       GET_E(ctx, 1), GET_S(ctx, 2)));		\
	}

#define BIND_SE(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_S(ctx, 0), GET_E(ctx, 1)));		\
	}

#define BIND_EEE(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_E(ctx, 2)));		\
	}

#define BIND_EEEE(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_E(ctx, 2),		\
			       GET_E(ctx, 3)));				\
	}

#define BIND_EEEI(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_E(ctx, 2),		\
			       GET_I(ctx, 3)));				\
	}

#define BIND_EEEEI(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_E(ctx, 2),		\
			       GET_E(ctx, 3), GET_I(ctx, 4)));		\
	}

#define BIND_EEEEE(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_E(ctx, 2),		\
			       GET_E(ctx, 3), GET_E(ctx, 4)));		\
	}

#define BIND_EEESI(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_E(ctx, 2),		\
			       GET_S(ctx, 3), GET_I(ctx, 4)));		\
	}

#define BIND_EESI(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0), GET_E(ctx, 1),		\
			       GET_S(ctx, 2), GET_I(ctx, 3)));		\
	}

#define BIND_EESS(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0), GET_E(ctx, 1),		\
			       GET_S(ctx, 2), GET_S(ctx, 3)));		\
	}

#define BIND_EEES(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_E(ctx, 2),		\
			       GET_S(ctx, 3)));				\
	}

#define BIND_EEIS(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_I(ctx, 2),		\
			       GET_S(ctx, 3)));				\
	}

#define BIND_EIES(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_E(ctx, 2),		\
			       GET_S(ctx, 3)));				\
	}

#define BIND_SEES(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_S(ctx, 0),				\
			       GET_E(ctx, 1), GET_E(ctx, 2),		\
			       GET_S(ctx, 3)));				\
	}

#define BIND_IIES(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_I(ctx, 0),				\
			       GET_I(ctx, 1),GET_E(ctx, 2),		\
			       GET_S(ctx, 3)));				\
	}

#define BIND_EIIE(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_E(ctx, 3)));				\
	}

#define BIND_EIII(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_I(ctx, 3)));				\
	}

#define BIND_EIIII(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_I(ctx, 3), GET_I(ctx, 4)));		\
	}

#define BIND_EIIS(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_S(ctx, 3)));				\
	}

#define BIND_EIIEE(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_E(ctx, 3), GET_E(ctx, 4)));		\
	}

#define BIND_EIIES(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_E(ctx, 3), GET_S(ctx, 4)));		\
	}

#define BIND_EIIIS(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_I(ctx, 3),				\
			       GET_S(ctx, 4)));				\
	}

#define BIND_EIIIIS(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_I(ctx, 3), GET_I(ctx, 4),		\
			       GET_S(ctx, 5)));				\
	}

#define BIND_EIIISI(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_I(ctx, 3), GET_S(ctx, 4),		\
			       GET_I(ctx, 5)));				\
	}

#define BIND_IIIIES(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_I(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_I(ctx, 3), GET_E(ctx, 4),		\
			       GET_S(ctx, 5)));				\
	}

#define BIND_EIIIISI(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_I(ctx, 3), GET_I(ctx, 4),		\
			       GET_S(ctx, 5), GET_I(ctx, 6)));		\
	}

#define BIND_EI6SI(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_I(ctx, 3), GET_I(ctx, 4),		\
			       GET_I(ctx, 5), GET_I(ctx, 6),		\
			       GET_S(ctx, 7), GET_I(ctx, 8)));		\
	}

#define DUMB_FUNC(x)							\
	static JSValue qjs##x(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		printf("BIND not yet implemented %s\n", #x);		\
		return JS_NULL;						\
	}

typedef struct {
	YScriptOps ops;
	JSRuntime *runtime;
	JSContext *ctx;
} YScriptQjs;

static void destroy_entity(JSRuntime *rt, JSValue val)
{
	Entity *e = JS_GetOpaque(val, freeable_entity_class_id);
	yeDestroy(e);
}


static void destroy_entity_it(JSRuntime *rt, JSValue val)
{
	void *e = JS_GetOpaque(val, it_class_id);
	free(e);
}

static JSClassDef it_class = {
	"Entity_Iterator",
	.finalizer = destroy_entity_it,
};

static JSClassDef entity_class = {
	"Entity",
	.finalizer = NULL,
};

static JSClassDef freeable_entity_class = {
	"F_Entity",
	.finalizer = destroy_entity,
};

static JSClassDef sm_class = {
	"script_manager", .finalizer = NULL
};
static JSClassDef widget_class = {
	"WidgetState", .finalizer = NULL
};

struct EntityIterator {
	Entity *e;
	int i;
};

static inline JSValue mk_ent(JSContext *ctx, Entity *e, int add_destroy)
{
	JSValue obj;

	if (!e)
		return JS_NULL;
	if (add_destroy) {
		obj = JS_NewObjectClass(ctx, freeable_entity_class_id);
	} else {
		obj = JS_NewObjectClass(ctx, entity_class_id);
	}

	JS_SetOpaque(obj, e);
	return obj;
}

static inline JSValue new_str(JSContext *ctx, const char *s)
{
	if (!s)
		return JS_NewString(ctx, "");
	return JS_NewString(ctx, s);
}

static inline JSValue new_ent(JSContext *ctx, Entity *e)
{
	return mk_ent(ctx, e, 0);
}

DUMB_FUNC(yevCreateGrp);
BIND_ED(ywCanvasRotate);
BIND_EII(ywCanvasObjSetPos, 3, 0);
BIND_EIIE(ywCanvasNewText, 2, 2);
BIND_IESI(yeCreateIntAt, 4, 0);
BIND_ESI(yeCreateArrayAt, 3, 0);
BIND_IES(yeReCreateInt, 3, 0);
BIND_SES(yeReCreateString, 3, 0);
BIND_EID(yeSetFloatAt, 3, 0);
BIND_DESI(yeCreateFloatAt, 4, 0);
BIND_EESI(ywCanvasNewPolygonExt, 3, 1);
BIND_EI6SI(ywCanvasNewTriangleExt, 8, 1);
BIND_EIIES(ywCanvasNewTextExt, 5, 0);
BIND_IIIIES(yeCreateQuadInt, 4, 2);

#define NO_ywTextureNewImg
/* make all bindings here */
#include "binding.c"

static JSValue functions[256];
static uint64_t f_mask[4];

static JSValue qjsyeCreateFunction(JSContext *ctx, JSValueConst this_val,
				   int argc, JSValueConst *argv)
{
	if (JS_IsFunction(ctx, argv[0])) {
		Entity *r;
		char *rname = NULL;
		int mask_i = 0;
		int f_pos = 0;
		int f_mod;

	again:
		if (f_mask[mask_i] == YUI_MASK_FULL) {
			if (++mask_i < 4) {
				f_pos += 64;
				goto again;
			}
			DPRINT_ERR("too much function\n");
			return JS_NULL;
		}
		f_mod = YUI_FIRST_ZERO(f_mask[mask_i]);
		f_mask[mask_i] |= 1 << f_mod;
		f_pos += f_mod;
		asprintf(&rname, "_r%d", f_pos);
		r = yeCreateFunctionExt(rname,
					ygGetManager("js"),
					GET_E(ctx, 1),
					GET_S(ctx, 2),
					YE_FUNC_NO_FASTPATH_INIT);
		functions[f_pos] = JS_DupValue(ctx, argv[0]);
		free(rname);
		YE_TO_FUNC(r)->idata = f_pos + 1;
		return mk_ent(ctx, r, !GET_E(ctx, 1));
	}
	return mk_ent(ctx, yeCreateFunction(GET_S(ctx, 0),
					    ygGetManager("js"),
					    GET_E(ctx, 1),
					    GET_S(ctx, 2)),
	       !GET_E(ctx, 1));

}

static JSValue qjsyjsCall(JSContext *ctx, JSValueConst this_val,
			  int argc, JSValueConst *argv)
{
	const char *name = NULL;
	JSValue r = argv[0];
	JSValue func = JS_NULL;
	JSValue global_obj = JS_GetGlobalObject(ctx);

	if (JS_IsObject(r)) {
		Entity *er = GET_E_(r);

		name = yeGetString(er);
		if (yeType(er) == YFUNCTION &&
		    YE_TO_FUNC(er)->manager == cur_manager) {
			int fidx = yeIData(er);

			if (fidx) {
				func = functions[fidx - 1];
				goto call;
			}
		}
		func = JS_GetPropertyStr(ctx, global_obj, name);
		if (!JS_IsFunction(ctx, func)) {
			return JS_NULL;
		}
	} else if (JS_IsFunction(ctx, r)) {
		func = r;
	}
call:
	global_obj = JS_GetGlobalObject(ctx);
	r = call_exept(ctx, func, global_obj, argc - 1, argv + 1);
	return r;
}


static void e_destroy(void *manager, Entity *e)
{
	int id;

	if (yeType(e) == YFUNCTION && (id = yeIData(e))) {
		id--;
		int mi = id / 64;
		int mm = id & 63;

		JS_FreeValue(CTX(manager), functions[id]);
		f_mask[mi] &= ~(1LL << mm);
	}
}

static JSValue qjsyeGet(JSContext *ctx, JSValueConst this_val,
			int argc, JSValueConst *argv)
{
	if (JS_IsNumber(argv[1]))
		return new_ent(ctx, yeGet(GET_E(ctx, 0), GET_I(ctx, 1)));
	else
		return new_ent(ctx, yeGet(GET_E(ctx, 0), GET_S(ctx, 1)));
	return JS_NULL;
}

static inline void call_set_arg(JSContext *L, int i, union ycall_arg *yargs,
				int *types, int nb, JSValueConst *argv,
				int argc)
{
	if (JS_IsNumber(argv[i])) {
		types[nb] = YS_INT;
		yargs[nb].i = GET_I(L, i);
	} else if (JS_IsString(argv[i])) {
		types[nb] = YS_STR;
		yargs[nb].str = GET_S(L, i);
	} else if (JS_IsObject(argv[i])) {
		types[nb] = YS_ENTITY;
		yargs[nb].e = GET_E(L, i);
	}
}

/* destroy only work on non freeable entity */
static JSValue qjsyeDestroy(JSContext *ctx, JSValueConst this_val,
			    int argc, JSValueConst *argv)
{
	Entity *e = JS_GetOpaque(argv[0], entity_class_id);

	yeDestroy(e);
	return JS_NULL;
}

static JSValue qjsyesCall(JSContext *ctx, JSValueConst this_val,
			  int argc, JSValueConst *argv)
{
	int nb = argc - 1;
	Entity *f = GET_E(ctx, 0);
	union ycall_arg yargs[nb];
	int types[nb];

	for (int i = 0; i < nb; ++i) {
		call_set_arg(ctx, i + 1, yargs, types, i, argv, argc);
	}
	struct ys_ret ret = yesCall2Int(f, nb, yargs, types);

	if (ret.t == YS_ENTITY)
		return new_ent(ctx, ret.v.e);
	else if (ret.t == YS_INT)
		return JS_NewInt64(ctx, ret.v.i);
	else if (ret.t == YS_STR)
		return JS_NewString(ctx, ret.v.str);
	return JS_NULL;
}

static JSValue qjsyevCheckKeys(JSContext *ctx, JSValueConst this_val,
			       int argc, JSValueConst *argv)
{
	Entity *eves = GET_E(ctx, 0);
	int type = GET_I(ctx, 1);
	int k[128];
	int i;

	if (argc >127) {
		DPRINT_ERR("too much args");
		return JS_NULL;
	}

	for (i = 2; i < argc; ++i) {
		k[i - 2] = GET_I(ctx, i);
	}

	k[i - 2] = 0;
	return JS_NewBool(ctx, yevCheckKeysInt(eves, type, k));
}

static JSValue qjsywidNewWidget(JSContext *ctx, JSValueConst this_val,
				int argc, JSValueConst *argv)
{
	YWidgetState *r = ywidNewWidget(GET_E(ctx, 0), GET_S(ctx, 1));
	JSValue obj = JS_NewObjectClass(ctx, widget_class_id);

	JS_SetOpaque(obj, r);
	return obj;
}

static JSValue qjsygGetManager(JSContext *ctx, JSValueConst this_val,
			       int argc, JSValueConst *argv)
{
	void *r = ygGetManager(GET_S(ctx, 0));
	JSValue obj = JS_NewObjectClass(ctx, script_manager_class_id);

	JS_SetOpaque(obj, r);
	return obj;
}

static JSValue qjsygFileToEnt(JSContext *ctx, JSValueConst this_val,
			       int argc, JSValueConst *argv)
{
	return mk_ent(ctx, ygFileToEnt(GET_I(ctx, 0), GET_S(ctx, 1), GET_E(ctx, 2)),
		      !GET_E(ctx, 2));
}

static JSValue qjsygLoadScript(JSContext *ctx, JSValueConst this_val,
			       int argc, JSValueConst *argv)
{
	int r = ygLoadScript(
		GET_E(ctx, 0),
		JS_GetOpaque(argv[1], script_manager_class_id),
		GET_S(ctx, 2));

	return JS_NewInt32(ctx, r);
}

static JSValue qjsysLoadFile(JSContext *ctx, JSValueConst this_val,
			     int argc, JSValueConst *argv)
{
	int r = ysLoadFile(
		JS_GetOpaque(argv[0], script_manager_class_id),
		GET_S(ctx, 1));

	return JS_NewInt32(ctx, r);
}

static JSValue qjsyeReCreateArray(JSContext *ctx, JSValueConst this_val,
				  int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeReCreateArray(GET_E(ctx, 0), GET_S(ctx, 1),
					   GET_E(ctx, 2)),
		      !GET_E(ctx, 0));
}

static JSValue qjsywPosAddCopy(JSContext *ctx, JSValueConst this_val,
			       int argc, JSValueConst *argv)
{
	return mk_ent(ctx, ywPosAddCopy(GET_E(ctx, 0), GET_I(ctx, 1), GET_I(ctx, 2),
					GET_E(ctx, 3), GET_S(ctx, 4)),
		      !GET_E(ctx, 0));
}


static JSValue qjsywCanvasNewCollisionsArrayWithRectangle(
	JSContext *ctx, JSValueConst this_val,
	int argc, JSValueConst *argv
	)
{
	return mk_ent(ctx,
		      ywCanvasNewCollisionsArrayWithRectangle(GET_E(ctx, 0),
							      GET_E(ctx, 1)),
		      1);
}

static JSValue qjsyeCreateArray(JSContext *ctx, JSValueConst this_val,
				int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeCreateArray(GET_E(ctx, 0), GET_S(ctx, 1)),
		      !GET_E(ctx, 0));
}

static JSValue qjsyeCreateHash(JSContext *ctx, JSValueConst this_val,
			       int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeCreateHash(GET_E(ctx, 0), GET_S(ctx, 1)),
		      !GET_E(ctx, 0));
}

static JSValue qjsyeCreateCopy(JSContext *ctx, JSValueConst this_val,
			       int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeCreateCopy(GET_E(ctx, 0),
					GET_E(ctx, 1),
					GET_S(ctx, 2)),
		      !GET_E(ctx, 1));
}

static JSValue qjsyeCreateCopy2(JSContext *ctx, JSValueConst this_val,
				int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeCreateCopy2(GET_E(ctx, 0),
					GET_E(ctx, 1),
					GET_S(ctx, 2),
					GET_I(ctx, 3)),
		      !GET_E(ctx, 1));
}

static JSValue qjsyeCreateFloat(JSContext *ctx, JSValueConst this_val,
			      int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeCreateFloat(GET_D(ctx, 0), GET_E(ctx, 1),
					 GET_S(ctx, 2)), !GET_E(ctx, 1));
}

static JSValue qjsyeCreateInt(JSContext *ctx, JSValueConst this_val,
			      int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeCreateInt(GET_I(ctx, 0), GET_E(ctx, 1),
				       GET_S(ctx, 2)), !GET_E(ctx, 1));
}

static JSValue qjsyeCreateString(JSContext *ctx, JSValueConst this_val,
				 int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeCreateString(GET_S(ctx, 0), GET_E(ctx, 1),
					  GET_S(ctx, 2)), !GET_E(ctx, 1));
}

static JSValue qjsywRectCreateInts(JSContext *ctx, JSValueConst this_val,
				   int argc, JSValueConst *argv)
{
	return mk_ent(ctx, ywRectCreateInts(GET_I(ctx, 0), GET_I(ctx, 1),
					    GET_I(ctx, 2), GET_I(ctx, 3),
					    GET_E(ctx, 4), GET_S(ctx, 5)),
		      !GET_E(ctx, 4));
}

static JSValue qjsywTextureNew(JSContext *ctx, JSValueConst this_val,
			       int argc, JSValueConst *argv)
{
	return mk_ent(ctx, ywTextureNew(GET_E(ctx, 0), GET_E(ctx, 1),
					GET_S(ctx, 2)), !GET_E(ctx, 1));
}

static JSValue qjsywTextureNewImg(JSContext *ctx, JSValueConst this_val,
				  int argc, JSValueConst *argv)
{
	return mk_ent(ctx, ywTextureNewImg(GET_S(ctx, 0), GET_E(ctx, 1),
					   GET_E(ctx, 2), GET_S(ctx, 3)),
		      !GET_E(ctx, 2));
}


static JSValue qjsyeSetIntAt(JSContext *ctx, JSValueConst this_val,
			     int argc, JSValueConst *argv)
{
	if (JS_IsNumber(argv[1]))
		yeSetIntAt(GET_E(ctx, 0), GET_I(ctx, 1), GET_I(ctx, 2));
	else
		yeSetIntAtStrIdx(GET_E(ctx, 0), GET_S(ctx, 1), GET_I(ctx, 2));
	return JS_NULL;
}

static JSValue qjsyeGetIntAt(JSContext *ctx, JSValueConst this_val,
			     int argc, JSValueConst *argv)
{
	if (JS_IsNumber(argv[1]))
		return JS_NewInt64(ctx, yeGetIntAt(GET_E(ctx, 0), GET_I(ctx, 1)));
	else
		return JS_NewInt64(ctx, yeGetIntAt(GET_E(ctx, 0), GET_S(ctx, 1)));
}

static JSValue qjsyeGetStringAt(JSContext *ctx, JSValueConst this_val,
				int argc, JSValueConst *argv)
{
	if (JS_IsNumber(argv[1]))
		return new_str(ctx, yeGetStringAt(GET_E(ctx, 0), GET_I(ctx, 1)));
	else
		return new_str(ctx, yeGetStringAt(GET_E(ctx, 0), GET_S(ctx, 1)));
}

static JSValue qjsywPosCreate(JSContext *ctx, JSValueConst this_val,
			      int argc, JSValueConst *argv)
{
	if (JS_IsNumber(argv[0])) {
		return mk_ent(ctx, ywPosCreate(GET_I(ctx, 0), GET_I(ctx, 1),
					       GET_E(ctx, 2), GET_S(ctx, 3)),
			      !GET_E(ctx, 2));
	} else {
		if (JS_IsNumber(argv[1]))
			return mk_ent(ctx, ywPosCreate(GET_E(ctx, 0), 0,
						       GET_E(ctx, 2), GET_S(ctx, 3)),
				      !GET_E(ctx, 2));
		return mk_ent(ctx, ywPosCreate(GET_E(ctx, 0), 0, GET_E(ctx, 1), GET_S(ctx, 2)),
			      !GET_E(ctx, 1));
	}
}

static JSValue qjsyeTryCreateInt(JSContext *ctx, JSValueConst this_val,
				 int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeTryCreateInt(GET_I(ctx, 0), GET_E(ctx, 1),
					  GET_S(ctx, 2)), !GET_E(ctx, 1));
}

static JSValue qjsyeIncrAt(JSContext *ctx, JSValueConst this_val,
				 int argc, JSValueConst *argv)
{
	Entity *e = GET_E(ctx, 0);

	if (JS_IsNumber(argv[1])) {
		yeIncrAt(e, GET_I(ctx, 1));
	} else {
		yeIncrAt(e, GET_S(ctx, 1));
	}
	return JS_NULL;
}

static JSValue qjsywPosSet(JSContext *ctx, JSValueConst this_val,
			   int argc, JSValueConst *argv)
{
	Entity *p = GET_E(ctx, 0);
	if (JS_IsNumber(argv[1])) {
		return mk_ent(ctx, ywPosSet(p, GET_I(ctx, 1), GET_I(ctx, 2)), 0);
	} else {
		return mk_ent(ctx, ywPosSet(p, GET_E(ctx, 1), 0), 0);
	}
}

static JSValue qjsyeAddAt(JSContext *ctx, JSValueConst this_val,
				 int argc, JSValueConst *argv)
{
	Entity *e = GET_E(ctx, 0);

	if (JS_IsNumber(argv[1])) {
		yeAddAt(e, GET_I(ctx, 1), GET_I(ctx, 2));
	} else {
		yeAddAt(e, GET_S(ctx, 1), GET_I(ctx, 2));
	}
	return JS_NULL;
}

static JSValue qjsyeTryCreateString(JSContext *ctx, JSValueConst this_val,
				    int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeTryCreateString(GET_S(ctx, 0), GET_E(ctx, 1),
					     GET_S(ctx, 2)), !GET_E(ctx, 1));
}

static JSValue qjsyeTryCreateArray(JSContext *ctx, JSValueConst this_val,
				 int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeTryCreateArray(GET_E(ctx, 0), GET_S(ctx, 1)),
		      !GET_E(ctx, 0));
}

static JSValue qjsywCanvasNewImg(JSContext *ctx, JSValueConst this_val,
				 int argc, JSValueConst *argv)
{
	return mk_ent(ctx, ywCanvasNewImg(GET_E(ctx, 0), GET_I(ctx, 1), GET_I(ctx, 2),
					  GET_S(ctx, 3), GET_E(ctx, 4)),
		      0);
}

static JSValue qjsto_str(JSContext *ctx, JSValueConst this_val,
			 int argc, JSValueConst *argv)
{
	if (JS_IsNumber(argv[0]))
		return JS_NewString(ctx, (void *)GET_I(ctx, 0));
	return JS_NULL;
}

static JSValue qjsyent_to_str(JSContext *ctx, JSValueConst this_val,
			  int argc, JSValueConst *argv)
{
	JSValue r;
	Entity *e = GET_E(ctx, 0);
	char *s = yeToCStr(e, 6, YE_FORMAT_PRETTY);

	r = JS_NewString(ctx, s);
	free(s);
	return r;
}

static JSValue qjsygRegistreFunc(JSContext *ctx, JSValueConst this_val,
				 int argc, JSValueConst *argv)
{
	ygRegistreFuncInternal(cur_manager, GET_I(ctx, 0), GET_S(ctx, 1), GET_S(ctx, 2));
	return JS_NULL;
}

static int loadString(void *s, const char *str);

static JSValue entity_bondary(JSContext *ctx, JSValueConst this_val,
			     int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);
	yeIntForceBound(e, GET_I(ctx, 0), GET_I(ctx, 1));
	return new_ent(ctx, e);
}

static JSValue entity_to_int(JSContext *ctx, JSValueConst this_val,
			     int argc, JSValueConst *argv)
{
	return JS_NewInt64(ctx, yeGetInt(GET_E_(this_val)));
}

static JSValue entity_to_str(JSContext *ctx, JSValueConst this_val,
			     int argc, JSValueConst *argv)
{
	return JS_NewString(ctx, yeGetString(GET_E_(this_val)));
}

static JSValue array_set_at(JSContext *ctx, JSValueConst this_val,
			    int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	if (JS_IsNumber(argv[1])) {
		if (JS_IsNumber(argv[0])) {
			return mk_ent(ctx, yeCreateIntAt(GET_I(ctx, 1), e, NULL, GET_I(ctx, 0)), 0);
		} else {
			return mk_ent(ctx, yeReCreateInt(GET_I(ctx, 1), e, GET_S(ctx, 0)), 0);
		}
	} else if (JS_IsString(argv[1])) {
		if (JS_IsNumber(argv[0])) {
			return mk_ent(ctx, yeCreateStringAt(GET_S(ctx, 1), e, NULL, GET_I(ctx, 0)), 0);
		} else {
			return mk_ent(ctx, yeReCreateString(GET_S(ctx, 1), e, GET_S(ctx, 0)), 0);
		}
	} else if (JS_IsObject(argv[1])) {
		if (JS_IsNumber(argv[0])) {
			Entity *to_push = GET_E_(argv[1]);
			yePushAt(e, to_push, GET_I(ctx, 0));
			return mk_ent(ctx, to_push, 0);
		} else {
			return new_ent(ctx, yeReplaceBack(e, GET_E_(argv[1]), GET_S(ctx, 0)));
		}
	}
	return JS_NULL;
}

static JSValue array_clear(JSContext *ctx, JSValueConst this_val,
			    int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	yeClearArray(e);
	return JS_NULL;
}

static JSValue array_add_at(JSContext *ctx, JSValueConst this_val,
			    int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	if (JS_IsNumber(argv[0])) {
		yeAddAt(e, GET_I(ctx, 0), GET_I(ctx, 1));
	} else if (JS_IsString(argv[0])) {
		yeAddAt(e, GET_S(ctx, 0), GET_I(ctx, 1));
	}
	return JS_NULL;
}

static JSValue entity_add(JSContext *ctx, JSValueConst this_val,
			  int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	if (JS_IsNumber(argv[0])) {
		yeAdd(e, GET_I(ctx, 0));
		return mk_ent(ctx, e, 0);
	} else if (JS_IsString(argv[0])) {
		yeAdd(e, GET_S(ctx, 0));
		return mk_ent(ctx, e, 0);
	}
	return JS_NULL;
}

static JSValue entity_mult(JSContext *ctx, JSValueConst this_val,
			  int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	if (JS_IsNumber(argv[0])) {
		yeMultInt(e, GET_I(ctx, 0));
		return mk_ent(ctx, e, 0);
	}
	return JS_NULL;
}

static JSValue array_push(JSContext *ctx, JSValueConst this_val,
			  int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	if (JS_IsNumber(argv[0])) {
		yeCreateInt(GET_I(ctx, 0), e, GET_S(ctx, 1));
	} else if (JS_IsString(argv[0])) {
		yeCreateString(GET_S(ctx, 0), e, GET_S(ctx, 1));
	} else {
		yePushBack(e, GET_E(ctx, 0), GET_S(ctx, 1));
	}
	return JS_NULL;
}

static JSValue entity_type(JSContext *ctx, JSValueConst this_val,
			   int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	return JS_NewInt64(ctx, yeType(e));
}

static JSValue array_get(JSContext *ctx, JSValueConst this_val,
			 int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	if (JS_IsNumber(argv[0])) {
		return new_ent(ctx, yeGet(e, GET_I(ctx, 0)));
	} else if (JS_IsString(argv[0])) {
		return new_ent(ctx, yeGet(e, GET_S(ctx, 0)));
	} else {
		return new_ent(ctx, yeGet(e, GET_E(ctx, 0)));
	}
	return JS_NULL;
}


static JSValue array_remove(JSContext *ctx, JSValueConst this_val,
			 int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	if (JS_IsNumber(argv[0])) {
		yeRemoveChild(e, GET_I(ctx, 0));
	} else if (JS_IsString(argv[0])) {
		yeRemoveChild(e, GET_S(ctx, 0));
	} else {
		yeRemoveChild(e, GET_E(ctx, 0));
	}
	return JS_NULL;
}

static JSValue array_geti(JSContext *ctx, JSValueConst this_val,
			 int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	if (JS_IsNumber(argv[0])) {
		return JS_NewInt64(ctx, yeGetIntAt(e, GET_I(ctx, 0)));
	} else if (JS_IsString(argv[0])) {
		return JS_NewInt64(ctx, yeGetIntAt(e, GET_S(ctx, 0)));
	} else {
		return JS_NewInt64(ctx, yeGetIntAt(e, GET_E(ctx, 0)));
	}
	return JS_NULL;
}

static JSValue array_getf(JSContext *ctx, JSValueConst this_val,
			 int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	if (JS_IsNumber(argv[0])) {
		return JS_NewFloat64(ctx, yeGetFloatAt(e, GET_I(ctx, 0)));
	} else if (JS_IsString(argv[0])) {
		return JS_NewFloat64(ctx, yeGetFloatAt(e, GET_S(ctx, 0)));
	} else {
		return JS_NewFloat64(ctx, yeGetFloatAt(e, GET_E(ctx, 0)));
	}
	return JS_NULL;
}

static JSValue array_getb(JSContext *ctx, JSValueConst this_val,
			 int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	if (JS_IsNumber(argv[0])) {
		return JS_NewBool(ctx, yeGetIntAt(e, GET_I(ctx, 0)));
	} else if (JS_IsString(argv[0])) {
		return JS_NewBool(ctx, yeGetIntAt(e, GET_S(ctx, 0)));
	} else {
		return JS_NewBool(ctx, yeGetIntAt(e, GET_E(ctx, 0)));
	}
	return JS_NULL;
}

static JSValue array_gets(JSContext *ctx, JSValueConst this_val,
			 int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	if (JS_IsNumber(argv[0])) {
		const char *s = yeGetStringAt(e, GET_I(ctx, 0));

		if (!s)
			return JS_NULL;
		return JS_NewString(ctx, s);
	} else if (JS_IsString(argv[0])) {
		const char *s = yeGetStringAt(e, GET_S(ctx, 0));

		if (!s)
			return JS_NULL;
		return JS_NewString(ctx, s);
	} else {
		const char *s = yeGetStringAt(e, GET_E(ctx, 0));

		if (!s)
			return JS_NULL;
		return JS_NewString(ctx, s);
	}
	return JS_NULL;
}

static JSValue entity_len(JSContext *ctx, JSValueConst this_val,
			 int argc, JSValueConst *argv)
{
	Entity *e = GET_E_(this_val);

	return JS_NewInt64(ctx, yeLen(e));
}

static JSValue entity_call(JSContext *ctx, JSValueConst this_val,
			   int argc, JSValueConst *argv)
{
	int nb = argc;
	Entity *f = GET_E_(this_val);
	union ycall_arg yargs[nb];
	int types[nb];

	for (int i = 0; i < nb; ++i) {
		call_set_arg(ctx, i, yargs, types, i, argv, argc);
	}
	struct ys_ret ret = yesCall2Int(f, nb, yargs, types);

	if (ret.t == YS_ENTITY)
		return new_ent(ctx, ret.v.e);
	else if (ret.t == YS_INT)
		return JS_NewInt64(ctx, ret.v.i);
	else if (ret.t == YS_STR)
		return JS_NewString(ctx, ret.v.str);
	return JS_NULL;
}

static JSValue array_forEach_(JSContext *ctx, JSValueConst this_val,
			      int argc, JSValueConst *argv, int skipp_null)
{
	Entity *e = GET_E_(this_val);
	JSValue callback = argv[0];
	JSValue arg = argv[1];
	JSValue r;

	if (yeType(e) == YHASH) {
		Entity *vvar;
		const char *kkey;

		kh_foreach(((HashEntity *)e)->values,
			   kkey, vvar, {
				   if (skipp_null && !vvar)
					   continue;
				   JSValue jse = new_ent(ctx, vvar);
				   JSValue jsidx = JS_NewString(ctx, kkey);
				   r = call_exept(ctx, callback, JS_GetGlobalObject(ctx), 4,
					       (JSValueConst []){
						   jse,
						   jsidx,
						   this_val,
						   arg
					   });
				   if (JS_IsBool(r) && JS_ToBool(ctx, r)) {
					   break;
				   }

			   });
		return JS_NULL;
	}

	for (int l = yeLen(e), i = 0; i < l; ++i) {
		if (skipp_null && !yeGet(e, i))
			continue;
		JSValue jse = new_ent(ctx, yeGet(e, i));
		JSValue jsidx = JS_NewInt32(ctx, i);
		r = call_exept(ctx, callback, JS_GetGlobalObject(ctx), 4, (JSValueConst []){
				jse,
				jsidx,
				this_val,
				arg
			});
		if (JS_IsBool(r) && JS_ToBool(ctx, r)) {
			break;
		}
	}
	return JS_NULL;
}

static JSValue array_forEach(JSContext *ctx, JSValueConst this_val,
			      int argc, JSValueConst *argv)
{
	return array_forEach_(ctx, this_val, argc, argv, 0);
}

static JSValue array_forEachNN(JSContext *ctx, JSValueConst this_val,
			      int argc, JSValueConst *argv)
{
	return array_forEach_(ctx, this_val, argc, argv, 1);
}

static JSValue container_it_next(JSContext *ctx, JSValueConst this_val,
				 int argc, JSValueConst *argv)
{
	JSValue ret = JS_NewObject(ctx);
	JSAtom atom;
	int is_done = 1;
	struct EntityIterator *it = JS_GetOpaque(this_val, it_class_id);
	Entity *container = it->e;
	int idx = it->i;

	if (it->e->type == YHASH) {
		/* Entity *old = it->cur_e; */
		Entity *vvar;
		const char *kkey;
		int  i = 0;

		kh_foreach(((HashEntity *)container)->values,
			   kkey, vvar, {
				   if (i >= idx) {
					   it->i = ++i;
					   atom = JS_NewAtom(ctx, "value");
					   JSValue val = new_ent(ctx, vvar);
					   JS_SetProperty(ctx, ret, atom, val);
					   JSValue jsidx = JS_NewString(ctx, kkey);
					   JS_SetPropertyStr(ctx, ret, "index", jsidx);
					   is_done = 0;
					   goto out;
				   }
				   ++i;
			   });
	} else {
		for (size_t i = idx; i < yeLen(container); i++) {
			if (!yeGet(container, i))
				continue;
			is_done = 0;
			atom = JS_NewAtom(ctx, "value");
			JSValue val = new_ent(ctx, yeGet(container, i));
			JS_SetProperty(ctx, ret, atom, val);
			it->i = ++i;
			goto out;
		}
	}

out:
	atom = JS_NewAtom(ctx, "done");
	JSValue is_done_js = JS_NewBool(ctx, is_done);
	JS_SetProperty(ctx, ret, atom, is_done_js);
	return ret;
}

static const JSCFunctionListEntry js_ent_it_proto_funcs[] = {
	JS_CFUNC_DEF("next", 0, container_it_next)
};

static JSValue array_iterator(JSContext *ctx, JSValueConst this_val,
			      int argc, JSValueConst *argv)
{
	JSValue obj = JS_NewObjectClass(ctx, it_class_id);
	Entity *e = GET_E_(this_val);
	int ent_type = yeType(e);
	if (ent_type != YHASH && ent_type != YARRAY)
		return JS_NULL;

	struct EntityIterator *it = malloc(sizeof *it);
	it->e = e;
	it->i = 0;
	JS_SetOpaque(obj, it);
	return obj;
}

static const JSCFunctionListEntry js_ent_proto_funcs[] = {
    JS_CFUNC_DEF("forEachNonNull", 0, array_forEachNN),
    JS_CFUNC_DEF("forEach", 0, array_forEach),
    JS_CFUNC_DEF("clear", 0, array_clear),
    JS_CFUNC_DEF("call", 0, entity_call),
    JS_CFUNC_DEF("rm", 0, array_remove),
    JS_CFUNC_DEF("type", 0, entity_type),
    JS_CFUNC_DEF("get", 1, array_get),
    JS_CFUNC_DEF("push", 1, array_push),
    JS_CFUNC_DEF("getb", 1, array_getb),
    JS_CFUNC_DEF("geti", 1, array_geti),
    JS_CFUNC_DEF("getf", 1, array_getf),
    JS_CFUNC_DEF("gets", 1, array_gets),
    JS_CFUNC_DEF("addAt", 0, array_add_at),
    JS_CFUNC_DEF("add", 0, entity_add),
    JS_CFUNC_DEF("mult", 0, entity_mult),
    JS_CFUNC_DEF("boundary", 0, entity_bondary),
    JS_CFUNC_DEF("toInt", 1, entity_to_int),
    JS_CFUNC_DEF("i", 1, entity_to_int),
    JS_CFUNC_DEF("s", 1, entity_to_str),
    JS_CFUNC_DEF("setAt", 1, array_set_at),
    JS_CFUNC_DEF("[Symbol.iterator]", 1, array_iterator),
    JS_CFUNC_DEF("len", 1, entity_len)
};

#define countof(x) (sizeof(x) / sizeof((x)[0]))

static int init(void *sm, void *args)
{
	JSRuntime *rn = JS_NewRuntime();
	JSContext *ctx = JS_NewContext(rn);
	JSValue global_obj = JS_GetGlobalObject(ctx);

	RUNTIME(sm) = rn;
	CTX(sm) = ctx;

	/* Stole that from qjs.c source */
        /* system modules */
	js_std_add_helpers(ctx, 0, 0);

	JS_NewClass(JS_GetRuntime(ctx), entity_class_id, &entity_class);
	JS_NewClass(JS_GetRuntime(ctx), freeable_entity_class_id,
		    &freeable_entity_class);
	JS_NewClass(JS_GetRuntime(ctx), it_class_id, &it_class);

	JSValue iterator_proto = JS_NewObject(ctx);
	JS_SetPropertyFunctionList(ctx, iterator_proto, js_ent_it_proto_funcs,
				   countof(js_ent_it_proto_funcs));
	JS_SetClassProto(ctx, it_class_id, iterator_proto);

	JSValue ent_proto = JS_NewObject(ctx);
	JS_SetPropertyFunctionList(ctx, ent_proto, js_ent_proto_funcs,
				   countof(js_ent_proto_funcs));
	JS_SetClassProto(ctx, entity_class_id, ent_proto);
	JS_SetClassProto(ctx, freeable_entity_class_id, ent_proto);

	JS_NewClass(JS_GetRuntime(ctx), script_manager_class_id, &sm_class);
	JS_NewClass(JS_GetRuntime(ctx), widget_class_id, &widget_class);

#define PUSH_I_GLOBAL_VAL(x, val)					\
	JS_SetPropertyStr(ctx, global_obj, #x,				\
			  JS_NewInt64(ctx, val));

#define PUSH_I_GLOBAL(x)				\
	JS_SetPropertyStr(ctx, global_obj, #x,		\
			  JS_NewInt64(ctx, x));

#define BIND(x, args, oargs)						\
	JS_SetPropertyStr(ctx, global_obj, #x,				\
			  JS_NewCFunction(ctx, qjs##x, #x, args + oargs));

	PUSH_I_GLOBAL_VAL(is_yirl, 1);
	PUSH_I_GLOBAL_VAL(YEVE_NOTHANDLE, NOTHANDLE);
	PUSH_I_GLOBAL_VAL(YEVE_ACTION, ACTION);

	BIND(ywPosSet, 2, 1);
	BIND(ywRectCreateInts, 6, 0);
	BIND(yeCreateFunction, 1, 2);
	BIND(yeCreateString, 1, 2);
	BIND(yeCreateFloat, 1, 2);
	BIND(yeCreateInt, 1, 2);
	BIND(yeCreateArray, 0, 2);
	BIND(yeCreateHash, 0, 2);
	BIND(yeCreateCopy, 0, 3);
	BIND(yeCreateCopy2, 0, 4);
	BIND(ywPosAddCopy, 3, 2);
	BIND(yeReCreateArray, 2, 1);
	BIND(yeGetStringAt, 0, 2);
	BIND(ygLoadScript, 3, 0);
	BIND(ywCanvasNewImg, 4, 1);
	BIND(ysLoadFile, 2, 0);
	BIND(ygGetManager, 1, 0);
	BIND(ywidNewWidget, 2, 0);
	BIND(yeTryCreateInt, 1, 2);
	BIND(yeTryCreateString, 1, 2);
	BIND(yeTryCreateArray, 0, 2);
	BIND(yjsCall, 0, 10);
	BIND(yesCall, 0, 10);
	BIND(yevCheckKeys, 2, 10);
	BIND(yeDestroy, 1, 0);
	BIND(yent_to_str, 1, 0);
	BIND(ygFileToEnt, 2, 1);
	BIND(ywTextureNew, 1, 2);
	BIND(ywCanvasRotate, 2, 0);
	BIND(ywCanvasNewCollisionsArrayWithRectangle, 2, 0);
	BIND(ygRegistreFunc, 3, 0);
	BIND(ywCanvasNewText, 2, 2);
	BIND(yeCreateIntAt, 4, 0);
	BIND(yeCreateArrayAt, 3, 0);
	BIND(yeReCreateInt, 3, 0);
	BIND(yeReCreateString, 3, 0);
	BIND(yeCreateFloatAt, 4, 0);
	BIND(yeSetFloatAt, 3, 0);
	BIND(ywCanvasObjSetPos, 3, 0);
	BIND(ywCanvasNewTriangleExt, 8, 1);
	BIND(ywCanvasNewPolygonExt, 3, 1);
	BIND(ywCanvasNewTextExt, 5, 0);
	BIND(yeCreateQuadInt, 4, 2);


#define IN_CALL 1
	#include "binding.c"
#undef IN_CALL
	loadString(sm, "function ywSizeCreate(a,b,c,d) {"
		   "return ywPosCreate(a,b,c,d)"
		   "}");

	return 0;
}

static struct ys_ret _call(void *sm, int nb, union ycall_arg *args,
			   int *types, JSValue func)
{
	JSContext *ctx = CTX(sm);
	JSValueConst global_obj = JS_GetGlobalObject(ctx);

	JSValue vals[nb];
	JSValue  r;

	cur_manager = sm;
	for (int i = 0; i < nb; ++i) {
		if (types[i] == YS_VPTR) {
			void *ptr = args[i].vptr;

			if (yeIsPtrAnEntity(ptr))
				vals[i] = new_ent(ctx, ptr);
			else
				vals[i] = JS_NewInt64(ctx, (intptr_t)ptr);
		} else if (types[i] == YS_ENTITY) {
			vals[i] = new_ent(ctx, args[i].e);
		} else if (types[i] == YS_INT) {
			vals[i] = JS_NewInt64(ctx, args[i].i);
		} else if (types[i] == YS_STR) {
			vals[i] = JS_NewString(ctx, args[i].str);
		}
	}

	r = call_exept(ctx, func, global_obj, nb, vals);
	if (JS_IsObject(r)) {
		void *e = GET_E_(r);
		if (likely(e))
			return (struct ys_ret){.t=YS_ENTITY, .v.e=e};
		e = JS_GetOpaque(r, widget_class_id);
		if (e)
			return (struct ys_ret){.t=YS_VPTR, .v.vptr=e};
		e = JS_GetOpaque(r, script_manager_class_id);
		if (e)
			return (struct ys_ret){.t=YS_VPTR, .v.vptr=e};
	} else if (JS_IsNumber(r)) {
		int64_t ir;

		JS_ToInt64(ctx, &ir, r);
		return (struct ys_ret){.t=YS_INT, .v.i=ir};
	} else if (JS_IsString(r)) {
		return (struct ys_ret){.t=YS_STR, .v.str=JS_ToCString(ctx, r)};
	}
	return (struct ys_ret){.t=YS_VPTR, .v.vptr=0};;
}

static struct ys_ret fCall2(void *sm, void *sym, int nb,
		   union ycall_arg *args, int *t_arrray)
{
	return _call(sm, nb, args, t_arrray, functions[(intptr_t)sym - 1]);
}

static struct ys_ret call2(void *sm, const char *name, int nb, union ycall_arg *args,
			   int *types)
{
	JSContext *ctx = CTX(sm);
	JSValueConst global_obj = JS_GetGlobalObject(ctx);
	JSValue func = JS_GetPropertyStr(ctx, global_obj, name);

	return _call(sm, nb, args, types, func);
}

static void *fCall(void *sm, void *sym, int nb,
		   union ycall_arg *args, int *t_arrray)
{
	return _call(sm, nb, args, t_arrray, functions[(intptr_t)sym - 1]).v.vptr;
}

static void *call(void *sm, const char *name, int nb, union ycall_arg *args,
		  int *types)
{
	JSContext *ctx = CTX(sm);
	JSValueConst global_obj = JS_GetGlobalObject(ctx);
	JSValue func = JS_GetPropertyStr(ctx, global_obj, name);

	return _call(sm, nb, args, types, func).v.vptr;
}

static int destroy(void *sm)
{
	JS_FreeContext(CTX(sm));
	JS_FreeRuntime(RUNTIME(sm));
	return 0;
}

static int eval_str_(JSContext *ctx, const char *str, int l,
		     const char *filename)
{
	JSValue r = JS_Eval(ctx, str, l, filename, JS_EVAL_TYPE_GLOBAL);

	if (JS_IsException(r)) {
		js_std_dump_error(ctx);
		return -1;
	}
	return 0;
}

static int loadString(void *s, const char *str)
{
	return eval_str_(CTX(s), str, strlen(str), "(string)");
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
	eval_str_(CTX(s), yeGetString(fstr), yeLen(fstr), file);
	close(fd);
	return 0;
}

static void addFuncSymbole(void *sm, const char *name, int nbArgs, Entity *func)
{
	Entity *str = yeCreateString("", NULL, NULL);
	char *tmp_name;
	JSContext *ctx = CTX(sm);
	JSValueConst global_obj = JS_GetGlobalObject(ctx);

	if (!name)
		name = yeGetString(func);
	tmp_name = y_strdup_printf("%sGlobal", name);
	JS_SetPropertyStr(ctx, global_obj, tmp_name, new_ent(ctx, func));

	yeStringAdd(str, "function ");
	yeAddStr(str, name);
	yeAddStr(str, "(");
	for (int i = 0; i < nbArgs; ++i) {
		if (i)
			yeAddStr(str, ",");
		yeAddStr(str, "var");
		yeAddInt(str, i);
	}

	yeStringAdd(str, ") { return yesCall(");
	yeStringAdd(str, tmp_name);

	for (int i = 0; i < nbArgs; ++i) {
		yeAddStr(str, ", var");
		yeAddInt(str, i);
	}
	yeStringAdd(str, ") }");
	loadString(sm, yeGetString(str));
	free(tmp_name);
	yeDestroy(str);
}

static void trace(void *sm)
{
	JSContext *ctx = CTX(sm);
	JS_ThrowTypeError(ctx, "abort in quickjs:");
	js_std_dump_error(ctx);
}

static void *allocator(void)
{
	YScriptQjs *ret;

	ret = calloc(1, sizeof(*ret));
	ret->ops.init = init;
	ret->ops.destroy = destroy;
	ret->ops.loadFile = loadFile;
	ret->ops.loadString = loadString;
	ret->ops.e_destroy = e_destroy;
	ret->ops.call = call;
	ret->ops.call2 = call2;
	ret->ops.trace = trace;
	ret->ops.fastCall = fCall;
	ret->ops.fastCall2 = fCall2;
	ret->ops.addFuncSymbole = addFuncSymbole;
	return (void *)ret;
}

int ysQjsInit(void)
{
	t = ysRegister(allocator);
	JS_NewClassID(&entity_class_id);
	JS_NewClassID(&freeable_entity_class_id);
	JS_NewClassID(&script_manager_class_id);
	JS_NewClassID(&widget_class_id);
	return t;
}

int ysQjsEnd(void)
{
	return ysUnregiste(t);
}

int ysQjsGetType(void)
{
	return t;
}
