/*
**Copyright (C) 2016 Matthias Gatto
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
#include "tcc-script.h"
#include "game.h"

extern char yg_user_dir[PATH_MAX];

#define FAIL(...) do {				\
		fprintf(stderr, __VA_ARGS__);	\
		return -1;			\
	} while (0);

int main(int argc, char **argv)
{
  GameConfig cfg = {0};
  int ret = 1;
  const char *start = NULL;
  const char *name = NULL;
  char *binaryRootPath = NULL;
  const char *start_dir = NULL;
  char buf[PATH_MAX];
  char *cpath = getcwd(buf, PATH_MAX);
  int width = -1;
  int height = -1;
  int linux_user_path = 0;

  yuiDebugInit();

  for (int i = 1; i < argc; ++i) {
	  if (!strcmp(argv[i], "--name") || !strcmp(argv[i], "-n")) {
		  if (i + 1 == argc)
			  FAIL("name require a name\n");
		  ++i;
		  name = argv[i];
	  } else if (!strcmp(argv[i], "--width") || !strcmp(argv[i], "-W")) {
		  if (i + 1 == argc)
			  FAIL("width require a width\n");
		  ++i;
		  width = atoi(argv[i]);
	  } else if (!strcmp(argv[i], "--height") || !strcmp(argv[i], "-H")) {
		  if (i + 1 == argc)
			  FAIL("height require a height\n");
		  ++i;
		  height = atoi(argv[i]);
	  } else if (!strcmp(argv[i], "--arg")) {
		  if (i + 1 == argc)
			  FAIL("arg require an argument\n");
		  ++i;
		  yProgramArg = argv[i];
	  } else if (!strcmp(argv[i], "--linux-user-path") || !strcmp(argv[i], "-L")) {
		  linux_user_path = 1;
	  } else if (!strcmp(argv[i], "--binary-root-path") || !strcmp(argv[i], "-P")) {
		  if (i + 1 == argc)
			  FAIL("binary-root-path require a path\n");
		  ++i;
		  binaryRootPath = argv[i];
	  } else if (!strcmp(argv[i], "--start-dir") || !strcmp(argv[i], "-d")) {
		  if (i + 1 == argc)
			  FAIL("start-dir require a path\n");
		  ++i;
		  start_dir = argv[i];
	  } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
		  printf("Usage: %s [options...]\n"
			 "-n, --name <name>		Window Name\n"
			 "-W, --width <width>		Window Width\n"
			 "-H, --height <height>		Window Height\n"
			 "--arg	<arg>			main module argument\n"
			 "-L, --linux-user-path		store user data in ~/.yirl\n"
			 "-P, --binary-root-path <path>	set path to binary directory(which contain, tcc, script-dependancies, and defaults polices)\n"
			 "-d, --start-dir <path>	move on the given directorry, use as starting module\n",
			 argv[0]);
		  return 0;
	  }
  }

  if (start_dir) {
    if (chdir(start_dir) < 0)
	    goto end;
    cpath = getcwd(buf, PATH_MAX);
  }

  start = cpath;

  ygInitGameConfig(&cfg, start, NULL);
  cfg.win_name = name;
  if (width > 0)
    cfg.w = width;
  if (height > 0)
    cfg.h = height;

  if (binaryRootPath) {
    ygBinaryRootPath = binaryRootPath;
  }

  if (linux_user_path) {
	  const char *home = getenv("HOME");

	  if (!home) {
		  DPRINT_ERR("no home found");
		  goto end;
	  }
	  strcpy(yg_user_dir, home);
	  strcpy(yg_user_dir + strlen(home), "/.yirl/");
	  yuiMkdir(yg_user_dir);
  }

  if (ygInit(&cfg) < 0)
    goto end;
  if (ygStartLoop(&cfg) < 0)
    goto end;
  ret = 0;
 end:
  ygEnd();
  ygCleanGameConfig(&cfg);

  return ret;
}
