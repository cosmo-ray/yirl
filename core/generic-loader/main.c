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

#include <glib.h>
#include <unistd.h>
#include "tcc-script.h"
#include "game.h"

int main(int argc, char **argv)
{
  GameConfig cfg;
  int ret = 1;
  GOptionContext *ctx;
  int default_tcc_path = 0;
  const char *render = NULL;
  const char *start = NULL;
  const char *name = NULL;
  char *binaryRootPath = NULL;
  const char *start_dir = NULL;
  char buf[PATH_MAX];
  char *cpath = getcwd(buf, PATH_MAX);
  const char *tcc_path = cpath;
  int width = -1;
  int height = -1;
  int render_need_free = 1;
  int start_need_free = 1;
  const GOptionEntry entries[10] = {
    {"render", 'r', 0,  G_OPTION_ARG_STRING, &render,
     "choose render('sdl2' or curses), default: sdl", NULL},
    {"start", 's', 0,  G_OPTION_ARG_STRING, &start,
     "starting module, default: current dit", NULL},
    {"name", 'n', 0,  G_OPTION_ARG_STRING, &name, "window name", NULL},
    {"width", 'W', 0,  G_OPTION_ARG_INT, &width, "window width", NULL},
    {"height", 'H', 0,  G_OPTION_ARG_INT, &height, "window height", NULL},
    {"arg", 0, 0,  G_OPTION_ARG_STRING, &yProgramArg, "program argument", NULL},
    {"default-tcc-path", 0, 0,  G_OPTION_ARG_NONE, &default_tcc_path,
     "set this if tcc files are not in start directory", NULL},
    {"binary-root-path", 0, 0,  G_OPTION_ARG_STRING, &binaryRootPath,
     "set path to binary directory(which contain, tcc, script-dependancies, and defaults polices)", NULL},
    {"start-dir", 'd', 0,  G_OPTION_ARG_STRING, &start_dir,
     "move on the given directorry,"
     " use as starting module if --start not set", NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };
  GError *error = NULL;

  ctx = g_option_context_new(NULL);
  g_option_context_set_help_enabled(ctx, 1);
  g_option_context_add_main_entries(ctx, entries, NULL);
  if (!g_option_context_parse(ctx, &argc, &argv, &error)) {
    printf("option parsing failed: %s\n", error->message);
    g_option_context_free(ctx);
    return 1;
  }
  g_option_context_free(ctx);

  if (!render) {
    render = "sdl2";
    render_need_free = 0;
  }

  if (start_dir) {
    tcc_path = start_dir;
    chdir(start_dir);
    cpath = getcwd(buf, PATH_MAX);
  }

  if (!default_tcc_path) {
      ysTccPath = tcc_path;
  }

  if (!start) {
    start = cpath;
    start_need_free = 0;
  }

  ygInitGameConfig(&cfg, start, render);
  cfg.win_name = name;
  if (width > 0)
    cfg.w = width;
  if (height > 0)
    cfg.h = height;

  if (binaryRootPath) {
    ygBinaryRootPath = binaryRootPath;
  }

  if (ygInit(&cfg) < 0)
    goto end;
  if (ygStartLoop(&cfg) < 0)
    goto end;
  ret = 0;
 end:
  ygEnd();
  ygCleanGameConfig(&cfg);

  if (render_need_free)
    g_free((char *)render);
  if (start_need_free)
    g_free((char *)start);

  g_free((char *)start_dir);
  g_free((char *)name);
  if (binaryRootPath) {
    ygBinaryRootPathFree();
  }
  return ret;
}
