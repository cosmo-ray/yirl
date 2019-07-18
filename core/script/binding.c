#ifdef IN_CALL
#define BIND_NONE	BIND
#define BIND_V(a)	BIND(a,0,0)
#define BIND_I	BIND
#define BIND_S	BIND
#define BIND_E	BIND
#define BIND_EE	BIND
#define BIND_ES	BIND
#define BIND_EI	BIND
#define BIND_EII	BIND
#define BIND_EEI	BIND
#define BIND_EES	BIND
#define BIND_EEE	BIND
#define BIND_SEES	BIND
#define BIND_EIIE	BIND
#define BIND_EIIS	BIND
#define BIND_EEEE	BIND
#define BIND_EIIEE    BIND
#define BIND_EIIIIS    BIND
#else

#define PUSH_I_GLOBAL(X)
#define PUSH_I_GLOBAL_VAL(X, V)

#define BIND_NONE(a, b, c)

#endif

BIND_NONE(ywPosCreate, 2, 2);
BIND_NONE(yeGet, 2, 0);
BIND_NONE(yeGetIntAt, 2, 0);
BIND_NONE(yeSetIntAt, 3, 0);

BIND_EES(yePushBack, 2, 1);

BIND_EES(ywCanvasCreateYTexture, 3, 0);

BIND_EEEE(ywCanvasCheckCollisions, 2, 2);
BIND_EEEE(ywCanvasMergeTexture, 4, 0);

BIND_EIIEE(ywCanvasNewImgFromTexture, 4, 1);

BIND_EIIE(ywCanvasNewRect, 2, 2);

BIND_EIIS(ywCanvasNewTextByStr, 2, 2);
BIND_EIIS(ywCanvasNewImgByPath, 4, 0);

BIND_S(ySoundLoad, 1, 0);
BIND_S(ySoundMusicLoad, 1, 0);

BIND_I(ySoundPlay, 1, 0);
BIND_I(ySoundPause, 1, 0);
BIND_I(ySoundStop, 1, 0);
BIND_I(ySoundDuration, 1, 0);

BIND_EE(yevIsGrpUp, 2, 0);
BIND_EE(yevIsGrpDown, 2, 0);

BIND_ES(yeStringAdd, 2, 0);
BIND_ES(yeSetString, 2, 0);
BIND_ES(ygPushToGlobalScope, 2, 0);

BIND_EI(yeStringAddInt, 2, 0);
BIND_EI(yeStringAddLong, 2, 0);

BIND_E(ywCanvasObjPos, 1, 0);
BIND_E(ywidNextEve, 1, 0);
BIND_E(yeIncrRef, 1, 0);

BIND_S(ygGet, 1, 0);
BIND_S(ygGetString, 1, 0);
BIND_S(ygGetInt, 1, 0);
BIND_S(ygRemoveFromGlobalScope, 1, 0);

BIND_E(ywCanvasDisableWeight, 1, 0);
BIND_E(ywCanvasEnableWeight, 1, 0);

BIND_I(ywSetTurnLengthOverwrite, 1, 0);

BIND_E(ywPosPrint, 1, 0);
BIND_E(ywSizePrint, 1, 0);
BIND_E(ywCanvasClear, 1, 0);

BIND_EE(ywCanvasStringSet, 2, 0);
BIND_EE(yeRemoveChildByEntity, 2, 0);
BIND_EE(ywCanvasRemoveObj, 2, 0);

BIND_NONE(yeIncrAt, 2, 0);
BIND_NONE(yeAddAt, 3, 0);

BIND_E(ywPosToString, 1, 0);
BIND_E(ywSizeToString, 1, 0);
BIND_E(ywRectToString, 1, 0);
BIND_E(yeGetString, 1, 0);

BIND_EE(ywCanvasForceSize, 2, 0);

BIND_E(yeRefCount, 1, 0);
BIND_E(yeType, 1, 0);
BIND_E(ywPosY, 1, 0);
BIND_E(ywPosX, 1, 0);
BIND_E(ywidAddSubType, 1, 0);
BIND_E(yeGetInt, 1, 0);
BIND_E(yeLen, 1, 0);
BIND_E(ywidEveKey, 1, 0);
BIND_E(ywidEveType, 1, 0);

BIND_EI(yeSetInt, 2, 0);

BIND_EII(ywCanvasMoveObjXY, 3, 0);

BIND_EEI(yePatchAplyExt, 2, 1);

BIND_V(ywGetTurnLengthOverwrite);
BIND_V(yWindowWidth);
BIND_V(yWindowHeight);
BIND_V(ywidGetTurnTimer);
BIND_V(yeEntitiesArraySize);

BIND_SEES(ywTextureNewImg, 4, 0);

BIND_EIIIIS(ywCanvasMergeRectangle, 6, 0);

BIND_EIIIIS(ywCanvasNewRectangle, 6, 0);

BIND_NONE(yevCreateGrp, 1, 9);

PUSH_I_GLOBAL(Y_ESC_KEY);
PUSH_I_GLOBAL(Y_UP_KEY);
PUSH_I_GLOBAL(Y_DOWN_KEY);
PUSH_I_GLOBAL(Y_LEFT_KEY);
PUSH_I_GLOBAL(Y_RIGHT_KEY);

PUSH_I_GLOBAL(Y_LSHIFT_KEY);
PUSH_I_GLOBAL(Y_RSHIFT_KEY);
PUSH_I_GLOBAL(Y_LCTRL_KEY);
PUSH_I_GLOBAL(Y_RCTRL_KEY);

PUSH_I_GLOBAL_VAL(Y_SPACE_KEY, ' ');

PUSH_I_GLOBAL(YKEY_DOWN);
PUSH_I_GLOBAL(YKEY_UP);

PUSH_I_GLOBAL(Y_REQUEST_ANIMATION_FRAME);

PUSH_I_GLOBAL(YE_PATCH_NO_SUP);

#undef BIND_EIIIIS
#undef BIND_EIIEE
#undef BIND_EIIS
#undef BIND_EIIE
#undef BIND_EEEE
#undef BIND_SEES
#undef BIND_EES
#undef BIND_EEE
#undef BIND_EEI
#undef BIND_EII
#undef BIND_EE
#undef BIND_ES
#undef BIND_EI
#undef BIND_NONE
#undef BIND_I
#undef BIND_V
#undef BIND_E
#undef BIND_S
#undef PUSH_I_GLOBAL
#undef PUSH_I_GLOBAL_VAL
#undef IN_CALL

