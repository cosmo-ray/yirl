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

#include <stdio.h>
#include <entity.h>
#include <widget.h>
#include <game.h>

void *load_map(int nb, void **args)
{
  printf("hi from map loader: %d - %s\n", nb, args[0]);
  return 0x32;
}

void *init_sm_reader(int nbArg, void **args)
{
  Entity *t = YE_TO_ENTITY(args[0]);
  Entity *f = yeCreateFunction("load_map", 1, ygGetManager("tcc"), t, "load-map");

  return NULL;
}

