#ifdef IN_CALL
#define BIND_NONE	BIND
#define BIND_B_EES	BIND
#define BIND_E_EIIE	BIND
#else
#define BIND_NONE(a, b, c)
#endif

BIND_NONE(ywPosCreate, 2, 2);
BIND_NONE(yeGet, 2, 0);

BIND_B_EES(yePushBack, 2, 1);

BIND_E_EIIE(ywCanvasNewRect, 2, 2);

#undef BIND_E_EIIE
#undef BIND_B_EES
#undef BIND_NONE
#undef IN_CALL

