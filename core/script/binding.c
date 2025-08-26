#ifdef IN_CALL
#define BIND_NONE	BIND
#define BIND_V(a)	BIND(a,0,0)
#define BIND_I		BIND
#define BIND_S		BIND
#define BIND_E		BIND
#define BIND_EE		BIND
#define BIND_II		BIND
#define BIND_ES		BIND
#define BIND_EI		BIND
#define BIND_SI		BIND
#define BIND_SE		BIND
#define BIND_SS		BIND
#define BIND_SES	BIND
#define BIND_EII	BIND
#define BIND_ESS	BIND
#define BIND_EEI	BIND
#define BIND_ESI	BIND
#define BIND_IIS	BIND
#define BIND_IIE	BIND
#define BIND_III	BIND
#define BIND_ISS	BIND
#define BIND_ISE	BIND
#define BIND_IES	BIND
#define BIND_ESE	BIND
#define BIND_EES	BIND
#define BIND_EEE	BIND
#define BIND_SEES	BIND
#define BIND_EIII	BIND
#define BIND_EIIE	BIND
#define BIND_EIIS	BIND
#define BIND_EEEE	BIND
#define BIND_EEIS	BIND
#define BIND_EIES	BIND
#define BIND_EESS	BIND
#define BIND_EEES	BIND
#define BIND_EEEI	BIND
#define BIND_IIES	BIND
#define BIND_EEEEI	BIND
#define BIND_EEEEE	BIND
#define BIND_EEESI	BIND
#define BIND_EIIEE	BIND
#define BIND_EIIII	BIND
#define BIND_EIIIS	BIND
#define BIND_EIIIIS	BIND
#define BIND_EIIISI	BIND
#define BIND_EIIIISI	BIND
#else

#define PUSH_I_GLOBAL(X)
#define PUSH_I_GLOBAL_VAL(X, V)

#define BIND_NONE(a, b, c)

#endif

BIND_NONE(ywPosCreate, 2, 2);
BIND_NONE(yeGet, 2, 0);
BIND_NONE(yeGetIntAt, 2, 0);
BIND_NONE(yeSetIntAt, 3, 0);

BIND_ESS(yeSplitInto, 3, 0);

BIND_ESS(yeRenameStrStr, 3, 0);

BIND_EES(yeGetReplace, 3, 0);

BIND_EES(ywCanvasNewRectangleByRect, 3, 0);

BIND_EES(yeStoreAll, 2, 1);

BIND_EES(yaePushBack, 2, 1);
BIND_EES(yePushBack, 2, 1);
BIND_EES(yeReplaceBack, 3, 0);

BIND_EES(yePush, 2, 1);

BIND_EES(ywCanvasCreateYTexture, 1, 2);

BIND_EES(ywMapRemoveByStr, 3, 0);

BIND_ESE(ycoRepushObj, 3, 0);
BIND_ESE(ywMenuPushEntry, 2, 1);
BIND_ESE(ywMenuPushSlider, 2, 1);

BIND_EEE(ywidActions, 2, 1);
BIND_EEE(ywidAction, 2, 1);

BIND_EEE(ywCanvasSwapObj, 3, 0);

BIND_EEE(ywCanvasNewIntersectArray, 3, 0);

BIND_IES(ygAddModule, 2, 1);

BIND_IES(yaeInt, 2, 1);

BIND_IES(ygLoadScript2, 3, 0);
BIND_IES(ygEntToFile2, 3, 0);

BIND_EEIS(yePushAt2, 3, 0);

BIND_EEIS(yeInsertAt, 3, 0);

BIND_EIES(ywMapPushNbr, 3, 1);

BIND_IIES(yeCreateQuadInt2, 2, 2);

BIND_EEES(yeMoveByEntity, 3, 1);

BIND_EEES(ywRectCreatePosSize, 2, 2);

BIND_EEES(ywMapTryPushElem, 3, 1);
BIND_EEES(ywMapPushElem, 3, 1);

BIND_EESS(ygInitWidgetModule2, 4, 0);

BIND_EEEE(ywCanvasCheckCollisions, 2, 2);
BIND_EEEE(ywCanvasMergeTexture, 4, 0);
BIND_EEEE(ywMapMoveByEntity, 4, 0);

BIND_EEEE(ywTextureMergeTexture, 4, 0);
BIND_EEEE(ywTextureMergeUnsafe, 4, 0);

BIND_EEEI(ywPosIsInRectPS, 4, 0);

BIND_EEEEE(ywCanvasNewProjectedCollisionsArrayExt, 2, 3);

BIND_EEEEI(ywMapDrawSegment, 4, 1);

BIND_EEESI(ywMapPushElemAt, 5, 0);

BIND_EIIEE(ywCanvasNewImgFromTexture, 4, 1);
BIND_EIIEE(ywCanvasNewHeadacheImg, 5, 0);

BIND_EIIE(ywCanvasNewRect, 2, 2);

BIND_EIIE(ywMapContainEnt, 4, 0);
BIND_EIIS(ywMapContainStr, 4, 0);

BIND_EIIS(ywCanvasNewTextByStr, 2, 2);
BIND_EIIS(ywCanvasNewImgByPath, 4, 0);

BIND_S(ygModDirPath, 1, 0);
BIND_S(ySoundLoad, 1, 0);
BIND_S(ySoundMusicLoad, 1, 0);
BIND_S(ygModDir, 1, 0);

BIND_I(yuiAbs, 1, 0);
BIND_I(ySoundPlay, 1, 0);
BIND_I(ySoundPause, 1, 0);
BIND_I(ySoundStop, 1, 0);
BIND_I(ySoundDuration, 1, 0);
BIND_I(yuiUsleep, 1, 0);

BIND_EE(ywEBSWrapperPush, 2, 0);

BIND_EE(yeHashKey, 2, 0);

BIND_EE(yePrint2, 1, 1);

BIND_EE(ywCanvasMoveObj, 2, 0);

BIND_EE(ywCanvasObjSize, 2, 0);
BIND_EE(ywCanvasArrayPop, 2, 0);
BIND_EE(ywCanvasClearArray, 2, 0);

BIND_EE(ywCanvasObjectsCheckColisions, 2, 0);
BIND_EE(ywCanvasObjectsCheckPointColisions, 2, 0);

BIND_EE(ywCanvasObjReplacePos, 2, 0);

BIND_EE(ywCanvasNewCollisionsArray, 2, 0);

BIND_EE(yePushArrayBack, 2, 0);
BIND_EE(yeveCopyBack, 2, 0);

BIND_EE(ywMapSetCamPos, 2, 0);
BIND_EE(ywMapCase, 2, 0);
BIND_EE(ywMapPop, 2, 0);

BIND_EE(yecpy, 2, 0);

BIND_EE(yeEraseByE, 2, 0);

BIND_EE(yevIsGrpUp, 2, 0);
BIND_EE(yevIsGrpDown, 2, 0);
BIND_EE(yeFindKey, 2, 0);
BIND_EE(ywPosDistance, 2 ,0);
BIND_EE(ywPosYDistance, 2 ,0);
BIND_EE(ywPosXDistance, 2 ,0);
BIND_EE(ywPosTotCases, 2 ,0);
BIND_EE(yeCopy, 2 ,0);

BIND_EE(ywCanvasCheckColisionsRectObj, 2, 0);

BIND_EE(ywRemoveEntryByEntity, 2, 0);

BIND_EE(ywCanvasObjSetPosByEntity, 2, 0);

BIND_ES(ywCntCreateChild, 2, 0);

BIND_ES(ygCallFuncOrQuit, 2, 0);

BIND_ES(ycoRmObj, 2, 0);

BIND_ES(yeFindString, 2, 0);

BIND_ES(ywCanvasSetStrColor, 2, 0);

BIND_ES(yeStringAddNl, 2, 0);
BIND_ES(yeStringAdd, 2, 0);
BIND_ES(yeSetString, 2, 0);
BIND_ES(ygPushToGlobalScope, 2, 0);

BIND_ES(yeRemoveChildByStr, 2, 0);

BIND_ES(yeGetByStr, 2, 0);

BIND_SI(ygReCreateInt, 2, 0);

BIND_SI(ygIncreaseInt, 2, 0);
BIND_SI(ygSetInt, 2, 0);

BIND_EI(ywEBSSwapGroup, 2, 0);
BIND_EI(ywEBSGroupeDir, 2, 0);

BIND_EI(yeConvert, 2, 0);
BIND_EI(ywPosDoPercent, 2, 0);

BIND_EI(yeRemoveChildByIdx, 2, 0);

BIND_EI(yeGetFloatAtByIdx, 2, 0);

BIND_EI(yeIntBAnd, 2, 0);
BIND_EI(yeIntCheckBAnd, 2, 0);

BIND_EI(yeIntInfTo, 2, 0);
BIND_EI(yeIntSupTo, 2, 0);

BIND_EI(ywMapIdAt, 2, 0);

BIND_EI(ywMapCamAddX, 2, 0);
BIND_EI(ywMapCamAddY, 2, 0);

BIND_EI(ywMenuMove, 2, 0);

BIND_EI(ywCanvasMultiplySize, 2, 0);

BIND_EI(ywRectSetX, 2, 0);
BIND_EI(ywRectSetY, 2, 0);
BIND_EI(ywRectSetW, 2, 0);
BIND_EI(ywRectSetH, 2, 0);

BIND_EI(ywCanvasPercentReduce, 2, 0);

BIND_EI(yevIsKeyDown, 2, 0);
BIND_EI(yevIsKeyUp, 2, 0);

BIND_EI(yeStringAddInt, 2, 0);
BIND_EI(yeStringAddLong, 2, 0);

BIND_EI(ywCntGetEntry, 2, 0);

BIND_EI(yeGetKeyAt, 2, 0);

BIND_EI(ywMapCamPointedContainId, 2, 0);

BIND_SS(ygReCreateString, 2, 0);

BIND_E(ygModDirByEntity, 1, 0);

BIND_E(ywTextureNormalize, 1, 0);

BIND_E(ywTextureH, 1, 0);
BIND_E(ywTextureW, 1, 0);

BIND_E(ywCanvasObjAngle, 1, 0);

BIND_E(yeCheckCondition, 1, 0);

BIND_E(ywCntPopLastEntry, 1, 0);

BIND_E(yeCountLines, 1, 0);

BIND_E(ywCanvasPix0X, 1, 0);
BIND_E(ywCanvasPix0Y, 1, 0);

BIND_E(ywCanvasRemoveColorMod, 1, 0);
BIND_E(ywCanvasObjUnsetVFlip, 1, 0);
BIND_E(ywCanvasObjUnsetHFlip, 1, 0);

BIND_E(ywCanvasVFlip, 1, 0);
BIND_E(ywCanvasHFlip, 1, 0);

BIND_E(yevAnyMouseDown, 1, 0);
BIND_E(yevMousePos, 1, 0);

BIND_E(yeGetRandomElem, 1, 0);

BIND_E(ywMapCamPointedCase, 1, 0);
BIND_E(ywMapCam, 1, 0);

BIND_E(ywMenuGetCurrentByEntity, 1, 0);

BIND_E(ywMenuGetCurSliderSlide, 1, 0);

BIND_E(ywMenuGetCurrentEntry, 1, 0);

BIND_E(ywMenuNbEntries, 1, 0);

BIND_E(yePrint, 1, 0);
BIND_E(yePrintAddr, 1, 0);

BIND_E(ywidInTree, 1, 0);

BIND_E(ywCntParent, 1, 0);

BIND_E(ywCanvasStrColor, 1, 0);

BIND_E(yeShuffle, 1, 0);

BIND_E(ywSizeDistance, 1, 0);

BIND_E(yeNbElems, 1, 0);

BIND_E(yeLastKey, 1, 0);

BIND_E(ywMenuClear, 1, 0);

BIND_E(yeSetConst, 1, 0);

BIND_E(yeClearArray, 1, 0);

BIND_E(yeLast, 1, 0);

BIND_E(yePopBack, 1, 0);

BIND_E(ywCanvasObjPosX, 1, 0);
BIND_E(ywCanvasObjPosY, 1, 0);
BIND_E(ywCanvasObjPos, 1, 0);

BIND_E(ywidNextEve, 1, 0);
BIND_E(yeIncrRef, 1, 0);
BIND_E(ywCntWidgetFather, 1, 0);

BIND_E(ywidXMouse, 1, 0);
BIND_E(ywidYMouse, 1, 0);
BIND_E(ywidEveMousePos, 1, 0);

BIND_S(ygGet, 1, 0);
BIND_S(ygGetString, 1, 0);
BIND_S(ygGetInt, 1, 0);
BIND_S(ygRemoveFromGlobalScope, 1, 0);

BIND_E(ywCanvasDisableWeight, 1, 0);
BIND_E(ywCanvasEnableWeight, 1, 0);

BIND_E(ywCanvasDisableWeight2, 1, 0);
BIND_E(ywCanvasEnableWeight2, 1, 0);

BIND_I(yui0Min, 1, 0);

BIND_I(ywSetTurnLengthOverwrite, 1, 0);

BIND_E(yeUnsetFirst, 1, 0);
BIND_E(yeFirst, 1, 0);

BIND_E(ywPosPrint, 1, 0);
BIND_E(ywSizePrint, 1, 0);
BIND_E(ywCanvasClear, 1, 0);

BIND_EE(ywCanvasStringSet, 2, 0);
BIND_EE(yeRemoveChildByEntity, 2, 0);
BIND_EE(ywCanvasRemoveObj, 2, 0);

BIND_NONE(yeIncrAt, 2, 0);
BIND_NONE(yeAddAt, 3, 0);
BIND_EE(yeAddEnt, 2, 0);
BIND_EI(yeAddInt, 2, 0);

BIND_E(ywPosToString, 1, 0);
BIND_E(ywSizeToString, 1, 0);
BIND_E(ywRectToString, 1, 0);
BIND_E(yeGetString, 1, 0);

BIND_EE(ywCanvasForceSize, 2, 0);

BIND_EE(yeArrayIdx_ent, 2, 0);

BIND_E(yeRefCount, 1, 0);
BIND_E(yeType, 1, 0);
BIND_E(ywPosY, 1, 0);
BIND_E(ywPosX, 1, 0);
BIND_E(ywSizeW, 1, 0);
BIND_E(ywSizeH, 1, 0);

BIND_E(ywRectY, 1, 0);
BIND_E(ywRectX, 1, 0);
BIND_E(ywRectW, 1, 0);
BIND_E(ywRectH, 1, 0);

BIND_E(ywidAddSubType, 1, 0);
BIND_E(ywidAddSubTypeNF, 1, 0);
BIND_E(yeGetInt, 1, 0);
BIND_E(yeLen, 1, 0);
BIND_E(ywidEveKey, 1, 0);
BIND_E(ywidEveType, 1, 0);

BIND_II(ySoundLevel, 2, 0);

BIND_II(yuiMin, 2, 0);
BIND_II(yuiMax, 2, 0);

BIND_II(yuiPercentOf, 2, 0);

BIND_III(yuiMinMax, 3, 0);

BIND_EI(ywMapReset, 1, 1);

BIND_EI(yeSetInt, 2, 0);

BIND_SES(yaeString, 2, 1);

BIND_ESE(ygInitWidgetModule, 3, 0);

BIND_EII(ywSetGroupeSpeed, 3, 0);

BIND_EII(yeIntRoundBound, 3, 0);

BIND_EII(yeIntForceBound, 3, 0);

BIND_EII(ywMapCaseXY, 3, 0);

BIND_EII(ywCanvasMoveObjXY, 3, 0);

BIND_EII(ywPosSetInts, 3, 0);

BIND_EII(yeSubIntMin, 3, 0);

BIND_EII(ywPosSubXY, 3, 0);
BIND_EII(ywPosAddXY, 3, 0);
BIND_EII(ywPosMultXY, 3, 0);

BIND_EII(ywCanvasForceSizeXY, 3, 0);

BIND_EES(yePushFront, 2, 1);

BIND_EEI(yeQuickSort, 2, 1);
BIND_EEI(yeDumbSort, 2, 1);

BIND_EEI(ywRectContainPos, 3, 0);

BIND_EEI(yeMergeInto, 3, 0);

BIND_EEI(ywReplaceEntry2, 3, 0);

BIND_EEI(ywPosIsSameEnt, 2, 1);

BIND_EEI(ywMenuCallActionOnByEntity, 3, 0);

BIND_EEI(yePatchAplyExt, 2, 1);

BIND_EEI(ywPushNewWidget, 2, 1);

BIND_EEI(ywCanvasSetWeight, 3, 0);

BIND_EIII(ywPosAddXYAbsMax, 4, 0);

BIND_EIII(ywRectContain, 3, 1);

BIND_EIIII(ywCanvasSetColorModRGBA, 5, 0);

BIND_V(yeFreeEntitiesInStack);

BIND_V(ywidFontW);
BIND_V(ywidFontH);

BIND_V(yeEntitiesUsed);

BIND_V(ysTraceCurrentScript);

BIND_V(ygGetBinaryRootPath);

BIND_V(ygTerminate);

BIND_V(ygUserDir);
BIND_V(ywGetTurnLengthOverwrite);
BIND_V(yWindowWidth);
BIND_V(yWindowHeight);
BIND_V(ywidGetTurnTimer);
BIND_V(yeEntitiesArraySize);
BIND_V(ygModDirOut);
BIND_V(ywidRendMainWid);

BIND_V(ywidGenericPollEvent);

BIND_V(yuiRand);

BIND_V(yeveMouseX);
BIND_V(yeveMouseY);

BIND_V(ygUpdateScreen);

#ifndef NO_ywTextureNewImg
BIND_SEES(ywTextureNewImg, 4, 0);
#else
#undef NO_ywTextureNewImg
#endif

BIND_EIIIS(ywCanvasNewFilledCircle, 5, 0);
BIND_EIIIS(ywCanvasNewCircle, 5, 0);
BIND_EIIIS(ywCanvasNewHSegment, 5, 0);
BIND_EIIIS(ywCanvasNewVSegment, 5, 0);

BIND_EIIISI(ywCanvasNewCircleExt, 5, 1);
BIND_EIIIISI(ywCanvasNewRectangleExt, 6, 1);

BIND_EIIIIS(ywCanvasNewLine, 6, 0);
BIND_EIIIIS(ywCanvasNewRectangle, 6, 0);
BIND_EIIIIS(ywCanvasMergeRectangle, 6, 0);
BIND_EIIIIS(ywCanvasMergeText, 6, 0);

BIND_EIIIIS(ywTextureMergeText, 6, 0);

BIND_EIIIIS(ywTextureMergeRectangle, 6, 0);

BIND_NONE(yevCreateGrp, 1, 9);

PUSH_I_GLOBAL(YCANVAS_POS_IDX);
PUSH_I_GLOBAL(YCANVAS_UDATA_IDX);

PUSH_I_GLOBAL(Y_MOD_LOCAL);
PUSH_I_GLOBAL(Y_MOD_YIRL);

PUSH_I_GLOBAL(YSTRING);
PUSH_I_GLOBAL(YARRAY);
PUSH_I_GLOBAL(YINT);
PUSH_I_GLOBAL(YHASH);

PUSH_I_GLOBAL(Y_ESC_KEY);
PUSH_I_GLOBAL(Y_UP_KEY);
PUSH_I_GLOBAL(Y_DOWN_KEY);
PUSH_I_GLOBAL(Y_LEFT_KEY);
PUSH_I_GLOBAL(Y_RIGHT_KEY);


PUSH_I_GLOBAL_VAL(Y_0_KEY, '0');
PUSH_I_GLOBAL_VAL(Y_1_KEY, '1');
PUSH_I_GLOBAL_VAL(Y_2_KEY, '2');
PUSH_I_GLOBAL_VAL(Y_3_KEY, '3');
PUSH_I_GLOBAL_VAL(Y_4_KEY, '4');
PUSH_I_GLOBAL_VAL(Y_5_KEY, '5');
PUSH_I_GLOBAL_VAL(Y_6_KEY, '6');
PUSH_I_GLOBAL_VAL(Y_7_KEY, '7');
PUSH_I_GLOBAL_VAL(Y_8_KEY, '8');
PUSH_I_GLOBAL_VAL(Y_9_KEY, '9');

PUSH_I_GLOBAL_VAL(Y_A_KEY, 'a');
PUSH_I_GLOBAL_VAL(Y_B_KEY, 'b');
PUSH_I_GLOBAL_VAL(Y_C_KEY, 'c');
PUSH_I_GLOBAL_VAL(Y_D_KEY, 'd');
PUSH_I_GLOBAL_VAL(Y_E_KEY, 'e');
PUSH_I_GLOBAL_VAL(Y_F_KEY, 'f');
PUSH_I_GLOBAL_VAL(Y_G_KEY, 'g');
PUSH_I_GLOBAL_VAL(Y_H_KEY, 'h');
PUSH_I_GLOBAL_VAL(Y_I_KEY, 'i');
PUSH_I_GLOBAL_VAL(Y_J_KEY, 'j');
PUSH_I_GLOBAL_VAL(Y_K_KEY, 'k');
PUSH_I_GLOBAL_VAL(Y_L_KEY, 'l');
PUSH_I_GLOBAL_VAL(Y_M_KEY, 'm');
PUSH_I_GLOBAL_VAL(Y_N_KEY, 'n');
PUSH_I_GLOBAL_VAL(Y_O_KEY, 'o');
PUSH_I_GLOBAL_VAL(Y_P_KEY, 'p');
PUSH_I_GLOBAL_VAL(Y_Q_KEY, 'q');
PUSH_I_GLOBAL_VAL(Y_R_KEY, 'r');
PUSH_I_GLOBAL_VAL(Y_S_KEY, 's');
PUSH_I_GLOBAL_VAL(Y_T_KEY, 't');
PUSH_I_GLOBAL_VAL(Y_U_KEY, 'u');
PUSH_I_GLOBAL_VAL(Y_V_KEY, 'v');
PUSH_I_GLOBAL_VAL(Y_W_KEY, 'w');
PUSH_I_GLOBAL_VAL(Y_X_KEY, 'x');
PUSH_I_GLOBAL_VAL(Y_Y_KEY, 'y');
PUSH_I_GLOBAL_VAL(Y_Z_KEY, 'z');


PUSH_I_GLOBAL(Y_LSHIFT_KEY);
PUSH_I_GLOBAL(Y_RSHIFT_KEY);
PUSH_I_GLOBAL(Y_LCTRL_KEY);
PUSH_I_GLOBAL(Y_RCTRL_KEY);

PUSH_I_GLOBAL_VAL(Y_SPACE_KEY, ' ');
PUSH_I_GLOBAL_VAL(Y_ENTER_KEY, '\n');

PUSH_I_GLOBAL(YKEY_DOWN);
PUSH_I_GLOBAL(YKEY_UP);

PUSH_I_GLOBAL(YKEY_MOUSEDOWN);
PUSH_I_GLOBAL(YKEY_MOUSEUP);
PUSH_I_GLOBAL(YKEY_MOUSEWHEEL);
PUSH_I_GLOBAL(YKEY_MOUSEMOTION);

PUSH_I_GLOBAL(YMAP_DRAW_NO_DOUBLE);
PUSH_I_GLOBAL(YMAP_DRAW_COPY_ELEM);
PUSH_I_GLOBAL(YMAP_DRAW_REPLACE_FIRST);

PUSH_I_GLOBAL(Y_REQUEST_ANIMATION_FRAME);

PUSH_I_GLOBAL(YE_PATCH_NO_SUP);

PUSH_I_GLOBAL(YJSON);
PUSH_I_GLOBAL(YRAW_FILE);

PUSH_I_GLOBAL(YLUA);
PUSH_I_GLOBAL(YS7);
PUSH_I_GLOBAL(YTCC);
PUSH_I_GLOBAL(YPH7);
PUSH_I_GLOBAL(YPERL);

#undef BIND_EIIIISI
#undef BIND_EIIIIS
#undef BIND_EIIISI
#undef BIND_EIIIS
#undef BIND_EIIEE
#undef BIND_EEEEI
#undef BIND_EEEEE
#undef BIND_EEESI
#undef BIND_EIIII
#undef BIND_EEEI
#undef BIND_EIII
#undef BIND_EIIS
#undef BIND_EIIE
#undef BIND_EESS
#undef BIND_EEEE
#undef BIND_EEES
#undef BIND_IIES
#undef BIND_EEIS
#undef BIND_EIES
#undef BIND_SEES
#undef BIND_EES
#undef BIND_SES
#undef BIND_ISE
#undef BIND_ESE
#undef BIND_EEE
#undef BIND_EEI
#undef BIND_EII
#undef BIND_ESS
#undef BIND_ESI
#undef BIND_ISS
#undef BIND_IES
#undef BIND_IIS
#undef BIND_IIE
#undef BIND_III
#undef BIND_II
#undef BIND_EE
#undef BIND_ES
#undef BIND_EI
#undef BIND_SE
#undef BIND_SI
#undef BIND_SS
#undef BIND_NONE
#undef BIND_I
#undef BIND_V
#undef BIND_E
#undef BIND_S
#undef PUSH_I_GLOBAL
#undef PUSH_I_GLOBAL_VAL
#undef IN_CALL

