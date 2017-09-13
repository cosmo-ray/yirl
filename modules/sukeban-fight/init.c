/*
**Copyright (C) 2017 Matthias Gatto
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

#include <yirl/game.h>

void *sukeFightInit(int nbArg, void **args)
{
  Entity *ent = args[0];
  Entity *entries = yeCreateArray(ent, "entries");
  Entity *canvas;
  Entity *menu;
  Entity *menu_entries;
  Entity *menu_entry;
  void *ret;

  printf("new sf wid\n");
  canvas = yeCreateArray(entries, NULL);
  yeCreateString("canvas", canvas, "<type>");
  yeCreateInt(70, canvas, "size");
  menu = yeCreateArray(entries, NULL);
  yeCreateString("menu", menu, "<type>");
  menu_entries = yeCreateArray(menu, "entries");
  menu_entry = yeCreateArray(menu_entries, NULL);
  yeCreateString("attack", menu_entry, "text");
  ret = ywidNewWidget(ent, "container");
  return ret;
}

void *init_sukeban_fight(int nbArg, void **args)
{
  Entity *mod = args[0];
  Entity *init = yeCreateArray(NULL, NULL);

  yeCreateString("sukeban-fight", init, "name");
  yeCreateFunction("sukeFightInit", ygGetManager("tcc"), init, "callback");
  ywidAddSubType(init);

  return NULL;
}
