
/*
**Copyright (C) 2015 Matthias Gatto
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

#ifndef _YIRL_SCRIPT_H_
#define _YIRL_SCRIPT_H_

#include <stdarg.h>
#include "entity.h"
#include "utils.h"

enum ys_type {
	YS_VPTR,
	YS_ENTITY,
	YS_STR,
	YS_FLOAT,
	YS_INT
};

union ycall_arg {
	void *vptr;
	Entity *e;
	intptr_t i;
	double f;
	const char *str;
};

struct ys_ret {
	enum ys_type t;
	union ycall_arg v;
};

typedef struct {
	int (*init)(void *opac, void *args);
	int (*loadFile)(void *opac, const char *fileName);
	int (*loadString)(void *opac, const char *str);
	int (* registreFunc)(void *opac, const char *name, void *arg);
	void (*addFuncSymbole)(void *sm, const char *name, int nbArgs,
			       Entity *func);
	/* call e_call and fastCall use the old convention
	 * that return a void *, problem is that with it
	 * I don't what type is return, but easy to implement
	 * and implement by all scripting language
	 */
	void *(*call)(void *sm, const char *name, int nb,
		      union ycall_arg *args, int *types);
	struct ys_ret (*call2)(void *sm, const char *name, int nb,
		      union ycall_arg *args, int *types);
	void *(*e_call)(void *sm, Entity *e, int nb,
		      union ycall_arg *args, int *types);
	void *(*fastCall)(void *opac, void *opacFunction,
			  int nb, union ycall_arg *args,
			  int *types);
	struct ys_ret (*fastCall2)(void *opac, void *opacFunction,
				   int nb, union ycall_arg *args,
				   int *types);
	void *(*getFastPath)(void *scriptManager, const char *name);
	void (*e_destroy)(void *manager, Entity *e);
	int (*addDefine)(void *opac, const char *name, const char *val);
	const char *(*getError)(void *opac);
	int (*destroy)(void *opac);
	void (*trace)(void *);
} YScriptOps;

YManagerAllocator *ysScriptsTab(void);

void *ysCallInt(void *sm, const char *name, int nb, union ycall_arg *args,
		int *types);

struct ys_ret ysCall2Int(void *sm, const char *name, int nb, union ycall_arg *args,
			 int *types);

#define YS_MK(x) {(void *)(intptr_t)x}

#define YS_MK_BRACES_1(a) YS_MK(a)
#define YS_MK_BRACES_2(a,b) YS_MK(a),YS_MK(b)
#define YS_MK_BRACES_3(a,b,c) YS_MK(a),YS_MK(b),YS_MK(c)
#define YS_MK_BRACES_4(a,b,c,d) YS_MK(a),YS_MK(b),YS_MK(c),YS_MK(d)
#define YS_MK_BRACES_5(a,b,c,d,e) YS_MK(a),YS_MK(b),YS_MK(c),YS_MK(d),YS_MK(e)
#define YS_MK_BRACES_6(a,b,c,d,e,f) YS_MK(a),YS_MK(b),YS_MK(c),YS_MK(d),\
		YS_MK(e),YS_MK(f)
#define YS_MK_BRACES_7(a,b,c,d,e,f,g) YS_MK(a),YS_MK(b),YS_MK(c),YS_MK(d),\
		YS_MK(e),YS_MK(f),YS_MK(g)
#define YS_MK_BRACES_8(a,b,c,d,e,f,g,h) YS_MK(a),YS_MK(b),YS_MK(c),YS_MK(d),\
		YS_MK(e),YS_MK(f),YS_MK(g),YS_MK(h)
#define YS_MK_BRACES_9(a,b,c,d,e,f,g,h,i) YS_MK(a),YS_MK(b),YS_MK(c),YS_MK(d),\
		YS_MK(e),YS_MK(f),YS_MK(g),YS_MK(h),YS_MK(i)

#define YS_MK_BRACES(nb, ...)				\
	YUI_CAT(YS_MK_BRACES_, nb) (__VA_ARGS__)

#define YS_ARGS(args...)			\
	YS_ARGS_(YUI_GET_ARG_COUNT(args), args)

#define YS_ARGS_(nb, args...)					\
	YUI_IF_ELSE(nb)						\
		((union ycall_arg []) {YS_MK_BRACES(nb, args)})	\
		(NULL)

#define YS_MK_TYPES_1(a)				\
	_Generic(a, Y_GENERIC_NUMBER(YS_INT),		\
		 Entity * : YS_ENTITY,			\
		 char * : YS_STR,			\
		 const char * : YS_STR,			\
		 default: YS_VPTR			\
		)

#define YS_MK_TYPES_2(a, b) YS_MK_TYPES_1(a), YS_MK_TYPES_1(b)
#define YS_MK_TYPES_3(a, b, c) YS_MK_TYPES_2(a, b), YS_MK_TYPES_1(c)
#define YS_MK_TYPES_4(a, b, c, d) YS_MK_TYPES_3(a, b, c), YS_MK_TYPES_1(d)
#define YS_MK_TYPES_5(a, b, c, d, e) YS_MK_TYPES_4(a, b, c, d), YS_MK_TYPES_1(e)

#define YS_MK_TYPES_6(a, b, c, d, e, f)			\
	YS_MK_TYPES_5(a, b, c, d, e), YS_MK_TYPES_1(f)

#define YS_MK_TYPES_7(a, b, c, d, e, f, g)			\
	YS_MK_TYPES_6(a, b, c, d, e, f), YS_MK_TYPES_1(g)

#define YS_MK_TYPES(nb, ...)			\
	YUI_CAT(YS_MK_TYPES_, nb) (__VA_ARGS__)

#define YS_ATYPES(args...)			\
	YS_ATYPES_(YUI_GET_ARG_COUNT(args), args)

#define YS_ATYPES_(nb, args...)				\
	YUI_IF_ELSE(nb)					\
		((int []) {YS_MK_TYPES(nb, args)}) (NULL)

#define ysCall(sm, name, args...)					\
	ysCallInt(sm, name,						\
		  YUI_GET_ARG_COUNT(args),				\
		  YS_ARGS(args),					\
		  YS_ATYPES(args))

#define ysFCall(sm, name, args...)					\
	ysFastCall(sm, name,						\
		   YUI_GET_ARG_COUNT(args),				\
		   YS_ARGS(args),					\
		   YS_ATYPES(args))


static inline void ysEDestroy(void *sm, Entity *f)
{
	if (unlikely(!sm) || likely(!((YScriptOps *)sm)->e_destroy))
		return;

	((YScriptOps *)sm)->e_destroy(sm, f);
}

static inline int ysAddDefine(void *sm, const char *name, const char *val)
{
	if (unlikely(!((YScriptOps *)sm)->addDefine))
		return -1;
	return ((YScriptOps *)sm)->addDefine(sm, name, val);
}

static inline void *ysGetFastPath(void *sm, const char *name)
{
	if (!((YScriptOps *)sm)->getFastPath)
		return NULL;
	return ((YScriptOps *)sm)->getFastPath(sm, name);
}

static inline _Bool ysHasEntityCall(void *sm)
{
	return !!((YScriptOps *)sm)->e_call;
}

void *ysEntityCall(void *sm, Entity *e, int nb,
		   union ycall_arg *args, int *types);

void *ysFastCall(void *sm, void *opacFunc, int nb,
		 union ycall_arg *args, int *types);

struct ys_ret ysFastCall2(void *sm, void *opacFunc, int nb,
			  union ycall_arg *args, int *types);

static inline void ysAddFuncSymbole(void *sm, const char *name,
				    int nbArgs, Entity *func)
{
	return ((YScriptOps *)sm)->addFuncSymbole(sm, name, nbArgs, func);
}

static inline int ysRegistreFunc(void *sm, const char *name, void *arg)
{
	return ((YScriptOps *)sm)->registreFunc(sm, name, arg);
}

static inline int ysLoadFile(void *sm, const char *name)
{
	return ((YScriptOps *)sm)->loadFile(sm, name);
}

static inline int ysLoadString(void *sm, const char *name)
{
	return ((YScriptOps *)sm)->loadString(sm, name);
}

static inline const char *ysGetError(void *sm)
{
	if (!sm)
		return "script manager is NULL";
	if (!((YScriptOps *)sm)->getError)
		return "(nil)";
	return ((YScriptOps *)sm)->getError(sm);
}

int ysRegister(void *(*allocator)(void));
int ysUnregiste(int t);

/**
 * @args the arguments
 * @type the type of script
 */
void *ysNewManager(void *args, int type);

/**
 * @scr the opaque scriptionManager object 
 */
int ysDestroyManager(void *sm);

void ysTraceCurrentScript(void);

/**
 * @return symbol of object name
 */
void *ysTccGetSym(void *vstate, const char *name);

int ysTccPushSym(void *state, const char *name, void *sym);

int ysTccPushSysincludePath(void *state, const char *path);

#define YSCRIPT_RET_OTHER YSCRIPT_RET_STR
enum {
	YSCRIPT_RET_VOID,   /* 0 */
	YSCRIPT_RET_ENTITY, /* 1 */
	YSCRIPT_RET_STR,    /* 2 */
	YSCRIPT_RET_INT,    /* 3 */
	YSCRIPT_RET_UINT,   /* 4 */
	YSCRIPT_RET_LONG,   /* 5 */
	YSCRIPT_RET_BOOL    /* 6 */
};



#define YS_GET_THIS(type) YUI_CAT(GET_, type)

#define YS_GETTER_NB_1(a)			\
	YS_GET_THIS(a)(0)

#define YS_GETTER_NB_2(a, b)			\
	YS_GET_THIS(a)(0), YS_GET_THIS(b)(1)

#define YS_GETTER_NB_3(a, b, c)			\
	YS_GET_THIS(a)(0), YS_GET_THIS(b)(1), YS_GET_THIS(c)(2)

#define YS_GETTER_NB_4(a, b, c, d)		\
	YS_GET_THIS(a)(0), YS_GET_THIS(b)(1), YS_GET_THIS(c)(2), YS_GET_THIS(d)(3)

#define YS_GETTER_NB_5(a, b, c, d, e)		\
	YS_GET_THIS(a)(0), YS_GET_THIS(b)(1), YS_GET_THIS(c)(2), \
		YS_GET_THIS(d)(3), YS_GET_THIS(e)(4)

#define YS_GETTER_NB_6(a, b, c, d, e, f)	\
	YS_GET_THIS(a)(0), YS_GET_THIS(b)(1), YS_GET_THIS(c)(2), \
		YS_GET_THIS(d)(3), YS_GET_THIS(e)(4), YS_GET_THIS(f)(5)

#define YS_GETTER_NB_7(a, b, c, d, e, f, g)			 \
	YS_GET_THIS(a)(0), YS_GET_THIS(b)(1), YS_GET_THIS(c)(2), \
		YS_GET_THIS(d)(3), YS_GET_THIS(e)(4), YS_GET_THIS(f)(5), \
		YS_GET_THIS(g)(6)

#define YS_GETTER_LST_(nb, lst...)		\
	YUI_CAT(YS_GETTER_NB_, nb)(lst)

#define YS_GETTER_LST(lst...)					\
	YS_GETTER_LST_(YUI_GET_ARG_COUNT(lst), lst)

#define YSCRIPT_RET_TYPE(call, multy)			\
	_Generic(call,					\
		 default: 0,				\
		 char *: 2,				\
		 const char *: 2,			\
		 Entity *: 1,				\
		 const Entity *: 1,			\
		 int: 2 + 1 * multy,			\
		 _Bool: 2 + 4 * multy,			\
		 long: 2 + 1 * multy,			\
		 long long int: 2 + 1 * multy,		\
		 double: 2 + 2 * multy,			\
		 float: 2 + 2 * multy,			\
		 unsigned long: 2 + 3 * multy,		\
		 unsigned int: 2 + 3 * multy)

#define YSCRIPT_VOID_CALL(call)			\
	_Generic(call,				\
		 default: NULL,			\
		 char *: call,			\
		 const char *: call,		\
		 Entity *: call,		\
		 const Entity *: call,		\
		 int: call,			\
		 _Bool: call,			\
		 long long: call,		\
		 long: call,			\
		 double: call,			\
		 float: call,			\
		 unsigned long: call,		\
		 unsigned int: call)



#endif
