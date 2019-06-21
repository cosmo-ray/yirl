#ifdef IN_CALL
#define BIND_NONE	BIND
#define BIND_I_V(a)	BIND(a,0,0)
#define BIND_I_I	BIND
#define BIND_I_S	BIND
#define BIND_I_E	BIND
#define BIND_I_EE	BIND
#define BIND_S_E	BIND
#define BIND_E_S	BIND
#define BIND_V_I	BIND
#define BIND_V_E	BIND
#define BIND_V_EI	BIND
#define BIND_V_EE	BIND
#define BIND_V_EII	BIND
#define BIND_E_E	BIND
#define BIND_E_EE	BIND
#define BIND_E_ES	BIND
#define BIND_E_EI	BIND
#define BIND_B_EE	BIND
#define BIND_B_EES	BIND
#define BIND_B_EEE	BIND
#define BIND_E_SEES	BIND
#define BIND_E_EIIE	BIND
#define BIND_E_EIIS	BIND
#define BIND_B_EEEE	BIND
#define BIND_E_EIIEE    BIND
#else

#define PUSH_I_GLOBAL(X)
#define BIND_NONE(a, b, c)

#endif

BIND_NONE(ywPosCreate, 2, 2);
BIND_NONE(yeGet, 2, 0);
BIND_NONE(yeGetIntAt, 2, 0);
BIND_NONE(yeSetIntAt, 3, 0);

BIND_B_EES(yePushBack, 2, 1);
BIND_B_EES(ywCanvasCreateYTexture, 3, 0);

BIND_B_EEEE(ywCanvasCheckCollisions, 3, 1);

BIND_E_EIIEE(ywCanvasNewImgFromTexture, 4, 1);

BIND_E_EIIE(ywCanvasNewRect, 2, 2);
BIND_E_EIIE(ywCanvasNewText, 2, 2);

BIND_E_EIIS(ywCanvasNewTextByStr, 2, 2);
BIND_E_EIIS(ywCanvasNewImgByPath, 4, 0);

BIND_I_S(ySoundLoad, 1, 0);
BIND_I_S(ySoundMusicLoad, 1, 0);

BIND_I_I(ySoundPlay, 1, 0);

BIND_B_EE(yevIsGrpUp, 2, 0);
BIND_B_EE(yevIsGrpDown, 2, 0);

BIND_E_ES(yeStringAdd, 2, 0);
BIND_E_ES(yeSetString, 2, 0);

BIND_E_EI(yeStringAddInt, 2, 0);
BIND_E_EI(yeStringAddLong, 2, 0);

BIND_E_E(ywCanvasObjPos, 1, 0);
BIND_E_E(ywidNextEve, 1, 0);
BIND_E_E(yeIncrRef, 1, 0);

BIND_E_S(ygGet, 1, 0);

BIND_V_I(ywSetTurnLengthOverwrite, 1, 0);

BIND_V_E(ywPosPrint, 1, 0);
BIND_V_E(ywSizePrint, 1, 0);

BIND_V_EE(ywCanvasStringSet, 2, 0);
BIND_V_EE(yeRemoveChildByEntity, 2, 0);
BIND_V_EE(ywCanvasRemoveObj, 2, 0);

BIND_NONE(yeIncrAt, 2, 0);
BIND_NONE(yeAddAt, 3, 0);

BIND_S_E(ywPosToString, 1, 0);
BIND_S_E(ywSizeToString, 1, 0);
BIND_S_E(ywRectToString, 1, 0);
BIND_S_E(yeGetString, 1, 0);

BIND_I_EE(ywCanvasForceSize, 2, 0);

BIND_I_E(yeRefCount, 1, 0);
BIND_I_E(yeType, 1, 0);
BIND_I_E(ywPosY, 1, 0);
BIND_I_E(ywPosX, 1, 0);
BIND_I_E(ywidAddSubType, 1, 0);
BIND_I_E(yeGetInt, 1, 0);
BIND_I_E(yeLen, 1, 0);
BIND_I_E(ywidEveKey, 1, 0);
BIND_I_E(ywidEveType, 1, 0);

BIND_V_EI(yeSetInt, 2, 0);

BIND_V_EII(ywCanvasMoveObjXY, 3, 0);
BIND_V_EII(ywCanvasObjSetPos, 3, 0);

BIND_I_V(ywGetTurnLengthOverwrite);
BIND_I_V(yWindowWidth);
BIND_I_V(yWindowHeight);

BIND_E_SEES(ywTextureNewImg, 4, 0);

BIND_NONE(yevCreateGrp, 1, 9);
BIND_NONE(yesCall, 1, 15);

PUSH_I_GLOBAL(Y_ESC_KEY);
PUSH_I_GLOBAL(Y_UP_KEY);
PUSH_I_GLOBAL(Y_DOWN_KEY);
PUSH_I_GLOBAL(Y_LEFT_KEY);
PUSH_I_GLOBAL(Y_RIGHT_KEY);

PUSH_I_GLOBAL(Y_LSHIFT_KEY);
PUSH_I_GLOBAL(Y_RSHIFT_KEY);
PUSH_I_GLOBAL(Y_LCTRL_KEY);
PUSH_I_GLOBAL(Y_RCTRL_KEY);

PUSH_I_GLOBAL(YKEY_DOWN);
PUSH_I_GLOBAL(YKEY_UP);

PUSH_I_GLOBAL(Y_REQUEST_ANIMATION_FRAME);

#undef BIND_E_EIIEE
#undef BIND_E_EIIS
#undef BIND_E_EIIE
#undef BIND_B_EEEE
#undef BIND_E_SEES
#undef BIND_B_EES
#undef BIND_B_EEE
#undef BIND_B_EE
#undef BIND_E_EE
#undef BIND_E_ES
#undef BIND_E_EI
#undef BIND_V_EI
#undef BIND_V_EII
#undef BIND_NONE
#undef BIND_V_I
#undef BIND_V_E
#undef BIND_V_EE
#undef BIND_E_S
#undef BIND_S_E
#undef BIND_I_E
#undef BIND_I_EE
#undef BIND_I_I
#undef BIND_I_S
#undef BIND_E_E
#undef BIND_I_V
#undef PUSH_I_GLOBAL
#undef IN_CALL

