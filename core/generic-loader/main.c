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
  const char *start_dir = NULL;
  const GOptionEntry entries[6] = {
    {"render", 'r', 0,  G_OPTION_ARG_STRING, &render,
     "choose render('sdl2' or curses), default: sdl", NULL},
    {"start", 's', 0,  G_OPTION_ARG_STRING, &start,
     "starting module", NULL},
    {"name", 'n', 0,  G_OPTION_ARG_STRING, &name, "window name", NULL},
    {"default-tcc-path", 0, 0,  G_OPTION_ARG_NONE, &default_tcc_path,
     "set this if tcc files are not in start directory", NULL},
    {"start-dir", 'd', 0,  G_OPTION_ARG_STRING, &start_dir,
     "allow to cp on the given directorry,"
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

  if (!render)
    render = "sdl2";

  if (start_dir) {
    if (!default_tcc_path) {
      ysTccPath = start_dir;
    }
    chdir(start_dir);
    if (!start)
      start = "./";
  }

  if (!start) {
    printf("start is not set\n");
    return 1;
  }

  ygInitGameConfig(&cfg, start, render);
  cfg.win_name = name;
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
