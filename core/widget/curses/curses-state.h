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

#ifndef CURSES_STATE_H_
#define CURSES_STATE_H_

#include <curses.h>
#include "widget.h"
#include "curses-driver.h"
#include "debug.h"

typedef struct
{
  WINDOW *win;
  YWidgetState *wid;
  int h;
  int w;
  int x;
  int y;
} CWidget;

/* private curses func */
void resize(YWidgetState *wid, int renderType);
void CWidgetDestroy(YWidgetState *wid, int renderType);
void CWidgetInit(YWidgetState *wid, int renderType);
Entity *CWaitEvent(void);
Entity *CPollEvent(void);

#endif
