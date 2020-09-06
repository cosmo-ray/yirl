/**
 * This module is here to allow to load C application and run them in yirl
 * for now I only support sl, and only partially
 * some function are emulated to behave diferently as they C conterpart
 * like exit, that should not exit, and some are add yirl in tcc_sym.c
 * At first I'd like to support curses more, then text redirection
 * into a text-screen, maybe latter graphical app
 */
#include <yirl/all.h>

int exit_env = -1;
const char *mod_path;

Entity *cur_wid_;

Entity *cur_wid(void)
{
	return cur_wid_;
}

void exit(int status)
{
	if (!exit_env < 0)
		return;
	yuiLongExit(exit_env, status + 1);
}

void *capp_action(int nbArg, void **w_args)
{
	Entity *capp = w_args[0];
	cur_wid_ = capp;
	Entity *args = yeGet(capp, "args");
	int argc = yeLen(args) + 1;
	char *argv[argc];
	unsigned int argv_tot_len = 0;

	{
		YE_FOREACH(args, arg) {
			argv_tot_len += yeLen(arg);
		}
	}

	char argv_strs[argv_tot_len];
	int cnt = 0;
	int i = 0;

	if (!yeGetIntAt(capp, "symboles_add")) {
		yeSetAt(capp, "symboles_add", 1);
		ysTccPushSysincludePath(ygGetManager("tcc"), mod_path);
		ysTccPushSym(ygGetManager("tcc"), "exit", exit);
		ysTccPushSym(ygGetManager("tcc"), "cur_wid", cur_wid);
		Entity *files = yeGet(capp, "files");
		YE_FOREACH(files, file) {
			ysLoadFile(ygGetManager("tcc"), yeGetString(file));
		}
	}
	argv[i++] = "yirl-app";
 	YE_FOREACH(args, arg) {
		strcpy(argv_strs, yeGetString(arg));
		argv[i++] = argv_strs;
		argv_strs += yeLen(arg) + 1;
	}

	yuiTryMain(ysTccGetSym(ygGetManager("tcc"), "main"), argc, argv);

out:
	if (yeGet(capp, "quit")) {
		Entity *call = yeGet(capp, "quit");

		yesCall(call, capp);
	} else {
		ygTerminate();
	}
	return NULL;
}

void *init_capp(int nbArg, void **args)
{
	Entity *capp = args[0];
	void *ret;
	Entity *txt;
	char empty_line[61];

	memset(empty_line, ' ', 60);
	empty_line[60] = 0;
	YEntityBlock {
		capp.action = capp_action;
		capp.text = [];
		capp.symboles_add = 0;
		capp["text-align"] = "center";
	}

	if (!yeGet(capp, "background"))
		yeCreateString("rgba: 255 255 255 255", capp, "background");
	txt = yeGet(capp, "text");
	for (int i = 0; i < 25; ++i) {
		yeCreateString(empty_line, txt, NULL);
	}
	ret = ywidNewWidget(capp, "text-screen");
	return ret;
}

void *mod_init(int nbArg, void **args)
{
	Entity *mod = args[0];
	Entity *init;

	init = yeCreateArray(NULL, NULL);
	YEntityBlock {
		init.name = "capp";
		init.callback = init_capp;
		mod.name = "capp";
		mod.starting_widget = "sl";
		mod.sl = [];
		mod.sl["<type>"] = "capp";
		mod.sl.files = [];
		mod.sl.files[0] = "sl/sl.c";
		mod.sl.args = [];
		mod.sl.args[0] = "-G";
		mod.sl.args[2] = "-w";
	}
	mod_path = yeGetStringAt(mod, "$path");
	ywidAddSubType(init);
	return mod;
}
