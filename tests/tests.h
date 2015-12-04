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

#ifndef _TESTS_H_
#define _TESTS_H_

void testBlockArray(void);

void testLifecycleSimple(void);
void testLifecycleFlow(void);
void testLifecycleComplex(void);
void testLifecycleAkwarde(void);
void testLifeDeathRebirdAndAgain(void);

void testSetSimple(void);
void testSetComplex(void);
void testSetGeneric(void);

void testLuaScritLifecycle(void);
void testLuaScritEntityBind(void);

void testTccScritLifecycle(void);

void testJsonLoadFile(void);
void testJsonMultipleObj(void);

void testSdlLife(void);
void testCursesLife(void);
void testAllLife(void);

void testYWTextScreenCurses(void);
void testYWTextScreenSdl2(void);
void testYWTextScreenAll(void);

void testYWMenuCurses(void);
void testYWMenuSdl2(void);

void testYWMapCurses(void);
void testYWMapSdl2(void);

void testVerticalContenerSdl(void);

void testYGameSdlLibBasic(void);
void testYGameAllLibBasic(void);

#endif
