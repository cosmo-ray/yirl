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
#ifdef likely
#undef likely
#endif
#ifdef unlikely
#undef unlikely
#endif
#include <yirl/all.h>

static int t = -1;
static void *cur_manager;

struct YScriptKrk {
	YScriptOps ops;
};

static int init(void *sm, void *args)
{
    krk_initVM(0); // Initialize the VM with default flags
    printf("Kuroko VM initialized.\n");
    return 0;
}

// Helper function to prepare arguments on the stack
static int krk_prepareArguments(int nb, union ycall_arg *args, int *types) {
    for (int i = 0; i < nb; i++) {
        switch (types[i]) {
        case YS_INT:
            krk_push(krk_int(args[i].i));
            break;
        case YS_FLOAT:
            krk_push(krk_float(args[i].f));
            break;
        case YS_STR:
            krk_push(krk_string(args[i].str));
            break;
        case YS_VPTR:
            krk_push(krk_pointer(args[i].vptr));
            break;
        default:
            printf("Error: Unsupported argument type %d\n", types[i]);
            return -1; // Return error for unsupported types
        }
    }
    return 0; // Success
}

// Core function to handle calls (normal and fast)
static struct ys_ret krk_coreCall(KrkValue funcValue, int nb, union ycall_arg *args, int *types) {
    struct ys_ret result = { .t = YS_VPTR, .v.vptr = NULL }; // Default return

    // Push the function onto the stack
    krk_push(funcValue);

    // Push the arguments onto the stack
    if (krk_prepareArguments(nb, args, types) < 0) {
        return result; // Return default result on error
    }

    // Call the function
    KrkValue returnValue = krk_callStack(nb);
    if (IS_NONE(returnValue)) {
        printf("Error: Function execution failed\n");
        return result;
    }

    // Determine the return type and populate the result
    if (IS_INTEGER(returnValue)) {
        result.t = YS_INT;
        result.v.i = AS_INTEGER(returnValue);
    } else if (IS_FLOATING(returnValue)) {
        result.t = YS_FLOAT;
        result.v.vptr = (void *)&AS_FLOATING(returnValue); // Cast to void *
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

// Implementation of call2
static struct ys_ret krk_call2(void *sm, const char *name, int nb, union ycall_arg *args, int *types) {
    KrkValue funcValue;

    // Look up the function by name in the global variables
    if (!krk_tableGet(&krk_currentThread.globals->fields, OBJECT_VAL(S(name)), &funcValue)) {
        printf("Error: Function '%s' not found in the VM\n", name);
        struct ys_ret result = { .t = YS_VPTR, .v.vptr = NULL };
        return result; // Return default value if the function is not found
    }

    return krk_coreCall(funcValue, nb, args, types);
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

static int loadFile(void *s, const char *fileName)
{
    if (!fileName) {
        printf("Error: File name is NULL.\n");
        return -1;
    }

    KrkValue result = krk_runfile(fileName, fileName);
    if (IS_NONE(result)) {
        printf("Error: Failed to execute file '%s'.\n", fileName);
        return -1;
    }

    printf("File '%s' executed successfully.\n", fileName);
    return 0;
}

static int loadString(void *s, const char *str)
{
    if (!str) {
        printf("Error: Script string is NULL.\n");
        return -1;
    }

    KrkValue result = krk_interpret(str, "<string>");
    if (IS_NONE(result)) {
        printf("Error: Failed to execute string script.\n");
        return -1;
    }

    printf("String script executed successfully.\n");
    return 0;
}

static int destroy(void *sm)
{
    krk_freeVM(); // Free the VM resources
    printf("Kuroko VM destroyed.\n");
    return 0;
}

static void addFuncSymbole(void *sm, const char *name, int nbArgs, Entity *func)
{
    KrkValue global = krk_currentThread.globals->fields;
    KrkValue funcValue = krk_pointer(func); // Wrap the Entity as a KrkValue
    char *generatedFunction = NULL;

    // If no name is provided, use the function's name
    const char *funcName = name ? name : yeGetString(func);

    // Generate a unique name for the global function
    char *uniqueName = malloc(strlen(funcName) + 8);
    sprintf(uniqueName, "%sGlobal", funcName);

    // Add the function to the global namespace
    krk_push(funcValue);
    krk_tableSet(&krk_currentThread.globals->fields, OBJECT_VAL(S(uniqueName)), krk_peek(0));
    krk_pop(); // Remove the value from the stack

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
