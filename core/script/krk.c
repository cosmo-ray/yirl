/*
**Copyright (C) 2020-2025 Matthias Gatto
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

#include <kuroko/kuroko.h>
#include <kuroko/vm.h>
#include <kuroko/util.h>
#include <kuroko/object.h>
#include <kuroko/value.h>

#ifdef likely
#undef likely
#endif
#ifdef unlikely
#undef unlikely
#endif
#include <yirl/all.h>

static int t = -1;
static void *cur_manager;

KrkClass *yent_krk_class;
KrkClass *yent_krk_str_class;
KrkClass *yent_krk_int_class;
KrkClass *yent_krk_float_class;
KrkClass *yent_krk_array_class;

struct YScriptKrk {
	YScriptOps ops;
	KrkInstance *module;
};

struct YKrkEntity {
	KrkInstance inst;
	Entity *e;
	int need_free;
};

#define IS_yent_krk_str_class(o) (krk_isInstanceOf(o,yent_krk_class))
#define AS_yent_krk_str_class(o) ((struct YKrkEntity*)AS_OBJECT(o))

#define IS_yent_krk_int_class(o) (krk_isInstanceOf(o,yent_krk_class))
#define AS_yent_krk_int_class(o) ((struct YKrkEntity*)AS_OBJECT(o))

#define IS_yent_krk_float_class(o) (krk_isInstanceOf(o,yent_krk_class))
#define AS_yent_krk_float_class(o) ((struct YKrkEntity*)AS_OBJECT(o))

#define IS_yent_krk_array_class(o) (krk_isInstanceOf(o,yent_krk_class))
#define AS_yent_krk_array_class(o) ((struct YKrkEntity*)AS_OBJECT(o))

#define IS_yent_krk_class(o) (krk_isInstanceOf(o,yent_krk_class))
#define AS_yent_krk_class(o) ((struct YKrkEntity*)AS_OBJECT(o))
#define CURRENT_CTYPE struct YKrkEntity *
#define CURRENT_NAME  self

static int loadString(void *s, const char *str);

#define GET_E(idx)				\
	AS_yent_krk_class(argv[idx])->e

#define GET_S(idx)				\
	AS_CSTRING(argv[idx])

#define GET_I(idx)				\
	AS_INTEGER(argv[idx])

#define GET_D(idx)				\
	AS_FLOATING(argv[idx])

#define BIND_AUTORET(call)						\
	int t = YSCRIPT_RET_TYPE(call, 1);				\
	switch (t) {							\
	case YSCRIPT_RET_VOID:						\
		call;							\
		return NONE_VAL();					\
	case YSCRIPT_RET_ENTITY:					\
	{								\
		Entity *ret = call;					\
		return make_ent(ret);					\
	}								\
	case YSCRIPT_RET_STR:						\
	{								\
		char *ret = ((char *)(intptr_t)YSCRIPT_VOID_CALL(call)); \
		return OBJECT_VAL(krk_copyString(ret, strlen(ret)));	\
	}								\
	case YSCRIPT_RET_INT:						\
	case YSCRIPT_RET_LONG:						\
		return INTEGER_VAL((intptr_t)YSCRIPT_VOID_CALL(call));	\
	case YSCRIPT_RET_UINT:						\
		return INTEGER_VAL((uintptr_t)YSCRIPT_VOID_CALL(call));	\
	case YSCRIPT_RET_BOOL:						\
		return BOOLEAN_VAL((_Bool)YSCRIPT_VOID_CALL(call));	\
	}

static KrkValue krkyePrint(int argc, const KrkValue argv[], int hasKw) {
	if (argc != 1) return krk_runtimeError(vm.exceptions->argumentError, "yePrint() expects exactly 1 argument, %d given", argc);
	Entity *e = GET_E(0);
	yePrint(e);
	return NONE_VAL();
}

static KrkValue make_ent(Entity *e) {
	struct YKrkEntity *ret;
	if (yeType(e) == YARRAY)
		ret = (struct YKrkEntity *)krk_newInstance(yent_krk_array_class);
	else
		ret = (struct YKrkEntity *)krk_newInstance(yent_krk_class);
	ret->e = e;
	ret->need_free = 0;
	return OBJECT_VAL(ret);
}

KRK_Method(yent_krk_class, __repr__) {
	char *r = yeToCStr(self->e, 3, YE_FORMAT_PRETTY);
	return OBJECT_VAL(krk_takeString(r, strlen(r)));
}


static void _ent_gcsweep(KrkInstance * self) {
	struct YKrkEntity *ke = (void *)self;
	if (ke->need_free) {
		printf("destroy ");
		yePrint(ke->e);
		yeDestroy(ke->e);
	}
	printf("delete entity !!!\n");
}

KRK_Method(yent_krk_class, __init__) {
	int have_int_ptr0 = 0;
	long long int int_ptr;
	printf("init entity\n");
	if (!krk_parseArgs(".|L?", (const char *[]){"int_ptr"},
			   &have_int_ptr0, &int_ptr))
		return NONE_VAL();

	if (have_int_ptr0) {
		self->e = (void *)int_ptr;
		yePrint(self->e);
	} else {
		self->e = NULL;
	}
	self->need_free = 0;
	return NONE_VAL();
}

KRK_Method(yent_krk_str_class, __init__) {
	const char *str, *name;
	struct YKrkEntity *mother;
	int have_mother, have_name;

	if (!krk_parseArgs(".s|O?s?", (const char *[]){"string", "parent", "name"},
			   &str, &have_mother, &mother, &have_name, &name))
		return NONE_VAL();

	self->e = yeCreateString(str, have_mother ? mother->e : NULL, have_name ? name : NULL);
	self->need_free = !have_mother;
	return NONE_VAL();
}

KRK_Method(yent_krk_int_class, __init__) {
	const char *name;
	int i;
	struct YKrkEntity *mother;
	int have_mother, have_name;

	if (!krk_parseArgs(".i|O?s?", (const char *[]){"int", "parent", "name"},
			   &i, &have_mother, &mother, &have_name, &name))
		return NONE_VAL();

	self->e = yeCreateInt(i, have_mother ? mother->e : NULL, have_name ? name : NULL);
	self->need_free = !have_mother;
	return NONE_VAL();
}

KRK_Method(yent_krk_float_class, __init__) {
	const char *name;
	double d;
	struct YKrkEntity *mother;
	int have_mother, have_name;

	if (!krk_parseArgs(".d|O?s?", (const char *[]){"float", "parent", "name"},
			   &d, &have_mother, &mother, &have_name, &name))
		return NONE_VAL();

	printf("f %f\n", d);
	self->e = yeCreateFloat(d, have_mother ? mother->e : NULL, have_name ? name : NULL);
	self->need_free = !have_mother;
	return NONE_VAL();
}

KRK_Method(yent_krk_array_class, __getitem__) {
	KrkValue v;
	Entity *eret;
	if (!krk_parseArgs(".V", (const char *[]){"key"}, &v))
		return NONE_VAL();
	if (IS_INTEGER(v))
		eret = yeGet(self->e, AS_INTEGER(v));
	else if (IS_FLOATING(v))
		eret = yeGet(self->e, (int)AS_FLOATING(v));
	else if (IS_STRING(v))
		eret = yeGet(self->e, AS_CSTRING(v));
	return make_ent(eret);
}

KRK_Method(yent_krk_array_class, __setitem__) {
	KrkValue key, val;
	Entity *eret;
	if (!krk_parseArgs(".VV", (const char *[]){"key"}, &key, &val))
		return NONE_VAL();
	if (IS_INTEGER(key)) {
		int k = AS_INTEGER(key);

		if (IS_INTEGER(val)) {
			yeCreateIntAt(AS_INTEGER(val), self->e, NULL, k);
		} else if (IS_yent_krk_class(val)) {
			yePushAt(self->e, AS_yent_krk_class(val)->e, k);
		} else if (IS_STRING(val)) {
			yeCreateStringAt(AS_CSTRING(val), self->e, NULL, k);
		} else if (IS_FLOATING(val)) {
			yeCreateFloatAt(AS_FLOATING(val), self->e, NULL, k);
		}
	} else if (IS_STRING(key)) {
		const char *k = AS_CSTRING(key);
		if (IS_INTEGER(val)) {
			yeCreateInt(AS_INTEGER(val), self->e, k);
		} else if (IS_yent_krk_class(val)) {
			yePushBack(self->e, AS_yent_krk_class(val)->e, k);
		} else if (IS_STRING(val)) {
			yeCreateString(AS_CSTRING(val), self->e, k);
		} else if (IS_FLOATING(val)) {
			yeCreateFloat(AS_FLOATING(val), self->e, k);
		}
		eret = yeGet(self->e, AS_CSTRING(key));
	}
	return NONE_VAL();
}

KRK_Method(yent_krk_array_class, __init__) {
	const char *name;
	struct YKrkEntity *mother;
	int have_mother, have_name, have_int_ptr;
	long long int int_ptr = 0;

	if (!krk_parseArgs(".|O?s?L?", (const char *[]){"parent", "name", "int_ptr"},
			   &have_mother, &mother, &have_name, &name,
			   &have_int_ptr, &int_ptr)) {
		return NONE_VAL();
	}

	if (have_int_ptr) {
		self->e = (void *)int_ptr;
		self->need_free = 0;
	} else {
		self->e = yeCreateArray(have_mother ? mother->e : NULL, have_name ? name : NULL);
		self->need_free = !have_mother;
	}
	return NONE_VAL();
}

static int init(void *sm, void *args)
{
	struct YScriptKrk *this = sm;
#define BIND(x, args...)					\
	krk_defineNative(&vm.builtins->fields, #x, krk##x);

	krk_initVM(0); // Initialize the VM with default flags
	BIND(yePrint);
	this->module = krk_startModule("__main__");
	yent_krk_class = krk_makeClass(this->module, &yent_krk_class, "Entity",
				       KRK_BASE_CLASS(object));
	yent_krk_class->allocSize = sizeof(struct YKrkEntity);
	yent_krk_class->_ongcsweep = _ent_gcsweep;;
	BIND_METHOD(yent_krk_class, __init__);
	BIND_METHOD(yent_krk_class, __repr__);
	krk_finalizeClass(yent_krk_class);

	yent_krk_str_class = krk_makeClass(this->module, &yent_krk_str_class, "StringEntity",
				       yent_krk_class);
	yent_krk_class->allocSize = sizeof(struct YKrkEntity);
	BIND_METHOD(yent_krk_str_class, __init__);
	krk_finalizeClass(yent_krk_str_class);


	yent_krk_int_class = krk_makeClass(this->module, &yent_krk_str_class, "IntEntity",
				       yent_krk_class);
	yent_krk_class->allocSize = sizeof(struct YKrkEntity);
	BIND_METHOD(yent_krk_int_class, __init__);
	krk_finalizeClass(yent_krk_int_class);

	yent_krk_float_class = krk_makeClass(this->module, &yent_krk_str_class, "FloatEntity",
				       yent_krk_class);
	yent_krk_class->allocSize = sizeof(struct YKrkEntity);
	BIND_METHOD(yent_krk_float_class, __init__);
	krk_finalizeClass(yent_krk_float_class);

	yent_krk_array_class = krk_makeClass(this->module, &yent_krk_str_class, "ArrayEntity",
					     yent_krk_class);
	yent_krk_class->allocSize = sizeof(struct YKrkEntity);
	BIND_METHOD(yent_krk_array_class, __init__);
	BIND_METHOD(yent_krk_array_class, __getitem__);
	BIND_METHOD(yent_krk_array_class, __setitem__);
	krk_finalizeClass(yent_krk_array_class);

	return 0;
}

// Helper function to prepare arguments on the stack
static int prepareArguments(int nb, union ycall_arg *args, int *types) {
	for (int i = 0; i < nb; i++) {
		switch (types[i]) {
		case YS_ENTITY:
		{
			KrkValue v = make_ent(args[i].e);
			krk_push(v);
			break;
		}
		case YS_INT:
			krk_push(INTEGER_VAL(args[i].i));
			break;
		case YS_FLOAT:
			krk_push(FLOATING_VAL(args[i].f));
			break;
		case YS_STR:
			krk_push(OBJECT_VAL(krk_copyString(args[i].str,
							   strlen(args[i].str))));
			break;
		case YS_VPTR:
			krk_push(INTEGER_VAL(args[i].vptr));
			break;
		default:
			printf("Error: Unsupported argument type %d\n", types[i]);
			return -1; // Return error for unsupported types
		}
	}
	return 0; // Success
}

static struct ys_ret ys_ret_from_krk_ret(KrkValue returnValue)
{
	struct ys_ret result = { .t = YS_VPTR, .v.vptr = NULL }; // Default return
	if (IS_NONE(returnValue)) {
		DPRINT_ERR("Error: Function execution failed\n");
		return result;
	}

	if (IS_INTEGER(returnValue)) {
		result.t = YS_INT;
		result.v.i = AS_INTEGER(returnValue);
	} else if (IS_yent_krk_class(returnValue)) {
		result.t = YS_ENTITY;
		result.v.e = AS_yent_krk_class(returnValue)->e;
	} else if (IS_FLOATING(returnValue)) {
		result.t = YS_FLOAT;
		result.v.f = AS_FLOATING(returnValue); // Cast to void *
	} else if (IS_STRING(returnValue)) {
		result.t = YS_STR;
		result.v.str = AS_CSTRING(returnValue);
	} else if (IS_NONE(returnValue)) {
		result.t = YS_VPTR;
		result.v.vptr = NULL;
	} else {
		printf("Error: Unsupported return type\n");
	}

	return result;
}

// Core function to handle calls (normal and fast)
static struct ys_ret krk_coreCall(KrkValue funcValue, int nb, union ycall_arg *args, int *types) {
	struct ys_ret result = { .t = YS_VPTR, .v.vptr = NULL }; // Default return

	// Push the function onto the stack
	krk_push(funcValue);

	// Push the arguments onto the stack
	if (prepareArguments(nb, args, types) < 0) {
		return result; // Return default result on error
	}

	// Call the function
	KrkValue returnValue = krk_callStack(nb);
	return ys_ret_from_krk_ret(returnValue);
}

// Implementation of call2
static struct ys_ret krk_call2(void *sm, const char *name, int nb, union ycall_arg *args, int *types) {
	yeAutoFree Entity *str = yeCreateString(name, NULL, NULL);

	yeStringAddCh(str, '(');
	for (int i = 0; i < nb; ++i) {
		if (i)
			yeStringAddCh(str, ',');
		switch (types[i]) {
		case YS_ENTITY: {
			if (yeType(args[i].e) == YARRAY)
				yeStringAdd(str, "ArrayEntity(int_ptr=");
			else
				yeStringAdd(str, "Entity(int_ptr=");
			yeStringAddI64(str, args[i].i);
			yeStringAddCh(str, ')');
		}
			break;
		case YS_INT:
		case YS_VPTR:
			yeStringAddI64(str, args[i].i);
			break;
		case YS_FLOAT:
			yeStringAddDouble(str, args[i].f);
			break;
		case YS_STR:
			yeStringAddCh(str, '\'');
			yeStringAdd(str, args[i].str);
			yeStringAddCh(str, '\'');
			break;
		default:
			yeStringAdd(str, "None");
		}
	}
	yeStringAddCh(str, ')');
	yePrint(str);
	KrkValue result = krk_interpret(yeGetString(str), "<stdin>");
}

// Implementation of call
static void *krk_call(void *sm, const char *name, int nb, union ycall_arg *args, int *types) {
	struct ys_ret result = krk_call2(sm, name, nb, args, types);
	return result.v.vptr;
}

// Implementation of fastCall2
static struct ys_ret krk_fastCall2(void *sm, void *opacFunction, int nb, union ycall_arg *args, int *types) {
	KrkValue funcValue = *(KrkValue *)opacFunction; // Use the provided function value directly
	return krk_coreCall(funcValue, nb, args, types);
}

// Implementation of fastCall
static void *krk_fastCall(void *sm, void *opacFunction, int nb, union ycall_arg *args, int *types) {
	struct ys_ret result = krk_fastCall2(sm, opacFunction, nb, args, types);
	return result.v.vptr;
}

static int loadString(void *s, const char *str)
{
	if (!str) {
		printf("Error: Script string is NULL.\n");
		return -1;
	}

	krk_interpret(str, "<stdin>");
	return 0;
}

static int loadFile(void *s, const char *fileName)
{
	if (!fileName) {
		printf("Error: File name is NULL.\n");
		return -1;
	}

	KrkValue result = krk_runfile(fileName, "<stdin>");
	if (IS_NONE(result)) {
		printf("Error: Failed to execute file '%s'.\n", fileName);
		return -1;
	}

	loadString(s, "print(kuroko.modules())\ntata(\"ho\")");
	printf("File '%s' executed successfully.\n", fileName);
	return 0;
}

static int destroy(void *sm)
{
	krk_freeVM(); // Free the VM resources
	((struct YScriptKrk *)sm)->module = NULL;
	printf("Kuroko VM destroyed.\n");
	return 0;
}

static void addFuncSymbole(void *sm, const char *name, int nbArgs, Entity *func)
{
	/* KrkValue global = krk_currentThread.globals->fields; */
	/* KrkValue funcValue = krk_pointer(func); // Wrap the Entity as a KrkValue */
	char *generatedFunction = NULL;

	// If no name is provided, use the function's name
	const char *funcName = name ? name : yeGetString(func);

	// Generate a unique name for the global function
	char *uniqueName = malloc(strlen(funcName) + 8);
	sprintf(uniqueName, "%sGlobal", funcName);

	// Add the function to the global namespace
	// TODO
	/* krk_push(funcValue); */
	/* krk_tableSet(&krk_currentThread.globals->fields, OBJECT_VAL(S(uniqueName)), krk_peek(0)); */
	/* krk_pop(); // Remove the value from the stack */

	// Use an Entity to dynamically build the function string
	Entity *strBuilder = yeCreateString("", NULL, NULL);

	// Construct the function definition
	yeAddStr(strBuilder, "def ");
	yeAddStr(strBuilder, funcName);
	yeAddStr(strBuilder, "(");

	for (int i = 0; i < nbArgs; i++) {
		if (i > 0) {
			yeAddStr(strBuilder, ", ");
		}
		yeAddStr(strBuilder, "arg");
		yeAddInt(strBuilder, i);
	}

	yeAddStr(strBuilder, "):\n    return yesCall(");
	yeAddStr(strBuilder, uniqueName);

	for (int i = 0; i < nbArgs; i++) {
		yeAddStr(strBuilder, ", arg");
		yeAddInt(strBuilder, i);
	}

	yeAddStr(strBuilder, ")\n");

	// Execute the dynamically constructed function string
	const char *finalFunction = yeGetString(strBuilder);
	KrkValue result = krk_interpret(finalFunction, "<addFuncSymbole>");
	if (IS_NONE(result)) {
		printf("Error: Failed to add function '%s' to Kuroko.\n", funcName);
	} else {
		printf("Function '%s' added successfully.\n", funcName);
	}

	// Clean up
	free(uniqueName);
	yeDestroy(strBuilder);
}

static void *allocator(void)
{
	struct YScriptKrk *ret;

	ret = calloc(1, sizeof(*ret));
	ret->ops.init = init;
	ret->ops.destroy = destroy;
	ret->ops.loadFile = loadFile;
	ret->ops.loadString = loadString;
	ret->ops.call = krk_call;
	ret->ops.call2 = krk_call2;
	ret->ops.fastCall = krk_fastCall;
        ret->ops.fastCall2 = krk_fastCall2;
	ret->ops.addFuncSymbole = addFuncSymbole;
	return (void *)ret;
}

int ysKrkInit(void)
{
	t = ysRegister(allocator);
	return t;
}

int ysKrkEnd(void)
{
	return ysUnregiste(t);
}

int ysKrkGetType(void)
{
	return t;
}
