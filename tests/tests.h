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

#include <glib.h>

void testBlockArray(void);

void testLifecycleSimple(void);
void testLifecycleFlow(void);
void testLifecycleComplex(void);
void testLifecycleAkwarde(void);
void testLifeDeathRebirdAndAgain(void);
void testAutoFree(void);

void stringsTests(void);

void testCopy(void);
void testEntityPatch(void);

void testGet(void);
void testGetByEntity(void);

void testSetSimple(void);
void testSetComplex(void);
void testSetGeneric(void);

void ysciptAdd(void);
void yscriptLoop(void);
void yscriptBenchLoop(void);
void ysciptAddFunction(void);
void ybytecodeScript(void);
void ybytecodeAddFunction(void);
void ybytecodeLoopCallFunction(void);
void ybytecodeConditions(void);
void ybytecodeReadFile(void);

void testLuaScritLifecycle(void);
void testLuaScritEntityBind(void);

void testS7ScriptCall(void);

void testTccScritLifecycle(void);
void testTccTestsMacros(void);
void testTccAddDefine(void);

void testScriptAddFunction(void);

void testJsonLoadFile(void);
void testJsonMultipleObj(void);
void testJsonToFile(void);

void testRawFileLoad(void);

void testSdlLife(void);
void testCursesLife(void);
void testAllLife(void);

void testYWTextScreenCurses(void);
void testYWTextScreenSdl2(void);
void testYWTextScreenAll(void);
void fmtTxtScreen(void);

void testCanvasSdl2(void);

void testYWMenuCurses(void);
void testYWMenuSdl2(void);
void testPanelMenuSdl2(void);

void testYWMapCurses(void);
void testYWMapSdl2(void);
void testYWMapAll(void);

void testYBigWMapSdl2(void);

void testHorizontalContainerSdl(void);
void testVerticalContainerSdl(void);
void testStackContainerSdl(void);
void testMixContainerSdl(void);
void testDynamicStackContainerSdl(void);

void testYGameLifecycle(void);
void testYGameSdlLibBasic(void);
void testYGameAllLibBasic(void);

void testYSoundLib(void);

void testListMod(void);

void testsTiled(void);
void testsLpcSpritesheet(void);

void testDialogueBox(void);
void testDialogueMod(void);
void testDialogueCanvasMod(void);
void testTextInputMod(void);
void testSukeFightMod(void);

void testMazeGenMod(void);
#endif
