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

static int t = -1;

#define OPS(s) ((YScriptQjs *)s)->ops
#define RUNTIME(s) ((YScriptQjs *)s)->runtime
#define CTX(s) ((YScriptQjs *)s)->ctx


static JSClassID widget_class_id;
static JSClassID entity_class_id;
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

#define BIND_AUTORET(call)				\
	int t = LUAT(call);				\
	switch (t) {					\
	case 0:						\
		call;					\
		return JS_NULL;				\
	case 1:						\
		return AUTOPUSH(VOID_CALL(call), ctx);	\
	}						\
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

#define BIND_EEEEI(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_E(ctx, 2),		\
			       GET_E(ctx, 3), GET_I(ctx, 4)));		\
	}

#define BIND_EEESI(f, useless...)					\
	static JSValue qjs##f(JSContext *ctx, JSValueConst this_val,	\
			      int argc, JSValueConst *argv) {		\
		BIND_AUTORET(f(GET_E(ctx, 0),				\
			       GET_E(ctx, 1), GET_E(ctx, 2),		\
			       GET_S(ctx, 3), GET_I(ctx, 4)));		\
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

static void destroy_entity(JSRuntime *rt, JSValue val)
{
	Entity *e = JS_GetOpaque(val, freeable_entity_class_id);
	yeDestroy(e);
}


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
DUMB_FUNC(yeIncrAt);
DUMB_FUNC(yeAddAt);
BIND_ED(ywCanvasRotate);
BIND_EII(ywCanvasObjSetPos, 3, 0);
BIND_EIIE(ywCanvasNewText, 2, 2);

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
	return JS_Call(ctx, func, global_obj, argc - 1, argv + 1);
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
	return new_ent(ctx, yesCallInt(f, nb, yargs, types));
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

static JSValue qjsyeCreateCopy(JSContext *ctx, JSValueConst this_val,
			       int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeCreateCopy(GET_E(ctx, 0),
					GET_E(ctx, 1),
					GET_S(ctx, 2)),
		      !GET_E(ctx, 1));
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
		      !GET_E(ctx, 1));
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
	return mk_ent(ctx, ywPosCreate(GET_I(ctx, 0), GET_I(ctx, 1),
				       GET_E(ctx, 2), GET_S(ctx, 3)),
		      !GET_E(ctx, 2));
}

static JSValue qjsyeTryCreateInt(JSContext *ctx, JSValueConst this_val,
				 int argc, JSValueConst *argv)
{
	return mk_ent(ctx, yeTryCreateInt(GET_I(ctx, 0), GET_E(ctx, 1),
					  GET_S(ctx, 2)), !GET_E(ctx, 1));
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

	BIND(ywRectCreateInts, 6, 0);
	BIND(yeCreateFunction, 1, 2);
	BIND(yeCreateString, 1, 2);
	BIND(yeCreateInt, 1, 2);
	BIND(yeCreateArray, 0, 2);
	BIND(yeCreateCopy, 0, 3);
	BIND(yeReCreateArray, 2, 1);
	BIND(yeGetStringAt, 0, 2);
	BIND(ygLoadScript, 3, 0);
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

#define IN_CALL 1
	#include "binding.c"
#undef IN_CALL
	loadString(sm, "function ywSizeCreate(a,b,c,d) {"
		   "return ywPosCreate(a,b,c,d)"
		   "}");

	return 0;
}

static void *_call(void *sm, int nb, union ycall_arg *args,
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

	r = JS_Call(ctx, func, global_obj, nb, vals);
	if (JS_IsException(r)) {
		js_std_dump_error(ctx);
	} else if (JS_IsObject(r)) {
		void *e = GET_E_(r);
		if (likely(e))
			return e;
		e = JS_GetOpaque(r, widget_class_id);
		if (e)
			return e;
		e = JS_GetOpaque(r, script_manager_class_id);
		if (e)
			return e;
	} else if (JS_IsNumber(r)) {
		int64_t ir;

		JS_ToInt64(ctx, &ir, r);
		return (void *)ir;
	} else if (JS_IsString(r)) {
		return (void *)JS_ToCString(ctx, r);
	}
	return NULL;
}

static void *fCall(void *sm, void *sym, int nb,
		   union ycall_arg *args, int *t_arrray)
{
	return _call(sm, nb, args, t_arrray, functions[(intptr_t)sym - 1]);
}

static void *call(void *sm, const char *name, int nb, union ycall_arg *args,
		  int *types)
{
	JSContext *ctx = CTX(sm);
	JSValueConst global_obj = JS_GetGlobalObject(ctx);
	JSValue func = JS_GetPropertyStr(ctx, global_obj, name);

	return _call(sm, nb, args, types, func);
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
	ret->ops.trace = NULL;
	ret->ops.fastCall = fCall;
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
