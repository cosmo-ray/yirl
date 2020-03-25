/*
**Copyright (C) 2020 Matthias Gatto
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
#include "keydef.h"
#include "widget.h"
#include "texture.h"
#include "pos.h"
#include "entity-script.h"
#include "events.h"

static int t = -1;

#define OPS(s) ((YScriptQjs *)s)->ops
#define RUNTIME(s) ((YScriptQjs *)s)->runtime
#define CTX(s) ((YScriptQjs *)s)->ctx

#define GET_E(ctx, idx)					\
	((struct entity_wrapper *)JS_GetOpaque(argv[idx], entity_class_id))->e

#define GET_S(ctx, idx)							\
	JS_ToCString(ctx, argv[idx])

#define GET_I(ctx, idx)							\
	({ int64_t res; JS_ToInt64(ctx, &res, argv[idx]);  res; })



#define LUAT(call)				\
	_Generic(call,				\
		 default: 0,			\
		 char *: 1,			\
		 const char *: 1,		\
		 Entity *: 1,			\
		 const Entity *: 1,		\
		 int: 1,			\
		 long: 1,			\
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
		 double: call,			\
		 float: call,			\
		 unsigned long: call,		\
		 unsigned int: call)

#define BIND_AUTORET(call)						\
	int t = LUAT(call);						\
	switch (t) {							\
	case 0:								\
		call;							\
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
		 char *: JS_NewString,			\
		 const char *: JS_NewString,		\
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

#define BIND_ES(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_S(ctx, 1)));				\
	}

#define BIND_EII(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2)));		\
	}

#define BIND_EEI(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_I(ctx, 2)));		\
	}

#define BIND_EES(f, useless...)						\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_S(ctx, 2)));		\
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

#define BIND_SEES(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_S(ctx, 0),				\
			       GET_E(ctx, 1), GET_E(ctx, 2),		\
			       GET_S(ctx, 3)));				\
	}

#define BIND_EIIE(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_E(ctx, 3)));				\
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

#define BIND_EIIIIS(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_I(ctx, 1), GET_I(ctx, 2),		\
			       GET_I(ctx, 3), GET_I(ctx, 4),		\
			       GET_S(ctx, 5)));				\
	}

#define DUMB_FUNC(x)						\
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

struct entity_wrapper {
	Entity *e;
	int should_free;
};

static JSClassID entity_class_id;

static void destroy_entity(JSRuntime *rt, JSValue val)
{
	printf("destroy_entity\n");
	struct entity_wrapper *ew = JS_GetOpaque(val, entity_class_id);
	if (ew->should_free)
		yeDestroy(ew->e);
	free(ew);
}


static JSClassDef entity_class = {
	"Entity",
	.finalizer = destroy_entity,
};

static inline JSValue mk_ent(JSContext *ctx, Entity *e, int add_destroy)
{
	JSValue obj = JS_NewObjectClass(ctx, entity_class_id);
	struct entity_wrapper *ew = malloc(sizeof(*ew));

	ew->e = e;
	ew->should_free = add_destroy;
	JS_SetOpaque(obj, ew);
	printf("%p %p\n", ew, e);
	return obj;
}

static inline JSValue new_ent(JSContext *ctx, Entity *e)
{
	return mk_ent(ctx, e, 0);
}

DUMB_FUNC(yevCreateGrp);
DUMB_FUNC(yeIncrAt);
DUMB_FUNC(yeAddAt);
BIND_EII(ywCanvasObjSetPos, 3, 0);
BIND_EIIE(ywCanvasNewText, 2, 2);

/* make all bindings here */
#include "binding.c"

static JSValue qjsyjsCall(JSContext *ctx, JSValueConst this_val,
			  int argc, JSValueConst *argv)
{
	const char *name = NULL;
	JSValue r = argv[0];
	JSValue func = JS_NULL;
	JSValue global_obj = JS_GetGlobalObject(ctx);

	printf("in qjsyjsCall\n");
	if (JS_IsObject(r)) {
		struct entity_wrapper *ew = JS_GetOpaque(r, entity_class_id);
		name = yeGetString(ew->e);
		func = JS_GetPropertyStr(ctx, global_obj, name);
	} else if (JS_IsFunction(ctx, r)) {
		func = r;
	}
	global_obj = JS_GetGlobalObject(ctx);
	return JS_Call(ctx, func, global_obj, argc - 1, argv + 1);
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
				int *types, int nb, JSValueConst *argv)
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

static JSValue qjsyesCall(JSContext *ctx, JSValueConst this_val,
			  int argc, JSValueConst *argv)
{
	int nb = argc - 1;
	Entity *f = GET_E(ctx, 0);
	union ycall_arg yargs[nb];
	int types[nb];

	for (int i = 0; i < nb; ++i) {
		call_set_arg(ctx, i + 1, yargs, types, i, argv);
	}
	return new_ent(ctx, yesCallInt(f, nb, yargs, types));
}

static JSValue qjsywRectCreateInts(JSContext *ctx, JSValueConst this_val,
				   int argc, JSValueConst *argv)
{
	return mk_ent(ctx, ywRectCreateInts(GET_I(ctx, 0), GET_I(ctx, 1),
					    GET_I(ctx, 2), GET_I(ctx, 3),
					    GET_E(ctx, 4), GET_S(ctx, 5)),
		      !GET_E(ctx, 4));
}

static JSValue qjsyeSetIntAt(JSContext *ctx, JSValueConst this_val,
			     int argc, JSValueConst *argv)
{
	yeSetIntAt(GET_E(ctx, 0), GET_I(ctx, 1), GET_I(ctx, 2));
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

static JSValue qjsywPosCreate(JSContext *ctx, JSValueConst this_val,
			      int argc, JSValueConst *argv)
{
	return mk_ent(ctx, ywPosCreate(GET_I(ctx, 0), GET_I(ctx, 1),
				       GET_E(ctx, 2), GET_S(ctx, 3)),
		      !GET_E(ctx, 2));
}

static JSValue qjsto_str(JSContext *ctx, JSValueConst this_val,
			 int argc, JSValueConst *argv)
{
	if (JS_IsNumber(argv[0]))
		return JS_NewString(ctx, (void *)GET_I(ctx, 0));
	return JS_NULL;
}

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

	BIND(ywRectCreateInts, 6, 0);

#define IN_CALL 1
	#include "binding.c"
#undef IN_CALL

	return 0;
}

static void *call(void *sm, const char *name, int nb, union ycall_arg *args,
		  int *types)
{
	JSContext *ctx = CTX(sm);
	JSValueConst global_obj = JS_GetGlobalObject(ctx);
	JSValue func = JS_GetPropertyStr(ctx, global_obj, name);
	JSValue vals[nb];
	JSValue  r;

	printf("call %s: ", name);
	for (int i = 0; i < nb; ++i) {
		printf(" %d(is e %d) - %p", types[i],
		       types[i] == YS_ENTITY, args[i].vptr);
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

	r = JS_Call(ctx, func, global_obj, nb, vals);
	if (JS_IsException(r)) {
		printf("EXEPTION !!!:\n");
		js_std_dump_error(ctx);
	} else if (JS_IsObject(r))
		return JS_GetOpaque(r, entity_class_id);
	else if (JS_IsNumber(r)) {
		int64_t ir;

		JS_ToInt64(ctx, &ir, r);
		return (void *)ir;
	} else if (JS_IsString(r)) {
		return (void *)JS_ToCString(ctx, r);
	}
	/*
	  * Basically... yeah this seems broken
	  * At last as broken as yirl, but it seems they talk in same
	  * boken dialect
	  */
	return JS_VALUE_GET_PTR(r);
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
	JS_Eval(ctx, str, l, filename, JS_EVAL_TYPE_GLOBAL);
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

static void *allocator(void)
{
	YScriptQjs *ret;

	ret = calloc(1, sizeof(*ret));
	ret->ops.init = init;
	ret->ops.destroy = destroy;
	ret->ops.loadFile = loadFile;
	ret->ops.loadString = loadString;
	ret->ops.call = call;
	return (void *)ret;
}

int ysQjsInit(void)
{
	t = ysRegister(allocator);
	JS_NewClassID(&entity_class_id);
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
