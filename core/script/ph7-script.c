/*
**Copyright (C) 2022-2023 Matthias Gatto
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

#if PH7_ENABLE > 0

#include "ph7-script.h"
#include <yirl/all.h>
#include "game.h"
#include <stdlib.h>
#include <string.h>

static int t = -1;

#define PH7_MAX_VMS 128

#define RETURN_VOID 0
#define RETURN_TYPE_INT 1
#define RETURN_TYPE_ENTITY 2
#define RETURN_TYPE_WIDPTR 3

#define GC_STACK_LEN 64

static int gc_stack_i;
static Entity *gc_stack[GC_STACK_LEN];

static ph7_context *last_ctx;

static struct {
	int t;
	union {
		int64_t i;
		Entity *e;
		void *vptr;
	};
} call_ret = {};

struct vm_info {
	ph7_vm *pVm;
	int in_use;
	char *prog;
	unsigned int slowpath;
	/* uLong file_hash; */
};

typedef struct {
	YScriptOps ops;
	ph7 *pEngine; /* PH7 engine */
	struct vm_info vms[PH7_MAX_VMS]; /* Compiled PHP program */
	int nb_vms;
	Entity *preload_string;
	int cur_vm;
} YScriptPH7;

static YScriptPH7 *manager;

static int yph7_result_string(ph7_context *pCtx, const char *zString)
{
	return ph7_result_string(pCtx, zString, -1);
}

static int make_nothing(ph7_context *pCtx,  ...)
{
	abort();
}

#define Fatal(x...) do { printf(x); return NULL; } while (0);
#define Fatali(x...) do { printf(x); return -1; } while (0);

#define E_AT(a, idx) (argc > idx ? ph7_value_to_resource(a[idx]) : NULL)
#define I_AT(a, idx) (argc > idx ? ph7_value_to_int(a[idx]) : 0)
#define S_AT(a, idx) (argc > idx ? ph7_value_to_string(a[idx], NULL) : NULL)

#define PH7T(call)				\
	_Generic(call,				\
		 char *: 1,			\
		 const char *: 1,		\
		 Entity *: 1,			\
		 const Entity *: 1,		\
		 int: 1,			\
		 _Bool: 1,			\
		 long: 1,			\
		 long long int: 1,		\
		 double: 1,			\
		 float: 1,			\
		 unsigned long: 1,		\
		 unsigned int: 1,		\
		 default: 0)

#define PH7_RET(call, ...)						\
	int t = PH7T(call);						\
	last_ctx = pCtx;						\
	if (!t)	{							\
		_Pragma("GCC diagnostic ignored \"-Wunused-value\"");	\
		call;							\
		_Pragma("GCC diagnostic pop");				\
	} else								\
		S7A2(VOID_CALL(call), __VA_ARGS__);			\
	last_ctx = NULL;

#define VOID_CALL(call)				\
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

#define S7A2(call, ...)					\
	_Generic(call,					\
		 default: make_nothing,			\
		 char *: yph7_result_string,		\
		 const char *: yph7_result_string,	\
		 Entity *: ph7_result_resource,		\
		 int: ph7_result_int,			\
		 long: ph7_result_int64,		\
		 long long: ph7_result_int64,		\
		 _Bool: ph7_result_bool,		\
		 unsigned long: ph7_result_int64,	\
		 unsigned int: ph7_result_int64)	\
		(__VA_ARGS__, call)

#define BIND_EEIII(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1), I_AT(a, 2),		\
			  I_AT(a, 3), I_AT(a, 4)), pCtx);		\
		return 0;						\
	}

#define BIND_EIIIIS(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) {	\
		PH7_RET(f(E_AT(a, 0), I_AT(a, 1), I_AT(a, 2), I_AT(a, 3), \
			  I_AT(a, 4), S_AT(a, 5)), pCtx);		\
		return 0;						\
	}

#define BIND_EIIIISI(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) {	\
		PH7_RET(f(E_AT(a, 0), I_AT(a, 1), I_AT(a, 2), I_AT(a, 3), \
			  I_AT(a, 4), S_AT(a, 5), I_AT(a, 6)), pCtx);	\
		return 0;						\
	}

#define BIND_EIIIS(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) {	\
		PH7_RET(f(E_AT(a, 0), I_AT(a, 1), I_AT(a, 2), I_AT(a, 3), \
			  S_AT(a, 4)), pCtx);				\
		return 0;						\
	}

#define BIND_EIIISI(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) {	\
		PH7_RET(f(E_AT(a, 0), I_AT(a, 1), I_AT(a, 2), I_AT(a, 3), \
			  S_AT(a, 4), I_AT(a, 5)), pCtx);		\
		return 0;						\
	}

#define BIND_EIIEE(f, ...)			\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), I_AT(a, 1), I_AT(a, 2), E_AT(a, 3), \
			  E_AT(a, 4)), pCtx);				\
		return 0;						\
	}

#define BIND_EEES(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1), E_AT(a, 2), S_AT(a, 3)), pCtx); \
		return 0;						\
	}

#define BIND_EESS(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1), S_AT(a, 2), S_AT(a, 3)), pCtx); \
		return 0;						\
	}

#define BIND_IIES(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(I_AT(a, 0), I_AT(a, 1), E_AT(a, 2), S_AT(a, 3)), pCtx); \
		return 0;						\
	}

#define BIND_EEIS(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1), I_AT(a, 2), S_AT(a, 3)), pCtx); \
		return 0;						\
	}

#define BIND_EIES(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), I_AT(a, 1), E_AT(a, 2), S_AT(a, 3)), pCtx); \
		return 0;						\
	}


#define BIND_EEEE(f, ...)			\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1), E_AT(a, 2), E_AT(a, 3)), pCtx); \
		return 0;						\
	}

#define BIND_EEEI(f, ...)			\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1), E_AT(a, 2), I_AT(a, 3)), pCtx); \
		return 0;						\
	}

#define BIND_EEEEI(f, ...)			\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1),			\
			  E_AT(a, 2), E_AT(a, 3), I_AT(a, 4)), pCtx);	\
		return 0;						\
	}

#define BIND_EEEEE(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1),			\
			  E_AT(a, 2), E_AT(a, 3), E_AT(a, 4)), pCtx);	\
		return 0;						\
	}

#define BIND_EEESI(f, ...)			\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1),			\
			  E_AT(a, 2), S_AT(a, 3), I_AT(a, 4)), pCtx);	\
		return 0;						\
	}

#define BIND_EIIS(f, ...)			\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), I_AT(a, 1), I_AT(a, 2), S_AT(a, 3)), pCtx); \
		return 0;						\
	}

#define BIND_EIIE(f, ...)			\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), I_AT(a, 1), I_AT(a, 2), E_AT(a, 3)), pCtx); \
		return 0;						\
	}

#define BIND_EIII(f, ...)			\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), I_AT(a, 1), I_AT(a, 2), I_AT(a, 3)), pCtx); \
		return 0;						\
	}

#define BIND_EIIII(f, ...)			\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), I_AT(a, 1), I_AT(a, 2), I_AT(a, 3), I_AT(a, 4)), pCtx); \
		return 0;						\
	}

#define BIND_SEES(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(S_AT(a, 0), E_AT(a, 1), E_AT(a, 2), S_AT(a, 3)), pCtx); \
		return 0;						\
	}

#define BIND_EEE(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1), E_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_EES(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1), S_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_SES(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(S_AT(a, 0), E_AT(a, 1), S_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_SE(f, ...)							\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(S_AT(a, 0), E_AT(a, 1)), pCtx);		\
		return 0;						\
	}

#define BIND_ESE(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), S_AT(a, 1), E_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_ISE(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(I_AT(a, 0), S_AT(a, 1), E_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_IES(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(I_AT(a, 0), E_AT(a, 1), S_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_ISS(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(I_AT(a, 0), S_AT(a, 1), S_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_III(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(I_AT(a, 0), I_AT(a, 1), I_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_IIE(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(I_AT(a, 0), I_AT(a, 1),E_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_IIS(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(I_AT(a, 0), I_AT(a, 1),S_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_EEI(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1),I_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_EII(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), I_AT(a, 1),I_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_ESS(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), S_AT(a, 1),S_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_ESI(f, ...)						\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), S_AT(a, 1),I_AT(a, 2)), pCtx);	\
		return 0;						\
	}

#define BIND_SS(f, ...)							\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(S_AT(a, 0), S_AT(a, 1)), pCtx);		\
		return 0;						\
	}

#define BIND_SI(f, ...)							\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(S_AT(a, 0), I_AT(a, 1)), pCtx);		\
		return 0;						\
	}

#define BIND_EI(f, ...)							\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), I_AT(a, 1)), pCtx);		\
		return 0;						\
	}

#define BIND_ES(f, ...)							\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), S_AT(a, 1)), pCtx);		\
		return 0;						\
	}

#define BIND_II(f, ...)							\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(I_AT(a, 0), I_AT(a, 1)), pCtx);		\
		return 0;						\
	}

#define BIND_EE(f, ...)							\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0), E_AT(a, 1)), pCtx);		\
		return 0;						\
	}

#define BIND_E(f, ...)							\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(E_AT(a, 0)), pCtx);				\
		return 0;						\
	}

#define BIND_S(f, ...)							\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(S_AT(a, 0)), pCtx);				\
		return 0;						\
	}

#define BIND_I(f, ...)							\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) { \
		PH7_RET(f(I_AT(a, 0)), pCtx);				\
		return 0;						\
	}

#define BIND_V(f, ...)							\
	static int ph7##f(ph7_context *pCtx, int argc, ph7_value **a) {	\
		PH7_RET(f(), pCtx);					\
		return 0;						\
	}

BIND_E(yeDestroy);
BIND_EEIII(ywMapInitEntity);

#include "binding.c"

static int ph7ywPosCreate(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *ret;
	Entity *father = argc > 2 ? ph7_value_to_resource(argv[2]) :
		gc_stack[gc_stack_i - 1];
	const char *str = argc > 3 ? ph7_value_to_string(argv[3], NULL) : NULL;

	ret = ywPosCreate(ph7_value_to_int(argv[0]),
			  ph7_value_to_int(argv[1]), father, str);
	ph7_result_resource(pCtx, ret);
	return PH7_OK;
}

static int ph7ywSizeCreate(ph7_context *pCtx, int argc, ph7_value **argv)
{
	return ph7ywPosCreate(pCtx, argc, argv);
}

static int ph7yevCreateGrp(ph7_context *pCtx, int argc, ph7_value **argv)
{
	printf("NOT YET IMPLEMENTED");
	return -1;
}

static int ph7yeIncrAt(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *a = ph7_value_to_resource(argv[0]);
	ph7_value *k = argv[1];
	Entity *ent;

	if (ph7_value_is_int(k)) {
		ent = yeGet(a, ph7_value_to_int(k));
	} else if (ph7_value_is_string(k)) {
		ent = yeGet(a, ph7_value_to_string(k, NULL));
	} else {
		Fatali("Wrong type !");
	}
	yeAdd(ent, 1);
	return 0;
}

static int ph7yeSetIntAt(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *a = ph7_value_to_resource(argv[0]);
	ph7_value *k = argv[1];
	Entity *ent;

	if (ph7_value_is_int(k)) {
		ent = yeGet(a, ph7_value_to_int(k));
	} else if (ph7_value_is_string(k)) {
		ent = yeGet(a, ph7_value_to_string(k, NULL));
	} else {
		Fatali("Wrong type !");
	}
	yeSetInt(ent, I_AT(argv, 2));
	return 0;
}

static int ph7yeSetStringAt(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *a = ph7_value_to_resource(argv[0]);
	ph7_value *k = argv[1];
	Entity *ent;

	if (ph7_value_is_int(k)) {
		ent = yeGet(a, ph7_value_to_int(k));
	} else if (ph7_value_is_string(k)) {
		ent = yeGet(a, ph7_value_to_string(k, NULL));
	} else {
		Fatali("Wrong type !");
	}
	yeSetString(ent, S_AT(argv, 2));
	return 0;
}

static int ph7yeGet(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *a = ph7_value_to_resource(argv[0]);
	ph7_value *k = argv[1];
	Entity *ret;

	if (ph7_value_is_int(k)) {
		ret = yeGet(a, ph7_value_to_int(k));
	} else if (ph7_value_is_string(k)) {
		ret = yeGet(a, ph7_value_to_string(k, NULL));
	} else {
		Fatali("Wrong type !");
	}
	ph7_result_resource(pCtx, ret);
	return 0;
}

static int ph7yeGetIntAt(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *a = ph7_value_to_resource(argv[0]);
	ph7_value *k = argv[1];
	Entity *ret;

	if (ph7_value_is_int(k)) {
		ret = yeGet(a, ph7_value_to_int(k));
	} else if (ph7_value_is_string(k)) {
		ret = yeGet(a, ph7_value_to_string(k, NULL));
	} else {
		Fatali("Wrong type !");
	}
	ph7_result_int64(pCtx, yeGetInt(ret));
	return 0;
}

static int ph7yeGetStringAt(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *a = ph7_value_to_resource(argv[0]);
	ph7_value *k = argv[1];
	Entity *ret;

	if (ph7_value_is_int(k)) {
		ret = yeGet(a, ph7_value_to_int(k));
	} else if (ph7_value_is_string(k)) {
		ret = yeGet(a, ph7_value_to_string(k, NULL));
	} else {
		Fatali("Wrong type !");
	}
	yph7_result_string(pCtx, yeGetString(ret));
	return 0;
}

static int ph7yeAddAt(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *a = ph7_value_to_resource(argv[0]);
	ph7_value *k = argv[1];
	Entity *ent;

	if (ph7_value_is_int(k)) {
		ent = yeGet(a, ph7_value_to_int(k));
	} else if (ph7_value_is_string(k)) {
		ent = yeGet(a, ph7_value_to_string(k, NULL));
	} else {
		Fatali("Wrong type !");
	}
	yeAdd(ent, I_AT(argv, 2));
	return 0;
}

static int init(void *sm, void *args)
{
	YScriptPH7 *ph7sm = sm;
	int rc;

	rc = ph7_init(&ph7sm->pEngine);
	if(rc != PH7_OK) {
		/*
		 * If the supplied memory subsystem is so sick that we are unable
		 * to allocate a tiny chunk of memory, there is no much we can do here.
		 */
		DPRINT_ERR("Error while allocating a new PH7 engine instance");
		return -1;
	}
	ph7sm->preload_string = NULL;
	return 0;
}

#define CALL_STRING "\n"						\
	"if ($nb_args == 0)\n\tcall_user_func($function_to_call);\n"		\
	"else if ($nb_args == 1)\n\tcall_user_func($function_to_call, $arg_00);\n" \
	"else if ($nb_args == 2)\n"					\
	"\tcall_user_func($function_to_call, $arg_00, $arg_01);\n"		\
	"else if ($nb_args == 3)\n"					\
	"\tcall_user_func($function_to_call, $arg_00, $arg_01, $arg_02);\n"	\
	"else if ($nb_args == 4)\n"					\
	"\tcall_user_func($function_to_call, $arg_00, $arg_01, $arg_02, $arg_03);\n" \
	"else if ($nb_args == 5)\n"					\
	"\tcall_user_func($function_to_call, $arg_00, $arg_01, $arg_02,\n"	\
	"\t$arg_03, $arg_04);\n"						\
	"yclose_output();"						\
	"\n?>\n"							\

_Bool output_ok;

int yclose_output(ph7_context *pCtx, int argc, ph7_value **argv)
{
	output_ok = 0;
	return PH7_OK;
}

int yirl_return_wid(ph7_context *pCtx, int argc, ph7_value **argv)
{
	if (argc != 2) {
		Fatali("yirl_return_wid bad argument");
	}
	ph7_value *w = argv[0];
	ph7_value *s = argv[1];

	call_ret.t = RETURN_TYPE_WIDPTR;
	call_ret.vptr = ywidNewWidget(ph7_value_to_resource(w),
				      ph7_value_to_string(s, NULL));
	return PH7_OK;
}

int yirl_return(ph7_context *pCtx, int argc, ph7_value **argv)
{
	if (argc < 1) {
		return 0;
	}

	ph7_value *a = argv[0];

	if (ph7_value_is_int(a)) {
		call_ret.t = RETURN_TYPE_INT;
		call_ret.i = ph7_value_to_int(a);
	} else if (ph7_value_is_resource(a)) {
		call_ret.t = RETURN_TYPE_ENTITY;
		call_ret.e = ph7_value_to_resource(a);
	} else if (ph7_value_is_null(a)) {
		call_ret.t = RETURN_TYPE_INT;
		call_ret.vptr = 0;
	} else {
		DPRINT_ERR("UNKNOW RETURN TYPE !");
	}
	return PH7_OK;
}

int ph7yeCreateArray(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *ret;
	Entity *father = argc > 0 ? ph7_value_to_resource(argv[0]) :
		gc_stack[gc_stack_i - 1];
	const char *str = argc > 1 ? ph7_value_to_string(argv[1], NULL) : NULL;

	ret = yeCreateArray(father, str);
	ph7_result_resource(pCtx, ret);
	return PH7_OK;
}

int ph7yeReCreateArray(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *ret;
	Entity *father = argc > 0 ? ph7_value_to_resource(argv[0]) :
		gc_stack[gc_stack_i - 1];
	const char *str = argc > 1 ? ph7_value_to_string(argv[1], NULL) : NULL;
	Entity *child = argc > 2 ? ph7_value_to_resource(argv[2]) : NULL;

	ret = yeReCreateArray(father, str, child);
	ph7_result_resource(pCtx, ret);
	return PH7_OK;
}

int int_to_entity(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *e = (void *)(intptr_t)ph7_value_to_int64(argv[0]);
	ph7_result_resource(pCtx, e);
	return PH7_OK;
}

int ph7yeCreateString(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *ret;
	Entity *father = argc > 1 ? ph7_value_to_resource(argv[1]) :
		gc_stack[gc_stack_i - 1];
	const char *str = argc > 2 ? ph7_value_to_string(argv[2], NULL) : NULL;

	ret = yeCreateString(ph7_value_to_string(argv[0], NULL), father, str);
	ph7_result_resource(pCtx, ret);
	return PH7_OK;
}

int ph7yeCreateInt(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *ret;
	Entity *father = argc > 1 ? ph7_value_to_resource(argv[1]) :
		gc_stack[gc_stack_i - 1];
	const char *str = argc > 2 ? ph7_value_to_string(argv[2], NULL) : NULL;

	ret = yeCreateInt(ph7_value_to_int(argv[0]), father, str);
	ph7_result_resource(pCtx, ret);
	return PH7_OK;
}

int ph7yeCreateFunction(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *ret;
	Entity *father = argc > 1 ?
		ph7_value_to_resource(argv[1]) :
		gc_stack[gc_stack_i - 1];
	const char *str = argc > 2 ? ph7_value_to_string(argv[2], NULL) : NULL;

	ret = yeCreateFunction(ph7_value_to_string(argv[0], NULL),
			       ygPH7Manager(), father, str);
	ph7_result_resource(pCtx, ret);
	return PH7_OK;
}

int ph7yeCreateCopy(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *ret;
	Entity *father = argc > 1 ? ph7_value_to_resource(argv[1]) :
		gc_stack[gc_stack_i - 1];
	const char *str = argc > 2 ? ph7_value_to_string(argv[2], NULL) : NULL;

	ret = yeCreateCopy(ph7_value_to_resource(argv[0]), father, str);
	ph7_result_resource(pCtx, ret);
	return PH7_OK;
}

static int ph7yesCall(ph7_context *pCtx, int argc, ph7_value **a)
{
	union ycall_arg args[argc - 1];
	int types[argc - 1];
	void *r;
	int i = 1;

	for (; i < argc; ++i) {
		if (ph7_value_is_int(a[i])) {
			args[i - 1].i = I_AT(a, i);
			types[i - 1] = YS_INT;
		} else if (ph7_value_is_string(a[i])) {
			args[i - 1].str = S_AT(a, i);
			types[i - 1] = YS_STR;
		} else {
			args[i - 1].e = E_AT(a, i);
			types[i - 1] = YS_ENTITY;
			if (!args[i - 1].e) {
				break;
			}
		}
	}
	r = yesCallInt(E_AT(a, 0), i - 1, args, types);
	if (yeIsPtrAnEntity(r))
		ph7_result_resource(pCtx, r);
	else
		ph7_result_int64(pCtx, (intptr_t) r);
	return 0;
}

static int ph7ygScriptCall(ph7_context *pCtx, int argc, ph7_value **a)
{
	union ycall_arg args[argc - 2];
	int types[argc - 2];
	void *r;
	int i = 2;

	for (; i < argc; ++i) {
		if (ph7_value_is_int(a[i])) {
			args[i - 2].i = I_AT(a, i);
			types[i - 2] = YS_INT;
		} else if (ph7_value_is_string(a[i])) {
			args[i - 2].str = S_AT(a, i);
			types[i - 2] = YS_STR;
		} else {
			args[i - 2].e = E_AT(a, i);
			types[i - 2] = YS_ENTITY;
			if (!args[i - 2].e) {
				break;
			}
		}
	}
	r = ysCallInt(ygScriptManager(I_AT(a, 0)), S_AT(a, 1), i - 2, args, types);
	if (yeIsPtrAnEntity(r))
		ph7_result_resource(pCtx, r);
	else
		ph7_result_int64(pCtx, (intptr_t) r);
	return 0;
}

static int ph7ygFileToEnt(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *father = argc > 2 ? ph7_value_to_resource(argv[2]) :
		gc_stack[gc_stack_i - 1];
	const char *str = ph7_value_to_string(argv[1], NULL);
	Entity *r = ygFileToEnt(ph7_value_to_int(argv[0]), str, father);
	ph7_result_resource(pCtx, r);
	return 0;
}

static int ph7ywRectCreateInts(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *ret;
	Entity *father = argc > 4 ? ph7_value_to_resource(argv[4]) :
		gc_stack[gc_stack_i - 1];
	const char *str = argc > 5 ? ph7_value_to_string(argv[5], NULL) : NULL;

	ret = ywRectCreateInts(ph7_value_to_int(argv[0]),
			       ph7_value_to_int(argv[1]),
			       ph7_value_to_int(argv[2]),
			       ph7_value_to_int(argv[3]),
			       father, str);
	ph7_result_resource(pCtx, ret);
	return PH7_OK;
}

static int ph7ywCanvasNewImg(ph7_context *pCtx, int argc, ph7_value **argv)
{
	Entity *ret;

	ret = ywCanvasNewImg(ph7_value_to_resource(argv[0]),
			     ph7_value_to_int(argv[1]),
			     ph7_value_to_int(argv[2]),
			     ph7_value_to_string(argv[3], NULL),
			     ph7_value_to_resource(argv[4]));
	ph7_result_resource(pCtx, ret);
	return PH7_OK;
}

static int Output_Consumer(const void *pOutput, unsigned int nOutputLen,
			   void *pUserData /* Unused */)
{
	if (!output_ok)
		return PH7_OK;
#ifdef __WINNT__
	BOOL rc;
	rc = WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), pOutput, (DWORD)nOutputLen,0,0);
	if( !rc ){
		/* Abort processing */
		return PH7_ABORT;
	}
#else
	ssize_t nWr;
	nWr = write(STDOUT_FILENO,pOutput,nOutputLen);
	if( nWr < 0 ){
		/* Abort processing */
		return PH7_ABORT;
	}
#endif /* __WINT__ */
	/* All done,VM output was redirected to STDOUT */
	return PH7_OK;
}

static ph7_vm *loadProg(YScriptPH7 *ph7sm, char *prog, ph7_vm *vm)
{
	int rc;
	char *error_str = "(unknow)";

	rc = ph7_compile_v2(
		ph7sm->pEngine,  /* PH7 engine */
		prog, /* PHP test program */
		-1    /* Compute input length automatically*/,
		&vm,  /* OUT: Compiled PHP program */
		0     /* IN: Compile flags */
		);
	if(rc != PH7_OK) {
		DPRINT_ERR("Can't compile PH7(PHP) code: %s\n", error_str);
		return NULL;
	}

	rc = ph7_vm_config(vm,
			   PH7_VM_CONFIG_OUTPUT,
			   Output_Consumer,    /* Output Consumer callback */
			   0                   /* Callback private data */
		);
	if (rc != PH7_OK ) {
		Fatal("Error while installing the VM output consumer callback");
	}


#define BIND(name, other...) do {					\
		rc = ph7_create_function(vm, #name, ph7##name, 0);	\
	} while (0)

#define PUSH_I_GLOBAL_VAL(g, VAL) do {					\
		ph7_value *pv = ph7_new_scalar(vm);			\
		ph7_value_int(pv, VAL);					\
		rc = ph7_vm_config(vm, PH7_VM_CONFIG_CREATE_SUPER,	\
				   #g, pv);				\
		ph7_release_value(vm, pv);				\
	} while (0);

#define PUSH_I_GLOBAL(g) do {					\
	ph7_value *pv = ph7_new_scalar(vm);			\
	ph7_value_int(pv, g);					\
	rc = ph7_vm_config(vm, PH7_VM_CONFIG_CREATE_SUPER,	\
			   #g, pv);				\
	ph7_release_value(vm, pv);				\
	} while (0);

	PUSH_I_GLOBAL_VAL(YEVE_ACTION, ACTION);
	PUSH_I_GLOBAL_VAL(YEVE_NOTHANDLE, NOTHANDLE);
	PUSH_I_GLOBAL_VAL(Y_A_KEY, 'a');
	PUSH_I_GLOBAL_VAL(Y_B_KEY, 'b');
	PUSH_I_GLOBAL_VAL(Y_C_KEY, 'c');
	PUSH_I_GLOBAL_VAL(Y_D_KEY, 'd');
	PUSH_I_GLOBAL_VAL(Y_E_KEY, 'e');
	PUSH_I_GLOBAL_VAL(Y_F_KEY, 'f');
	PUSH_I_GLOBAL_VAL(Y_G_KEY, 'g');
	PUSH_I_GLOBAL_VAL(Y_H_KEY, 'h');
	PUSH_I_GLOBAL_VAL(Y_I_KEY, 'i');
	PUSH_I_GLOBAL_VAL(Y_J_KEY, 'j');
	PUSH_I_GLOBAL_VAL(Y_K_KEY, 'k');
	PUSH_I_GLOBAL_VAL(Y_L_KEY, 'l');
	PUSH_I_GLOBAL_VAL(Y_M_KEY, 'm');
	PUSH_I_GLOBAL_VAL(Y_N_KEY, 'n');
	PUSH_I_GLOBAL_VAL(Y_O_KEY, 'o');
	PUSH_I_GLOBAL_VAL(Y_P_KEY, 'p');
	PUSH_I_GLOBAL_VAL(Y_Q_KEY, 'q');
	PUSH_I_GLOBAL_VAL(Y_R_KEY, 'r');
	PUSH_I_GLOBAL_VAL(Y_S_KEY, 's');
	PUSH_I_GLOBAL_VAL(Y_T_KEY, 't');
	PUSH_I_GLOBAL_VAL(Y_U_KEY, 'u');
	PUSH_I_GLOBAL_VAL(Y_V_KEY, 'v');
	PUSH_I_GLOBAL_VAL(Y_W_KEY, 'w');
	PUSH_I_GLOBAL_VAL(Y_X_KEY, 'x');
	PUSH_I_GLOBAL_VAL(Y_Y_KEY, 'y');
	PUSH_I_GLOBAL_VAL(Y_Z_KEY, 'z');

#define IN_CALL

#include "binding.c"

	BIND(yeGetStringAt);
	BIND(yeCreateString);
	BIND(yeCreateInt);
	BIND(yeCreateFunction);
	BIND(yeCreateArray);
	BIND(yeReCreateArray);
	BIND(yeCreateCopy);
	BIND(yesCall);
	BIND(ygFileToEnt);
	BIND(ygScriptCall);
	BIND(ywRectCreateInts);
	BIND(ywCanvasNewImg);
	BIND(ywSizeCreate);
	BIND(yeSetStringAt);
	BIND(yeDestroy);
	BIND(ywMapInitEntity);

	rc = ph7_create_function(vm, "int_to_entity", int_to_entity, 0);
	if( rc != PH7_OK ) {
		Fatal("Error while registering foreign functions 'int_to_entity'");
	}

	rc = ph7_create_function(vm, "yclose_output", yclose_output, 0);
	if( rc != PH7_OK ) {
		Fatal("Error while registering foreign functions 'yclose_output'");
	}

	rc = ph7_create_function(vm, "yirl_return", yirl_return, 0);
	if( rc != PH7_OK ) {
		Fatal("Error while registering foreign functions 'yirl_return'");
	}

	rc = ph7_create_function(vm, "yirl_return_wid", yirl_return_wid, 0);
	if( rc != PH7_OK ) {
		Fatal("Error while registering foreign functions 'yirl_return_wid'");
	}
	return vm;
}

static int loadString_(void *sm, const char *str, _Bool do_crc)
{
	YScriptPH7 *ph7sm = sm;
	int len = strlen(str);
	Entity *preload_string = ph7sm->preload_string;
	int preload_string_l = preload_string ? yeLen(preload_string) : 0;
	char *prog = malloc(len + sizeof CALL_STRING + preload_string_l);
	char *end = NULL;
	char *error_str = NULL;

	if (!prog)
		Fatali("malloc fail");
	strcpy(prog, str);
	for (int i = len - 3; i > 0; --i) {
		if (!strncmp(&prog[i], "?>", 2)) {
			end = &prog[i];
			break;
		}
	}
	if (!end)
		Fatali("'?>' not found");
	if (preload_string)
		end = stpcpy(end, yeGetString(preload_string));
	memcpy(end, CALL_STRING, sizeof CALL_STRING);

	/* printf("%s\n", prog); */
	/* Extract error log */
	ph7_config(ph7sm->pEngine,
		   PH7_CONFIG_ERR_LOG,
		   &error_str,
		   NULL
		);

	ph7sm->vms[ph7sm->nb_vms].in_use = 0;
        /* Compile the PHP test program defined above */
	ph7sm->vms[ph7sm->nb_vms].pVm = loadProg(ph7sm, prog,
						 ph7sm->vms[ph7sm->nb_vms].pVm);
	if (!loadProg(ph7sm, prog, ph7sm->vms[ph7sm->nb_vms].pVm)) {
		return -1;
	}

	ph7sm->cur_vm = ph7sm->nb_vms;
	ph7sm->vms[ph7sm->nb_vms].slowpath = 0;
	ph7sm->vms[ph7sm->nb_vms].prog = prog;
	/* if (do_crc) { */
	/* 	ph7sm->vms[ph7sm->cur_vm].file_hash = */
	/* 		crc32(0, (const Bytef *)prog, strlen(prog)); */
	/* } */
	/* printf("PROG: %s - %ld\n", prog, crc32(0, (const Bytef *)prog, strlen(prog))); */

	ph7sm->nb_vms++;
	return 0;
}

static int loadString(void *sm, const char *str)
{
	return loadString_(sm, str, 1);
}

static int loadFile(void *s, const char *file)
{
	int ret;
	yeAutoFree Entity *f = ygFileToEnt(YRAW_FILE, file, NULL);
	ret = loadString_(s, yeGetString(f), 0);
	/* if (!ret) { */
	/* 	manager->vms[manager->cur_vm].file_hash = */
	/* 		crc32(0, (const Bytef *)file, strlen(file)); */
	/* } */
	return ret;
}

static int destroy(void *sm)
{
	YScriptPH7 *ph7sm = sm;

	manager = NULL;
	for (int i = 0; i < ph7sm->nb_vms; ++i) {
		free(ph7sm->vms[i].prog);
	}
	free((YScriptPH7 *)sm);
	return 0;
}

static void *getFastPath(void *scriptManager, const char *name)
{
	return (void *)(intptr_t)manager->cur_vm;
}


static void addFuncSymbole(void *sm, const char *name, int nbArgs, Entity *func)
{
	YScriptPH7 *ph7sm = sm;
	Entity *e = ph7sm->preload_string;

	if (!e) {
		ph7sm->preload_string = yeCreateString("", NULL, NULL);
		e = ph7sm->preload_string;
	}
	yeAdd(e, "\nfunction ");
	yeAdd(e, name);
	yeAdd(e, "(");
	for (int  i = 0; i < nbArgs; ++i) {
		char a[] = {'$', 'a', '0' + i / 10, '0' + i % 10, 0};

		if (i)
			yeAdd(e, ", ");
		yeAdd(e, a);
	}
	yeAdd(e, ") {\n");
	yeAdd(e, "    return yesCall(int_to_entity(");
	yeAddLong(e, (int_ptr_t)func);
	yeAdd(e, ")");

	for (int i = 0; i < nbArgs; ++i) {
		char a[] = {'$', 'a', '0' + i / 10, '0' + i % 10, 0};

		yeAdd(e, ", ");
		yeAdd(e, a);
	}
	yeAdd(e, ");\n}\n");
}

static void *call_(void *sm, const char *name, int nb, union ycall_arg *args,
		   int *t_array)
{
	YScriptPH7 *ph7sm = sm;
	ph7_vm *vm = ph7sm->vms[ph7sm->cur_vm].pVm;
	int in_use = ph7sm->vms[ph7sm->cur_vm].in_use;
	ph7_vm *tmp = NULL;
	int rc;

	if (in_use) {
		vm = loadProg(ph7sm, ph7sm->vms[ph7sm->cur_vm].prog, tmp);
		ph7sm->vms[ph7sm->cur_vm].slowpath++;
	} else {
		ph7sm->vms[ph7sm->cur_vm].in_use = 1;
	}

	ph7_value *pv = ph7_new_scalar(vm);
	ph7_value_string(pv, name, -1);
	rc = ph7_vm_config(vm, PH7_VM_CONFIG_CREATE_VAR,
			   "function_to_call", pv);

	pv = ph7_new_scalar(vm);
	ph7_value_int(pv, nb);
	rc = ph7_vm_config(vm, PH7_VM_CONFIG_CREATE_VAR,
			   "nb_args", pv);
	ph7_release_value(vm, pv);
	if (unlikely(rc != PH7_OK)) {
		Fatal("Error while installing the VM output consumer callback\n");
	}


	for (int i = 0; i < nb; ++i) {
		int arg_type = t_array[i];
		const char *arg_name = "outbound";

		/* it seems PH7 doesn't copy arg_name, */
		/* so I have to keep each pointed data unique */
		if (i == 0)
			arg_name = "arg_00";
		else if (i == 1)
			arg_name = "arg_01";
		else if (i == 2)
			arg_name = "arg_02";
		else if (i == 3)
			arg_name = "arg_03";
		else if (i == 4)
			arg_name = "arg_04";
		pv = ph7_new_scalar(vm);

		if (arg_type == YS_INT)
			rc = ph7_value_int(pv, args[i].i);
		else if (arg_type == YS_STR)
			rc = ph7_value_string(pv, args[i].str, -1);
		else if (arg_type == YS_VPTR || arg_type == YS_ENTITY)
			rc = ph7_value_resource(pv, args[i].vptr);

		if (unlikely(rc != PH7_OK)) {
			Fatal("Error while storing value\n");
		}
		rc = ph7_vm_config(vm, PH7_VM_CONFIG_CREATE_VAR,
				   arg_name, pv);
		ph7_release_value(vm, pv);
		if (unlikely(rc != PH7_OK)) {
			Fatal("Error while installing the VM argumtens callback\n");
		}
	}

	gc_stack[gc_stack_i++] = yeCreateArray(NULL, NULL);
	output_ok = 1;

	rc = ph7_vm_exec(vm, 0);
	if (rc != PH7_OK) {
		Fatal("Error trying to exec ph7 %d (stack id %d)\n", rc, gc_stack_i--);
	}

	if (!in_use) {
		ph7_vm_reset(vm);
		ph7sm->vms[ph7sm->cur_vm].in_use = 0;
	} else {
		ph7_vm_release(vm);
	}

	yeDestroy(gc_stack[--gc_stack_i]);
	return call_ret.vptr;
}

static void *call(void *sm, const char *name, int nb, union ycall_arg *args,
		  int *t_array)
{
	if (!strcmp(name, "mod_init") || !strcmp(name, "file_init"))
		return call_(sm, name, nb, args, t_array);
	Fatal("Ph7 handle call only from entity, and for file/mod init\n");
	return NULL;
}

static void *e_call(void *sm, Entity *fe, int nb, union ycall_arg *args,
		    int *t_array)
{
	FunctionEntity *f = (void *)fe;

	manager->cur_vm = f->idata;
	return call_(sm, yeGetFunction(fe), nb, args, t_array);
}

static void trace(void *sm)
{
	printf("ph7 TRACE !!!!!!!!!\n");
	if (last_ctx)
		ph7_context_print_backtrace(last_ctx);
	else
		fprintf(stderr, "ph7 trow: NO CONTEXT !!!!\n");
}

static void *allocator(void)
{
	YScriptPH7 *ret;

	if (manager) {
		DPRINT_ERR("Can''t create more than 1 ph7 manager\n");
		return NULL;
	}

	ret = calloc(1, sizeof *ret);
	if (ret == NULL)
		return NULL;
	ret->ops.init = init;
	ret->ops.destroy = destroy;
	ret->ops.loadFile = loadFile;
	ret->ops.loadString = loadString;
	ret->ops.call = call;
	ret->ops.e_call = e_call;
	ret->ops.getFastPath = getFastPath;
	ret->ops.getError = NULL;
	ret->ops.trace = trace;
	ret->ops.registreFunc = NULL;
	ret->ops.addFuncSymbole = addFuncSymbole;
	manager = ret;
	return (void *)ret;
}

int ysPH7Init(void)
{
	t = ysRegister(allocator);
	return t;
}

int ysPH7End(void)
{
	return ysUnregiste(t);
}

int ysPH7GetType(void)
{
	return t;
}

#endif /* PH7_ENABLE */
