/*
**Copyright (C) 2018 Matthias Gatto
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

#include <string.h>
#include <glib.h>
#include <ctype.h>
#include "tcc-script.h"
#include "texture.h"
#include "canvas.h"

#include "text-screen.h"
#include "condition.h"
#include "container.h"
#include "entity-script.h"
#include "game.h"
#include "events.h"
#include "menu.h"
#include "simple-net.h"

void	fflushout(void)
{
  fflush(stdout);
}

#define ADD_SYM(s)				\
  tcc_add_symbol(l, #s, s)

void	tccAddSyms(TCCState *l)
{
  tcc_add_symbol(l, "free", free);
  tcc_add_symbol(l, "strdup", strdup);
  tcc_add_symbol(l, "printf", printf);
  tcc_add_symbol(l, "ygFileToEnt", ygFileToEnt);
  tcc_add_symbol(l, "ywMapSetSmootMovement", ywMapSetSmootMovement);
  tcc_add_symbol(l, "ywMapPushElem", ywMapPushElem);
  tcc_add_symbol(l, "ywMapGetCase", ywMapGetCase);
  tcc_add_symbol(l, "ywMapMvTableRemove", ywMapMvTableRemove);
  tcc_add_symbol(l, "yuiRand", yuiRand);
  tcc_add_symbol(l, "ywPosCreateInts", ywPosCreateInts);
  tcc_add_symbol(l, "ywidNext", ywidNext);
  tcc_add_symbol(l, "yuiRandInit", yuiRandInit);
  tcc_add_symbol(l, "ywMapCreateDefaultEntity", ywMapCreateDefaultEntity);
  tcc_add_symbol(l, "ywMapPushNbr", ywMapPushNbr);
  tcc_add_symbol(l, "ywRectReCreateInts", ywRectReCreateInts);
  tcc_add_symbol(l, "ywMapGetIdByElem", ywMapGetIdByElem);
  tcc_add_symbol(l, "ywMapAdvenceWithPos", ywMapAdvenceWithPos);
  tcc_add_symbol(l, "yeGetByStrFast", yeGetByStrFast);
  tcc_add_symbol(l, "yBlockArrayIteratorCreate", yBlockArrayIteratorCreate);
  tcc_add_symbol(l, "memmove", memmove);
  tcc_add_symbol(l, "yBlockArrayIteratorIncr", yBlockArrayIteratorIncr);
  tcc_add_symbol(l, "yeGetString", yeGetString);
  tcc_add_symbol(l, "strlen", strlen);
  tcc_add_symbol(l, "snprintf", snprintf);
  tcc_add_symbol(l, "yeRenamePtrStr", yeRenamePtrStr);
  tcc_add_symbol(l, "ywTextureNewImg", ywTextureNewImg);
  tcc_add_symbol(l, "ywRectCreateInts", ywRectCreateInts);
  tcc_add_symbol(l, "yeCreateArrayByCStr", yeCreateArrayByCStr);
  tcc_add_symbol(l, "yeCreateArrayAt", yeCreateArrayAt);
  tcc_add_symbol(l, "ywCanvasNewObj", ywCanvasNewObj);
  tcc_add_symbol(l, "yeGetInt", yeGetInt);
  tcc_add_symbol(l, "ywCanvasNewImgFromTexture", ywCanvasNewImgFromTexture);
  tcc_add_symbol(l, "ywCanvasRotate", ywCanvasRotate);
  tcc_add_symbol(l, "yeLen", yeLen);
  tcc_add_symbol(l, "ygGet", ygGet);
  tcc_add_symbol(l, "yeStrCmp", yeStrCmp);
  tcc_add_symbol(l, "yeStrCaseCmp", yeStrCaseCmp);
  tcc_add_symbol(l, "yeToLower", yeToLower);
  tcc_add_symbol(l, "yeCreateString", yeCreateString);
  tcc_add_symbol(l, "yeCreateInt", yeCreateInt);
  tcc_add_symbol(l, "yeCreateFloat", yeCreateFloat);
  tcc_add_symbol(l, "yeDestroy", yeDestroy);
  tcc_add_symbol(l, "yePushBack", yePushBack);
  tcc_add_symbol(l, "ygGetManager", ygGetManager);
  tcc_add_symbol(l, "yeCreateFunction", yeCreateFunction);
  tcc_add_symbol(l, "yeGetFloat", yeGetFloat);
  tcc_add_symbol(l, "yeGetByIdx", yeGetByIdx);
  tcc_add_symbol(l, "yeSetInt", yeSetInt);
  tcc_add_symbol(l, "yesCallInt", yesCallInt);
  tcc_add_symbol(l, "yeCheckCondition", yeCheckCondition);
  tcc_add_symbol(l, "ywCntGetEntry", ywCntGetEntry);
  tcc_add_symbol(l, "ywidActions", ywidActions);
  tcc_add_symbol(l, "ywContainerUpdate", ywContainerUpdate);
  tcc_add_symbol(l, "ywCntWidgetFather", ywCntWidgetFather);
  tcc_add_symbol(l, "ywidAddSubType", ywidAddSubType);
  tcc_add_symbol(l, "ygGetTccManager", ygGetTccManager);
  tcc_add_symbol(l, "ywMenuDown", ywMenuDown);
  tcc_add_symbol(l, "yeCreateData", yeCreateData);
  tcc_add_symbol(l, "ywidNewWidget", ywidNewWidget);
  tcc_add_symbol(l, "ygGetMod", ygGetMod);
  tcc_add_symbol(l, "yeAttach", yeAttach);
  tcc_add_symbol(l, "yePushAt", yePushAt);
  tcc_add_symbol(l, "yeGetByStr", yeGetByStr);
  tcc_add_symbol(l, "yeGetData", yeGetData);
  tcc_add_symbol(l, "yeRemoveChildByEntity", yeRemoveChildByEntity);
  tcc_add_symbol(l, "yeSetString", yeSetString);
  tcc_add_symbol(l, "ygRegistreFuncInternal", ygRegistreFuncInternal);
  tcc_add_symbol(l, "yePushBackExt", yePushBackExt);
  tcc_add_symbol(l, "yeRemoveChildByStr", yeRemoveChildByStr);
  tcc_add_symbol(l, "ywCanvasNewImg", ywCanvasNewImg);
  tcc_add_symbol(l, "fflushout", fflushout);
  tcc_add_symbol(l, "yeStringAddInt", yeStringAddInt);
  tcc_add_symbol(l, "strcmp", strcmp);
  tcc_add_symbol(l, "yProgramArg", &yProgramArg);
  tcc_add_symbol(l, "ywCanvasDisableWeight", ywCanvasDisableWeight);
  tcc_add_symbol(l, "ywCanvasSetWeight", ywCanvasSetWeight);
  tcc_add_symbol(l, "ywTurnId", ywTurnId);
  tcc_add_symbol(l, "ywidTurnTimer", &ywidTurnTimer);
  ADD_SYM(yeStringAdd);
  ADD_SYM(yevMouseDown);
  ADD_SYM(yeStringAddCh);
  ADD_SYM(yeStringAdd);
  ADD_SYM(ywidSetMainWid);
  ADD_SYM(ywCanvasStringSet);
  ADD_SYM(ywCanvasRemoveObj);
  ADD_SYM(ygTerminate);
  ADD_SYM(ywCanvasNewTextByStr);
  ADD_SYM(ywCanvasEnableWeight);
  ADD_SYM(ygLoadMod);
  ADD_SYM(ywSetTurnLengthOverwrite);
  ADD_SYM(ywGetTurnLengthOverwrite);
  ADD_SYM(yeSetIntAtStrIdx);
  ADD_SYM(ywCanvasNewImgByPath);
  ADD_SYM(ywCanvasMoveObjXY);
  ADD_SYM(ywCanvasObjPos);
  ADD_SYM(ywCanvasObjSize);
  ADD_SYM(ywCanvasNewRect);
  ADD_SYM(ywCanvasObjSetPos);
  ADD_SYM(ywCanvasObjectsCheckColisions);
  ADD_SYM(ywCanvasNewText);
  ADD_SYM(ywCanvasObjSetPosByEntity);
  ADD_SYM(ygReCreateInt);
  ADD_SYM(ygSetInt);

  ADD_SYM(yevMousePos);
  ADD_SYM(yevIsKeyUp);
  ADD_SYM(yevIsKeyDown);
  ADD_SYM(ywtextScreenResetTimer);
  ADD_SYM(ywidAction);
  ADD_SYM(yeStringReplaceCharAt);
  ADD_SYM(yeStringReplaceStrAt);
  ADD_SYM(ywTextScreenReformat);
  ADD_SYM(yeStrChrIdx);
  ADD_SYM(yeStrIsRangeChr);
  ADD_SYM(yeStrIsRangeStr);
  ADD_SYM(yeStrDoesRangeContainChr);
  ADD_SYM(ywMenuGetCurrentEntry);
  ADD_SYM(yeGetKeyAt);
  ADD_SYM(yeArrayIdx_str);
  ADD_SYM(yeStrDoesRangeContainStr);
  ADD_SYM(ygTerminate);
  ADD_SYM(yeClearArray);
  ADD_SYM(yeSetIntAt);
  ADD_SYM(yePopBack);
  ADD_SYM(yeCreateStringAt);
  ADD_SYM(yeCreateIntAt);
  ADD_SYM(yeSetStringAt);
  ADD_SYM(ySoundPlay);
  ADD_SYM(yevIsGrpDown);
  ADD_SYM(yevIsGrpUp);
  ADD_SYM(yeStringAddCh);
  ADD_SYM(ygGetLuaManager);
  ADD_SYM(ySoundLoad);
  ADD_SYM(yeCreateInts_);
  ADD_SYM(ywMenuSetCurrentEntry);
  ADD_SYM(ySoundMusicLoad);
  ADD_SYM(yeToCStr);
  ADD_SYM(abort);
  ADD_SYM(ywidGetMainWid);
  ADD_SYM(ywidGenericPollEvent);
  ADD_SYM(memset);
  ADD_SYM(ywCanvasClear);
  ADD_SYM(yeTokString);
  ADD_SYM(malloc);
  ADD_SYM(realloc);
  ADD_SYM(yeSetDestroy);
  ADD_SYM(g_get_monotonic_time);
  ADD_SYM(yeStringNextTok);
  ADD_SYM(strtol);
  ADD_SYM(atoi);
  ADD_SYM(ywCanvasNewBicolorImg);
  ADD_SYM(ywCanvasForceSize);
  ADD_SYM(ywCanvasNewTextExt);
  ADD_SYM(ywidDrawScreen);
  ADD_SYM(ywidRend);
  ADD_SYM(abs);
  ADD_SYM(yeMultDestroy_);
  ADD_SYM(sqrt);
  ADD_SYM(ywCanvasNewCollisionsArrayWithRectangle);
  ADD_SYM(ywMapInitEntity);
  ADD_SYM(ywMenuPushEntryByEnt);
  ADD_SYM(yeCreateFunctionExt);
  ADD_SYM(yeMapPixielsToPos);
  ADD_SYM(ywContainerGetWidgetAt);
  ADD_SYM(ywMapPushNbrXY);
  ADD_SYM(ywMapCaseXY);
  ADD_SYM(yeGetLast);
  ADD_SYM(ywMapSmootMove);
  ADD_SYM(strcpy);
  ADD_SYM(strncpy);
  ADD_SYM(ysCallInt);
  ADD_SYM(ysFastCall);
  ADD_SYM(yuiLongExit);
  ADD_SYM(yuiTryMain);
  ADD_SYM(ysTccGetSym);
  ADD_SYM(ysTccPushSym);
  ADD_SYM(ysTccPushSysincludePath);
  ADD_SYM(isdigit);
  ADD_SYM(usleep);
#if defined(__unix__) || defined(__APPLE__)
  tcc_add_symbol(l, "yuiDebugPrint", yuiDebugPrint);
  ADD_SYM(fprintf);
  ADD_SYM(stderr);
#endif
}
