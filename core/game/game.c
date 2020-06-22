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

#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <glib.h>
#include <fcntl.h>

#include "game.h"
#include "timer.h"

/* description */
#include "json-desc.h"
#include "rawfile-desc.h"
#include "description.h"

/* scripting */
#include "entity-script.h"
#include "lua-script.h"
#include "lua-binding.h"
#include "tcc-script.h"
#include "s7-script.h"
#include "quickjs-script.h"
#include "native-script.h"
#include "ybytecode-script.h"

/* widgets */
#include "utils.h"
#include "entity.h"
#include "sdl-driver.h"
#include "menu.h"
#include "map.h"
#include "text-screen.h"
#include "canvas.h"
#include "container.h"
#include "native-script.h"
#include "condition.h"

#ifdef GAMEMOD
#include <gamemode_client.h>
#endif

static int init;
static void *jsonManager;
static void *rawfileManager;

static void *luaManager;
static void *tccManager;
static void *s7Manager;
static void *qjsManager;

static Entity *mainMod;
static Entity *modList;
static Entity *baseMod;
static Entity *globalsFunctions;
static Entity *stalked_array;

static int alive = 1;

static char main_dir[PATH_MAX];

static YTimer *game_tick;

static int nb_ext_core;
static int in_child;

#define MAX_CHILD 64
#define PIPE_MSG_SIZE 2048
#define PIPE_MSG_DATA_SIZE (2048 - sizeof(uint16_t))

enum {
	RANDOM_TXT,
	RANDOM_INT,
	EXEC_SCRIPT,
	CALLBACK,
	ENTITY_START,
	ENTITY_FAIL,
	ENTITY_END
};

static int pipefd_tochild[MAX_CHILD][2];
static int pipefd_fromchild[MAX_CHILD][2];

static int read_pipefd(void)
{
	return pipefd_tochild[nb_ext_core][0];
}

static int write_pipefd(void)
{
	return pipefd_fromchild[nb_ext_core][1];
}

char *yProgramArg;

char ygBinaryRootPathBuf[PATH_MAX];
char *ygBinaryRootPath;
char yg_user_dir[PATH_MAX];

const char *ygUserDir(void)
{
	if (yg_user_dir[0])
		return yg_user_dir;
	return "./";
}

void *ygQjsManager(void)
{
	return qjsManager;
}

void *ygS7Manager(void)
{
	return s7Manager;
}

void *ygGetLuaManager(void)
{
	return luaManager;
}

void *ygGetTccManager(void)
{
	return tccManager;
}

#define CHECK_AND_RET(operation, err_val, ret, fmt, args...) do {	\
		if ((operation) == (err_val)) {				\
			DPRINT_ERR(fmt, ## args);			\
			return (ret);					\
		}							\
	} while (0)

#define CHECK_AND_GOTO(operation, err_val, label, fmt, args...) do {    \
		if ((operation) == (err_val)) {				\
			DPRINT_ERR(fmt, ## args);			\
			goto label;					\
		}							\
	} while (0)

void ygTerminate(void)
{
	ysdl2WindowMode();
	alive = 0;
}

static void *ygTerminateCallback(int nb, union ycall_arg *args, int *types)
{
	ygTerminate();
	return (void *)ACTION;
}

static void *quitOnKeyDown(int nb, union ycall_arg *args, int *types)
{
	Entity *events = args[1].e;
	Entity *eve = events;

	YEVE_FOREACH(eve, events) {
		if (ywidEveType(eve) == YKEY_DOWN) {
			if (ywidEveKey(eve) == 'q' ||
			    ywidEveKey(eve) == Y_ESC_KEY) {
				alive = 0;
				return (void *)ACTION;
			}
		} else if (ywidEveType(eve) == YKEY_MOUSEDOWN) {
			alive = 0;
			return (void *)ACTION;
		}
	}
	return (void *)NOTHANDLE;
}

static void *callOnKeyDown(int nb, union ycall_arg *args, int *types)
{
	Entity *events = args[1].e;
	Entity *eve = events;

	YEVE_FOREACH(eve, events) {
		if (ywidEveType(eve) == YKEY_DOWN) {
			if (ywidEveKey(eve) == 'q' ||
			    ywidEveKey(eve) == Y_ESC_KEY) {
				yesCall(yeGet(args[0].e, "to_call"), args[0].e);
				return (void *)ACTION;
			}
		} else if (ywidEveType(eve) == YKEY_MOUSEDOWN) {
			yesCall(yeGet(args[0].e, "to_call"), args[0].e);
			return (void *)ACTION;
		}
	}
	return (void *)NOTHANDLE;
}

static void *fullScreenOnOff(int nb, union ycall_arg *args, int *types)
{
	static int fs;

	if (fs) {
		ysdl2WindowMode();
		fs = 0;
	} else {
		ysdl2FullScreen();
		fs = 1;
	}
	return (void *)NOTHANDLE;
}

static void *nextWid_(int nb, union ycall_arg *args, int *types, const char *dst)
{
	Entity *wid = args[0].e;
	Entity *target = nb > 1 ? args[1].e : NULL;
	Entity *next = yeGet(wid, dst);

	// adjuste for when called with events
	if (nb > 2)
		target = args[2].e;
	if (nb == 1 || (yeType(target) != YSTRING &&
			!yeGet(target, "<type>"))) {
		Entity *te = yeGet(wid, "next_target");

		if (te)
			target = te;
		else
			target = wid;
	}

	if (!next) {
		if (ywidGetState(target) != ywidGetMainWid()) {
			ywRemoveEntryByEntity(ywCntWidgetFather(target), target);
			return (void *)ACTION;
		}
		DPRINT_ERR("unable to get next widget");
		return (void *)BUG;
	}

	if (ywidNext(next, target) < 0)
		return (void *)BUG;
	return (void *)ACTION;
}

static void *nextWid(int nb, union ycall_arg *args, int *types)
{
	return nextWid_(nb, args, types, "next");
}

static void *nextWidLose(int nb, union ycall_arg *args, int *types)
{
	return nextWid_(nb, args, types, "next-lose");
}


static void *proto_maker(int nb, union ycall_arg *args, int *types)
{
	Entity *e = args[0].e;
	Entity *init = args[1].e;
	Entity *prototype = yeGet(init, "proto");
	yeAutoFree Entity *patch = yePatchCreate(e, prototype, NULL, NULL);

	yePatchAplyExt(e, patch, YE_PATCH_NO_SUP);
	return ywidNewWidget(e, NULL);
}

static void *setInt(int nb, union ycall_arg *args, int *types)
{
	// args[0] is the widget and args[1] are the events
	Entity *toSet = args[2].e;
	Entity *value = args[3].e;

	ygSetInt(yeGetString(toSet), yeGetInt(value));
	return 0;
}

static void *recreateInt(int nb, union ycall_arg *args, int *types)
{
	// args[0] is the widget and args[1] are the events
	Entity *toSet = args[2].e;
	Entity *value = args[3].e;

	ygReCreateInt(yeGetString(toSet), yeGetInt(value));
	return 0;
}

static void *recreateString(int nb, union ycall_arg *args, int *types)
{
	// args[0] is the widget and args[1] are the events
	Entity *toSet = args[2].e;
	Entity *value = args[3].e;

	ygReCreateString(yeGetString(toSet), yeGetString(value));
	return NULL;
}

static void *randomCall(int nb, union ycall_arg *args, int *types)
{
	int nb_rand = nb - 2;

	if (nb_rand <= 0)
		return NULL;
	ywidAction(args[2 + yuiRand() % nb_rand].e, args[0].e,
		   args[1].e);
	return NULL;
}

static void *chkInfD100(int nb, union ycall_arg *args, int *types)
{
	// args[0] is the widget and args[1] are the events
	int target = yeGetInt(args[2].e);

	return (void *)(target < (yuiRand() % 100));
}

static void *increaseInt(int nb, union ycall_arg *args, int *types)
{
	// args[0] is the widget and args[1] are the events
	Entity *toSet = args[2].e;
	Entity *value = args[3].e;

	ygIncreaseInt(yeGetString(toSet), yeGetInt(value));
	return 0;
}

static void *nextOnKeyDown(int nb, union ycall_arg *args, int *types)
{
	Entity *events = args[1].e;
	Entity *eve = events;
	void *ret = (void *)NOTHANDLE;

	YEVE_FOREACH(eve, events) {
		if (ywidEveType(eve) == YKEY_DOWN) {
			if (ywidEveKey(eve) == ' ' ||
			    ywidEveKey(eve) == '\n' ||
			    ywidEveKey(eve) == Y_ESC_KEY) {
				ret = nextWid(nb, args, types);
			}
		} else if (ywidEveType(eve) == YKEY_MOUSEDOWN) {
			ret = nextWid(nb, args, types);
		}
	}
	return ret;
}

static void addNativeFuncToBaseMod(void)
{
	ysRegistreNativeFunc("proto_maker", proto_maker);
	ysRegistreCreateNativeEntity(fullScreenOnOff, "FullScreenOnOff",
				     baseMod, NULL);
	ysRegistreCreateNativeEntity(ygTerminateCallback, "FinishGame",
				     baseMod, NULL);
	ysRegistreCreateNativeEntity(quitOnKeyDown, "QuitOnKeyDown", baseMod,
				     NULL);
	ysRegistreCreateNativeEntity(callOnKeyDown, "CallOnKeyDown", baseMod,
				     NULL);
	ysRegistreCreateNativeEntity(nextWid, "callNext", baseMod, NULL);
	ysRegistreCreateNativeEntity(nextWidLose, "callNextLose", baseMod, NULL);
	ysRegistreCreateNativeEntity(nextOnKeyDown, "nextOnKeyDown",
				     baseMod, NULL);
	ysRegistreCreateNativeEntity(setInt, "setInt", baseMod, NULL);
	ysRegistreCreateNativeEntity(recreateString, "recreateString",
				     baseMod, NULL);
	ysRegistreCreateNativeEntity(recreateInt, "recreateInt", baseMod, NULL);
	ysRegistreCreateNativeEntity(increaseInt, "increaseInt", baseMod, NULL);
	ysRegistreCreateNativeEntity(randomCall, "randomCall", baseMod, NULL);
	ysRegistreCreateNativeEntity(chkInfD100, "chkInfD100", baseMod, NULL);
	yeCreateFunctionSimple("menuMove", ysNativeManager(), baseMod);
	yeCreateFunctionSimple("menuNext", ysNativeManager(), baseMod);
	yeCreateFunctionSimple("menuMainWidNext", ysNativeManager(), baseMod);
	yeCreateFunctionSimple("menuActions", ysNativeManager(), baseMod);
	ygRegistreFunc(ysNativeManager(), 0, "FinishGame", "yFinishGame");
	ygRegistreFunc(ysNativeManager(), 1, "callNext", "yCallNextWidget");
	ygRegistreFunc(ysNativeManager(), 1, "callNextLose", "yNextLose");
}

int ygIsAlive(void)
{
	return alive;
}

int ygIsInit(void)
{
	return init;
}

int ygInit(GameConfig *cfg)
{
	static int t;
	char *path;

	/* trick use in case of failure in this function to free all */
	init = 1;

	yuiRandInit();
	yuiDebugInit();

	/* Init parseurs */
	yeInitMem();

	if (!ygBinaryRootPath)
		ygBinaryRootPath = getcwd(ygBinaryRootPathBuf, PATH_MAX);
       /* Init Game mode if GAMEMODE is set */
#ifdef GAMEMOD
	gamemode_request_start();
#endif

	globalsFunctions = yeCreateArray(NULL, NULL);
	CHECK_AND_RET(t = ydJsonInit(), -1, -1,
		      "json init failed");
	CHECK_AND_GOTO(jsonManager = ydNewManager(t), NULL, error,
		       "json init failed");
	CHECK_AND_GOTO(rawfileManager = ydNewManager(ydRawFileInit()),
		       NULL, error, "raw-file init failed");

  /* Init scripting */
  /* TODO init internal lua function */
	CHECK_AND_GOTO(t = ysLuaInit(), -1, error, "lua init failed");
	CHECK_AND_GOTO(luaManager = ysNewManager(NULL, t), NULL, error,
		       "lua init failed");
	CHECK_AND_GOTO(yesLuaRegister(luaManager), -1, error,
		       "lua init failed");

	path = g_strdup_printf("%s%s", ygBinaryRootPath,
			       "/scripts-dependancies/object-wrapper.lua");
	if (ysLoadFile(luaManager, path) < 0) {
		DPRINT_WARN("can't load %s: %s", path,
			    ysGetError(luaManager));
	}
	free(path);
	CHECK_AND_GOTO(t = ysTccInit(), -1, error, "tcc init failed");
	CHECK_AND_GOTO(tccManager = ysNewManager(NULL, t), NULL, error,
		       "tcc init failed");

	CHECK_AND_GOTO(t = ysS7Init(), -1, error, "s7 init failed");
	CHECK_AND_GOTO(s7Manager = ysNewManager(NULL, t), NULL, error,
		       "s7 init failed");

	CHECK_AND_GOTO(t = ysQjsInit(), -1, error, "Qjs init failed");
	CHECK_AND_GOTO(qjsManager = ysNewManager(NULL, t), NULL, error,
		       "Qjs init failed");

	/* Init widgets */
	baseMod = yeCreateArray(NULL, NULL);
	addNativeFuncToBaseMod();

	if (cfg->win_name)
		ywidSetWindowName(cfg->win_name);
	ywidChangeResolution(cfg->w, cfg->h);

	ysdl2Init();
	ysound_init();

	CHECK_AND_GOTO(ywMenuInit(), -1, error, "Menu init failed");
	CHECK_AND_GOTO(ywMapInit(), -1, error, "Map init failed");
	CHECK_AND_GOTO(ywTextScreenInit(), -1, error, "Text Screen init failed");
	CHECK_AND_GOTO(ywContainerInit(), -1, error, "Container init failed");
	CHECK_AND_GOTO(ywCanvasInit(), -1, error, "Canvas init failed");

	CHECK_AND_GOTO(ysdl2RegistreTextScreen(), -1, error,
		       "Text Screen init failed");
	CHECK_AND_GOTO(ysdl2RegistreMenu(), -1, error,
		       "Menu init failed");
	CHECK_AND_GOTO(ysdl2RegistreMap(), -1, error,
		       "Map init failed");
	CHECK_AND_GOTO(ysdl2RegistreCanvas(), -1, error,
		       "Canvas SDL2 init failed");
	modList = yeCreateArray(NULL, NULL);
	stalked_array = yeCreateArray(NULL, NULL);

	/* Get Current Path, this comment is useless,
	 * but that small function need more visibility */
	if (!getcwd(main_dir, PATH_MAX))
		goto error;
	game_tick = YTimerCreate();

	return 0;
error:
	ygEnd();
	return -1;
}

void ygEnd()
{
	if (!init)
		return;

	printf("kill all childs !\n");
	for (int i = 0; i < nb_ext_core; ++i) {
		close(pipefd_tochild[i][1]);
		close(pipefd_fromchild[i][0]);
	}
	nb_ext_core = 0;
	free(game_tick);
	ywidFreeWidgets();
	ydDestroyManager(jsonManager);
	ydJsonEnd();
	ydDestroyManager(rawfileManager);
	ydRawFileEnd();
	ywTextScreenEnd();
	ywMapEnd();
	ywMenuEnd();
	ywCanvasEnd();
	ywContainerEnd();
	ysound_end();
	ysdl2Destroy();
	yeDestroy(modList);
	modList = NULL;
	yeDestroy(baseMod);
	baseMod = NULL;
	yeDestroy(stalked_array);
	stalked_array = NULL;
	yeDestroy(globalsFunctions);
	globalsFunctions = NULL;
	ysNativeEnd();
	ysYBytecodeEnd();
	ysDestroyManager(tccManager);
	ysTccEnd();
	ysDestroyManager(luaManager);
	ysLuaEnd();
	ysDestroyManager(s7Manager);
	ysS7End();
	/* it seems V crash :( */
	/* ysDestroyManager(qjsManager); */
	/* ysQjsEnd(); */
	yeEnd();
	free(yProgramArg);
	yProgramArg = NULL;
	init = 0;
#ifdef GAMEMODE
	gamemode_request_end();
#endif
}

uint32_t ygGetTick(void)
{
	return YTimerGet(game_tick) / 1000;
}

int ygRegistreFuncInternal(void *manager, int nbArgs, const char *name,
			   const char *toRegistre)
{
	Entity *func = yeGet(globalsFunctions, name);

	if (!func) {
		func = yeCreateFunctionExt(name, manager, globalsFunctions,
					   name,
					   YE_FUNC_NO_FASTPATH_INIT);
	}

	if (!toRegistre || yuiStrEqual0(name, toRegistre)) {
		if (manager != tccManager)
			ysAddFuncSymbole(tccManager, NULL, nbArgs, func);
		if (manager != luaManager)
			ysAddFuncSymbole(luaManager, NULL, nbArgs, func);
	} else {
		ysAddFuncSymbole(tccManager, toRegistre, nbArgs, func);
		ysAddFuncSymbole(luaManager, toRegistre, nbArgs, func);
	}
	return 0;
}

int ygModDir(const char * restrict const mod)
{
	int r = chdir(yeGetStringAt(ygGet(mod), "$path"));
	return r;
}

int ygModDirOut(void)
{
	return chdir(main_dir);
}

void *ygGetManager(const char *name)
{
	if (yuiStrEqual0(name, "tcc"))
		return tccManager;
	else if (yuiStrEqual0(name, "lua"))
		return luaManager;
	else if (yuiStrEqual0(name, "yb"))
		return ysYBytecodeManager();
	else if (yuiStrEqual0(name, "s7"))
		return s7Manager;
	else if (yuiStrEqual0(name, "js"))
		return qjsManager;
	return NULL;
}

Entity *ygFileToEnt(YFileType t, const char *path, Entity *father)
{
	if (t == YJSON)
		return ydFromFile(jsonManager, path, father);
	else if (t == YRAW_FILE)
		return ydFromFile(rawfileManager, path, father);
	return NULL;
}

int ygEntToFile(YFileType t, const char *path, Entity *ent)
{
	if (t == YJSON)
		return ydToFile(jsonManager, path, ent);
	return -1;
}

enum {
	VOID_CALLBACK,
	INT_CALLBACK,
	STR_CALLBACK
};

struct msg {
	uint16_t action;
	union {
		char msg[PIPE_MSG_DATA_SIZE];
		int integer;
		struct {
			char manager[4];
			char path[1024 - sizeof(uint16_t)];
		};
		struct {
			uint16_t callback_id;
			uint16_t callback_type;
			union {
				char arg[PIPE_MSG_DATA_SIZE -
					 sizeof(uint16_t) * 2];
				int int_arg;
			};
		};
		struct {
			uint8_t entity_l;
			int8_t entity_type;
			char names[PIPE_MSG_DATA_SIZE - sizeof(uint8_t) * 2];
		} __attribute__((packed));
	} __attribute__((packed));
} __attribute__((packed));

static Entity *callback_array;

void ygChildRegisterCallback(Entity *func_e, int id)
{
	if (!in_child)
		return;
	yePushAt(callback_array, func_e, id);
}

int ygChildPushMsgInt(int val)
{
	struct msg m;
	if (!in_child)
		return -1;

	m.action = RANDOM_INT;
	m.integer = val;
	return write(write_pipefd(), &m, sizeof(m));
}

static int ygPipeFmtWriteEntBody(Entity *e, int fd)
{
	switch (yeType(e)) {
	case YINT:
		printf("go I\n");
		return ygChildPushMsgInt(yeGetInt(e));
	case YSTRING:
		printf("go str\n");
		return ygChildPushMsgStr(yeGetString(e));
	case YARRAY:
		printf("go o e\n");
		return ygChildPushEntity(e);
	default:;
		printf("default case %s\n", yeTypeAsString(e));
	}
	return -1;
}

int ygChildPushEntity(Entity *e)
{
	struct msg m;
	int t = yeType(e);
	int fd = write_pipefd();

	if (!in_child)
		return -1;

	m.action = ENTITY_START;
	m.entity_type = t;
	if (t == YARRAY) {
		char *msg = m.names;

		m.entity_l = yeLen(e);
		YE_ARRAY_FOREACH_ENTRY(e, entry) {
			/* I should count, but for now, I will overflow :( */
			if (entry->name)
				msg = stpcpy(msg, entry->name);
			else
				*msg = 0;
			++msg;
		}
	}
	if (write(fd, &m, sizeof(m)) < 0)
		return -1;

	if (t == YARRAY) {
		YE_FOREACH(e, elem) {
			if (ygPipeFmtWriteEntBody(elem, fd) < 0)
				goto fail;
		}
	} else {
		if (ygPipeFmtWriteEntBody(e, fd) < 0)
			goto fail;
	}
	printf("write end !\n");
	m.action = ENTITY_END;
	return write(fd, &m, sizeof(m));
fail:
	m.action = ENTITY_FAIL;
	write(fd, &m, sizeof(m));
	return -1;
}

int ygChildPushMsgStr(const char *s)
{
	struct msg m;
	/* int l = strlen(s); */

	if (!in_child)
		return -1;
	m.action = RANDOM_TXT;
	strcpy(m.msg, s);
	return write(write_pipefd(), &m, sizeof(m));
}


#define CORE_CHK_PARAMS(core) do {		\
if (in_child)					\
	return -1;				\
if (core >= nb_ext_core)			\
	return -1;				\
} while (0)

int ygCoreSetNonBlockingReader(int core)
{
	CORE_CHK_PARAMS(core);
	return fcntl(pipefd_fromchild[core][0], F_SETFL, O_NONBLOCK);
}

static int get_entity_end(int core)
{
	struct msg m;
	int r_ret = read(pipefd_fromchild[core][0], &m, PIPE_MSG_SIZE);

	if (r_ret < 0) {
		DPRINT_ERR("ENTITY_END/FAIL missing");
		return -1;
	}
	if (m.action != ENTITY_END) {
		DPRINT_ERR("Something went horribly wrong %d", m.action);
		abort();
		return -1;
	}
	return 0;
}

Entity *ygCoreGetEntity(int core, Entity *parent, const char *name)
{
	struct msg m;
	int r_ret = read(pipefd_fromchild[core][0], &m, sizeof(m));

	printf("%d\n", r_ret);
	if (r_ret <= 0)
		return NULL;
	if (m.action == RANDOM_TXT)
		return yeCreateString(m.msg, parent, name);
	else if (m.action == RANDOM_INT)
		return yeCreateInt(m.integer, parent, name);
	else if (m.action == ENTITY_START) {
		printf("Entity await mode\n");
		if (m.entity_type != YARRAY) {
			printf("can it even happen\n");
			Entity *r = ygCoreGetEntity(core, parent, name);
			if (get_entity_end(core) < 0)
				return NULL;
			return r;
		} else {
			Entity *r = yeCreateArray(parent, name);
			char *names = m.names;

			printf("l: %d\n", m.entity_l);
			for (int i = 0; i < m.entity_l; ++i) {
				char *name = *names == 0 ? NULL : names;

				printf("- %s - %d\n", name ? name : NULL, i);
				ygCoreGetEntity(0, r, name);
				names += strlen(names) + 1;
			}
			printf("out !\n");
			if (get_entity_end(core) < 0)
				return NULL;
			return r;
		}
	}
	DPRINT_ERR("Unknow message type %d", m.action);
	return NULL;
}

int ygCoreLoadScript(int core, const char *manager, const char *path)
{
	struct msg m;

	CORE_CHK_PARAMS(core);
	m.action = EXEC_SCRIPT;
	strncpy(m.manager, manager, 4);
	strncpy(m.path, path, 1024);
	/* 1024 + 4 + sizeof(uint16_t) */
	return write(pipefd_tochild[core][1], &m, 1030);
}

int ygCoreDoCallbackStr(int core, int callback_id, const char *arg)
{
	struct msg m;
	CORE_CHK_PARAMS(core);

	m.action = CALLBACK;
	m.callback_id = callback_id;
	m.callback_type = STR_CALLBACK;
	strncpy(m.arg, arg, PIPE_MSG_SIZE - sizeof(uint16_t) * 2);
	return write(pipefd_tochild[core][1], &m, PIPE_MSG_SIZE);
}

int ygCoreDoCallbackInt(int core, int callback_id, int arg)
{
	struct msg m;
	CORE_CHK_PARAMS(core);

	m.action = CALLBACK;
	m.callback_id = callback_id;
	m.callback_type = INT_CALLBACK;
	m.int_arg = arg;
	return write(pipefd_tochild[core][1], &m, PIPE_MSG_SIZE);
}

int ygCoreDoCallbackVoid(int core, int callback_id)
{
	struct msg m;
	CORE_CHK_PARAMS(core);

	m.action = CALLBACK;
	m.callback_type = VOID_CALLBACK;
	m.callback_id = callback_id;
	return write(pipefd_tochild[core][1], &m, PIPE_MSG_SIZE);
}

static void fork_wait(void)
{
	char buf[PIPE_MSG_SIZE];
	int r_ret;
	struct msg *m;

	while ((r_ret = read(read_pipefd(), buf, PIPE_MSG_SIZE)) > 0) {
		/* just in case, we truncat to avoir overflow */
		buf[PIPE_MSG_SIZE - 1] = 0;
		m = (void *)buf;
		if (m->action == EXEC_SCRIPT) {
			ysLoadFile(ygGetManager(m->manager), m->path);
		} else if (m->action == CALLBACK) {
			if (m->callback_type == STR_CALLBACK) {
				yesCall(yeGet(callback_array, m->callback_id),
					m->arg);
			} else if (m->callback_type == VOID_CALLBACK) {
				yesCall(yeGet(callback_array, m->callback_id));
			} else if (m->callback_type == INT_CALLBACK) {
				yesCall(yeGet(callback_array, m->int_arg));
			}
		}
	}
	yeDestroy(callback_array);
	close(read_pipefd());
	close(write_pipefd());
	return;
}

Entity *ygLoadMod(const char *path)
{
	char *tmp;
	Entity *mod = NULL;
	Entity *type;
	Entity *file;
	Entity *starting_widget;
	Entity *preLoad;
	Entity *initScripts;
	Entity *name;
	Entity *prototype;
	int request_core;

	YE_NEW(string, tmp_name, "");
	for (int i = 0; i < 4; ++i) {
		const char * const starts[] = {"/start.c", "/start.lua",
					       "/start.scm", "/start.js"};
		void * const managers[] = {tccManager, luaManager,
					   s7Manager, qjsManager};

		tmp = g_strconcat(path, starts[i], NULL);
		CHECK_AND_RET(tmp, NULL, NULL,
			      "cannot allocated path(like something went really wrong)");
		if (!access(tmp, F_OK) && !ysLoadFile(managers[i], tmp)) {
			mod = yeCreateArray(NULL, NULL);
			yeCreateString(path, mod, "$path");
			if (ysCall(managers[i], "mod_init", mod))
				break;
		}
		yeDestroy(mod);
		free(tmp);
		tmp = NULL;
	}

	if (!mod) {
		tmp = g_strconcat(path, "/start.json", NULL);
		CHECK_AND_RET(tmp, NULL, NULL,
			      "cannot allocated path(like something went really wrong)");
		mod = ydFromFile(jsonManager, tmp, NULL);
		yeCreateString(path, mod, "$path");
	}

	if (!mod)
		goto failure;
	request_core = yeGetIntAt(mod, "request-ext-core");

	while (request_core >= nb_ext_core) {
		int pid;

		if (nb_ext_core == MAX_CHILD) {
			DPRINT_ERR("ext core limit reach");
			goto failure;
		}

		if (pipe(pipefd_tochild[nb_ext_core]) == -1 ||
		    pipe(pipefd_fromchild[nb_ext_core]) == -1) {
			DPRINT_ERR("pipe fail");
			goto failure;
		}

		if ((pid = fork()) < 0) {
			DPRINT_ERR("fork fail");
			goto failure;
		} else if (!pid) {
			in_child = 1;
			yeRemoveChild(modList, mod);
			callback_array = yeCreateArray(NULL, NULL);
			free(tmp);
			close(pipefd_tochild[nb_ext_core][1]);
			close(pipefd_fromchild[nb_ext_core][0]);
			fork_wait();
			return NULL;
		}
		close(read_pipefd());
		close(write_pipefd());
		nb_ext_core++;
	}

	name = yeGet(mod, "name");

	if (!name) {
		char *last_slash = strrchr(path, '/');
		int short_end = 0;

		if (strlen(last_slash) == 1) {
			do {
				--last_slash;
			} while (last_slash != path && *last_slash != '/');
			short_end = 1;
		}
		if (*last_slash == '/')
			++last_slash;
		yeSetString(tmp_name, last_slash);
		yeStringTruncate(tmp_name, short_end);
		name = tmp_name;
	}

	if (yeGet(modList, yeGetString(name))) {
		yeDestroy(mod);
		goto exit;
	}
	yePushBack(modList, mod, yeGetString(name));
	yeDestroy(mod);
	type = yeGet(mod, "type");
	file = yeGet(mod, "file");
	preLoad = yeGet(mod, "pre-load");
	initScripts = yeGet(mod, "init-scripts");

	YE_ARRAY_FOREACH(preLoad, var) {
		Entity *tmpType = yeGet(var, "type");
		Entity *tmpFile = yeGet(var, "file");
		Entity *pathEnt = yeGet(var, "path");
		char *fileStr = g_strconcat(path, "/",
					    yeGetString(tmpFile), NULL);
		const char *pathCstr = "no path set";


		if (tmpFile) {
			pathCstr = fileStr;
		} else if (pathEnt) {
			char *mod_path = g_strdup_printf("%s%s",
							 ygBinaryRootPath,
							 "/modules/");

			yeStringReplace(pathEnt, "YIRL_MODULES_PATH", mod_path);
			free(mod_path);
			pathCstr = yeGetString(pathEnt);
		}
		if (yuiStrEqual0(yeGetString(tmpType), "lua")) {
			if (ysLoadFile(luaManager, pathCstr) < 0) {
				DPRINT_ERR("Error when loading '%s': %s\n",
					   pathCstr, ysGetError(luaManager));
				goto fail_preload;
			}

		} else if (yuiStrEqual0(yeGetString(tmpType), "tcc")) {
			if (ysLoadFile(tccManager, pathCstr) < 0) {
				DPRINT_ERR("Error when loading '%s': %s\n",
					   pathCstr, ysGetError(tccManager));
				goto fail_preload;
			}

		} else if (yuiStrEqual0(yeGetString(tmpType), "s7")) {
			if (ysLoadFile(s7Manager, pathCstr) < 0) {
				DPRINT_ERR("Error when loading '%s': %s\n",
					   pathCstr, ysGetError(s7Manager));
				goto fail_preload;
			}

		} else if (yuiStrEqual0(yeGetString(tmpType), "yb")) {
			if (ysLoadFile(ysYBytecodeManager(), pathCstr) < 0) {
				DPRINT_ERR("Error when loading '%s': %s\n",
					   pathCstr,
					   ysGetError(ysYBytecodeManager()));
				goto fail_preload;
			}
		} else if (yuiStrEqual0(yeGetString(tmpType), "js")) {
			if (ysLoadFile(qjsManager, pathCstr) < 0) {
				DPRINT_ERR("Error when loading '%s': %s\n",
					   pathCstr, ysGetError(qjsManager));
				goto fail_preload;
			}
		} else if (yuiStrEqual0(yeGetString(tmpType), "json")) {
			Entity *as = yeGet(var, "as");
			tmpFile = ydFromFile(jsonManager, pathCstr, mod);

			yeRenamePtrStr(mod, tmpFile, yeGetString(as));
		} else if (yuiStrEqual0(yeGetString(tmpType), "module")) {
			if (!ygLoadMod(pathCstr)) {
				DPRINT_ERR("fail to load module: %s", pathCstr);
			fail_preload:
				free(fileStr);
				goto failure;
			}
		}
		free(fileStr);
	}

	YE_ARRAY_FOREACH(initScripts, var2) {
		if (yeType(var2) == YSTRING) {
			void *fastPath = ysGetFastPath(tccManager,
						       yeGetString(var2));

			if (fastPath)
				ysFCall(tccManager, fastPath, mod);
			else
				ysCall(luaManager, yeGetString(var2), mod);
		} else if (yeType(var2) == YARRAY) {
			ysCall(ygGetManager(yeGetString(yeGet(var2, 0))),
			       yeGetString(yeGet(var2, 1)), mod);
		}
	}

	starting_widget = yeGet(mod, "starting widget");
	if (!starting_widget)
		starting_widget = yeGet(mod, "starting_widget");
	if (type) {
		if (yuiStrEqual(yeGetString(type), "json")) {
			char *fileStr;
			const char *mod_name = "$main file";

			fileStr = g_strconcat(path, "/",
					      yeGetString(file), NULL);
			file = ydFromFile(jsonManager, fileStr, mod);
			free(fileStr);
			if (!file) {
				goto failure;
			}
			if (yeGet(mod, "as"))
				mod_name = yeGetString(yeGet(mod, "as"));
			yeRenamePtrStr(mod, file, mod_name);

			starting_widget = yeGet(file,
						yeGetString(starting_widget));
		} else {
			DPRINT_ERR("start does not suport loader of type %s",
				   yeGetString(type));
		}
	} else {
		if (yeType(starting_widget) == YSTRING) {
			starting_widget =
				yeGetByStr(mod, yeGetString(starting_widget));
		}
	}

	if ((prototype = yeGet(mod, "make-prototype"))) {
		Entity *init = yeCreateArray(NULL, NULL);
		const char *proto_sc = yeGetString(prototype);

		yeCreateFunction("proto_maker", ysNativeManager(), init,
			"callback");

		yeCreateString(proto_sc, init, "name");
		yeCreateCopy(yeGet(mod, proto_sc), init, "proto");
		ywidAddSubType(init);
	}

	yePushBack(mod, starting_widget, "$starting widget");
	printf(" === add module: \"%s\" === \n", yeGetString(name));

	goto exit;
failure:
	yeRemoveChild(modList, mod);
	mod = NULL;
exit:
	free(tmp);
	return mod;
}

Entity *ygGetMod(const char *path)
{
	if (!path)
		return NULL;
	return yeGet(modList, path);
}

int ygLoadScript(Entity *mod, void *manager, const char *path)
{
	int ret;
	char *tmp = g_strconcat(yeGetString(yeGet(mod, "$path")), path, NULL);

	ret = ysLoadFile(manager, tmp);
	free(tmp);
	return ret;
}

static Entity *ygGetFunc(Entity *mod, const char *funcName)
{
	Entity *ret;

	if (mod)
		return yeGet(mod, funcName);

	ret = yeGet(globalsFunctions, funcName);
	if (!ret && ysGetFastPath(ysNativeManager(), funcName)) {
		ret = yeCreateFunctionSimple(funcName, ysNativeManager(),
					     globalsFunctions);
	}
	return ret;
}

static int isNestedEntity(const char *str, char *modName, const char **entName)
{
  uint32_t cpLen;

  *entName = g_strrstr(str, ":");
  if (!(*entName)) {
	  *entName = str;
	  return 0;
  }
  *entName = *entName + 1;

  cpLen = (*entName) - str - 1;
  strncpy(modName, str, cpLen);
  modName[cpLen] = '\0';
  return 1;
}

Entity *ygGetFuncExt(const char *func)
{
	char modName[254];
	const char *funcName;
	char *tmp;
	intptr_t ret;
	Entity *tmpMod;

	if (!func)
		return NULL;
	ret = isNestedEntity(func, modName, &funcName);
	if (ret)
		tmp = modName;
	else
		return ygGetFunc(NULL, funcName);

	tmpMod = ygGetMod(tmp);
	if (!tmpMod) {
		DPRINT_WARN("%s module desn't existe", tmp);
		return NULL;
	}
	return ygGetFunc(tmpMod, funcName);
}

Entity *ygGet(const char *toFind)
{
	char modName[254];
	const char *funcName;
	char *tmp;
	int ret;

	if (!toFind)
		return NULL;
	ret = isNestedEntity(toFind, modName, &funcName);
	tmp = modName;

	// if no module give in toFind, use modList as base
	if (!ret) {
		Entity *retVal = yeGetByStr(modList, funcName);

		if (!retVal)
			return ygGetFunc(NULL, funcName);
		return retVal;
	}
	return yeGetByStr(ygGetMod(tmp), funcName);
}

void ygIncreaseInt(const char *toInc, int val)
{
	Entity *e = ygGet(toInc);

	if (!e)
		return ygReCreateInt(toInc, val);
	yeAdd(e, val);
}

#define RECREATE_ARRAY()						\
	for (uint32_t i = 0, b_len = 0;					\
	     ({ buf[b_len] = toSet[i]; ++b_len; toSet[i] ;});		\
	     ++i) {							\
		if (unlikely(b_len > 1024))				\
			return;						\
		if (toSet[i] == '.' || toSet[i] == ':') {		\
			Entity *nfather;				\
			buf[b_len -1] = 0;				\
			nfather = yeGet(father, buf);			\
			if (!nfather)					\
				nfather = yeCreateArray(father, buf);	\
			father = nfather;				\
			b_len = 0;					\
			continue;					\
		}							\
	}								\

void ygReCreateInt(const char *toSet, int val)
{
	if (!toSet)
		return;
	Entity *father = modList;
	char buf[1024];							\

	RECREATE_ARRAY();
	yeReCreateInt(val, father, buf);
}

void ygReCreateString(const char *toSet, const char *str)
{
	Entity *father = modList;
	char buf[1024];							\

	RECREATE_ARRAY();
	yeReCreateString(str, father, buf);
}

#undef RECREATE_ARRAY

void ygSetInt(const char *toSet, int val)
{
	int len = strlen(toSet);
	Entity *father;
	Entity *e;
	const char *end;

	for (end = toSet + len - 1; end != toSet && *end != '.' &&  *end != ':';
	     --end, --len);
	if (end == toSet) {
		father = modList;
	} else {
		father = yeNGetByStr(modList, toSet, len - 1);
		toSet += len;
	}

	if ((e = yeGet(father, toSet)) == NULL)
		yeCreateInt(val, father, toSet);
	else
		yeSetInt(e, val);
}

int ygBind(YWidgetState *wid, const char *callback)
{
	Entity *callbackEnt = ygGetFuncExt(callback);
	if (!callbackEnt) {
		DPRINT_WARN("unable to find '%s'\n", callback);
	}
	if (!yeReplaceBack(wid->entity, callbackEnt, "action"))
		return -1;
	return 0;
}

static int ygParseStartAndGame(GameConfig *config)
{
	Entity *starting_widget;
	Entity *ws;
	Entity *wn;

	mainMod = ygLoadMod(config->startingMod->path);
	ws = yeGet(mainMod, "window size");
	wn = yeGet(mainMod, "window name");

	starting_widget = yeGet(mainMod, "$starting widget");

	if (starting_widget) {
		YWidgetState *wid = ywidNewWidget(starting_widget, NULL);

		if (!wid) {
			DPRINT_ERR("Fail to create widget of type '%s'.",
				   yeGetString(yeGet(starting_widget,
						     "<type>")));
			return -1;
		}
		ywidSetMainWid(wid);
	}

	if (!ywidGetMainWid()) {
		DPRINT_ERR("No main widget has been set.\n"
			   "see documentation about starting_widget\n"
			   "or set it manually with \"ywidSetMainWid()\"");
		return -1;
	}

	if (ws)
		ywidChangeResolution(yeGetIntAt(ws, 0), yeGetIntAt(ws, 1));
	if (wn)
		ywidSetWindowName(yeGetString(wn));
	return ygDoLoop();
}

int ygStalk(const char *entityPath, Entity *callback, Entity *arg)
{
	Entity *stalked = yeReCreateArray(stalked_array, entityPath, NULL);
	Entity *target = ygGet(entityPath);

	if (unlikely(!target || !callback))
		goto error;

	yeCreateString(entityPath, stalked, NULL);
	if (!yeCreateCopy(target, stalked, NULL))
		goto error;

	yePushBack(stalked, callback, NULL);
	yePushBack(stalked, arg, NULL);
	return 0;
error:
	yeRemoveChild(stalked_array, stalked);
	return -1;
}

int ygUnstalk(const char *entityPath)
{
	if (!yeGet(stalked_array, entityPath))
		return -1;
	yeRemoveChild(stalked_array, entityPath);
	return 0;
}

static void checkSlakedEntity(void)
{
	YE_ARRAY_FOREACH(stalked_array, elem) {
		Entity *original = ygGet(yeGetStringAt(elem, 0));
		Entity *copy = yeGet(elem, 1);
		if (!yeEqual(original, copy)) {
			yesCall(yeGet(elem, 2), original, copy, yeGet(elem, 3));
			yeCopy(original, copy);
		}
	}
}

int ygDoLoop(void)
{
	alive = 1;

	do {
		YWidgetState *wid = ywidGetMainWid();

		if (unlikely(!wid)) {
			return -1;
		}
		g_assert(ywidRend(wid) != -1);
		checkSlakedEntity();
		ywidDoTurn(wid);
	} while(alive);

	return 0;
}

int ygStartLoop(GameConfig *config)
{
	int ret = -1;

	if (!init) {
		DPRINT_ERR("ygInit() should be call before calling ygStartLoop");
		return -1;
	}

	if (!config || !config->startingMod) {
		DPRINT_ERR("GameConfig is brocken(some ptr are NULL)");
		return -1;
	}

	if (!config->startingMod->path) {
		DPRINT_ERR("!config->startingMod->path is NULL");
		return -1;
	}

	ret = ygParseStartAndGame(config);
	return ret;
}

int ygInitGameConfigByRenderType(GameConfig *cfg, const char *path,
				 RenderType t)
{

	cfg->win_name = NULL;
	cfg->w = ywidWindowWidth;
	cfg->h = ywidWindowHight;
	cfg->startingMod = malloc(sizeof(ModuleConf));
	cfg->startingMod->path = path;
	return 0;
}

int ygInitGameConfigByStr(GameConfig *cfg, const char *path, const char *render)
{
	cfg->startingMod = malloc(sizeof(ModuleConf));
	cfg->startingMod->path = path;
	cfg->w = ywidWindowWidth;
	cfg->h = ywidWindowHight;
	cfg->win_name = NULL;
	return 0;
}

void ygCleanGameConfig(GameConfig *cfg)
{
	free(cfg->startingMod);
}

void *ygCallInt(const char *mod, const char *callName, int nb,
		union ycall_arg *args, int *types)
{
	void *ret;
	Entity *modEntity;

	if (!callName)
		return NULL;
	modEntity = ygGetMod(mod);
	ret = yesCallInt(ygGetFunc(modEntity, callName), nb, args, types);
	return ret;
}

int ygAddDefine(const char *name, char *val)
{
	return ysAddDefine(tccManager, name, val);
}

int yePushToGlobalScope(Entity *entity, const char *name)
{
	if (yeGet(modList, name))
		return -1;
	return yePushBack(modList, entity, name);
}

void ygRemoveFromGlobalScope(const char *name)
{
	yeRemoveChild(modList, name);
}

#undef CHECK_AND_GOTO
#undef CHECK_AND_RET
