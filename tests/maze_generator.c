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
#include "tests.h"

void testMazeGenMod(void)
{
  Entity *maze_info;
  Entity *pos;

#define ymaze_create(maze_info, father, name)				\
  ysCall(ygGetTccManager(), "maze_create", maze_info, father, name)

  GameConfig cfg;

  g_assert(!ygInitGameConfig(&cfg, NULL, YNONE));
  g_assert(!ygInit(&cfg));
  g_assert(ygLoadMod(TESTS_PATH"../modules/maze/"));
  yuiRandInit();

  pos = ywPosCreateInts(0, 0, NULL, NULL);
  maze_info = yeCreateArray(NULL, NULL);
  yeCreateInt(0, maze_info, "type");
  yeCreateInt(25, maze_info, "width");
  yeCreateInt(20, maze_info, "height");
  yeCreateInt(4, maze_info, "nb_wall");
  yeCreateInt(1, maze_info, "wall_elem");
  /* maze = ymaze_create(maze_info, NULL, NULL); */
  /* g_assert(maze); */

  /* for (int i = 0; i < 20; ++i) { */
  /*   for (int j = 0; j < 25; ++j) { */
  /*     ywPosSet(pos, j, i); */
  /*     if (yeLen(ywMapGetCase(maze, pos))) */
  /* 	printf("#"); */
  /*     else */
  /* 	printf("."); */
  /*   } */
  /*   printf("\n"); */
  /* } */
  yeMultDestroy(maze_info, pos);
  ygEnd();
  ygCleanGameConfig(&cfg);
}
