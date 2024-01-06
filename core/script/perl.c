/*
**Copyright (C) 2023 Matthias Gatto
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

#if PERL_ENABLE > 0

#include <yirl/all.h>
#include <stdlib.h>

static int loadFile(void *sm, const char *file);
static int loadString(void *sm, const char *str);
static int destroy(void *sm);
static int init(void *sm, void *args);
static int destroy(void *sm);
static void *call(void *sm, const char *name, int nb, union ycall_arg *args,
		  int *t_array);
#include <EXTERN.h>
#include <perl.h>

static int t = -1;

struct YPerlScript {
	YScriptOps ops;
	PerlInterpreter *my_perl;
};

/* It seems calloc segfault on MSYS2 if XS is include here */
static void *allocator(void)
{
	struct YPerlScript *ret;

	ret = calloc(1, sizeof *ret);
	if (ret == NULL)
		return NULL;
	ret->my_perl = NULL;
	ret->ops.init = init;
	ret->ops.destroy = destroy;
	ret->ops.loadFile = loadFile;
	ret->ops.loadString = loadString;
	ret->ops.call = call;
	ret->ops.trace = NULL;
	ret->ops.getError = NULL;
	ret->ops.registreFunc = NULL;
	ret->ops.addFuncSymbole = NULL;
	return (void *)ret;
}

#include <XSUB.h>

Entity *toFree;

XS(XS_ygFileToEnt)
{
	Entity *parent;
	dXSARGS;
	if (items > 2)
		parent = (void *)SvIV(ST(2));
	else
		parent = toFree;
	Entity *r = ygFileToEnt(SvIV(ST(0)), SvPVbyte_nolen(ST(1)), parent);

	XSRETURN_IV(PTR2IV(r));
}

XS(XS_yeCreateFunction)
{
	Entity *parent;
	dXSARGS;

	if (items > 1)
		parent = (void *)SvIV(ST(1));
	else
		parent = toFree;
	Entity *r = yeCreateFunction(SvPVbyte_nolen(ST(0)),
				     ygPerlManager(), parent,
				     items > 2 ? SvPVbyte_nolen(ST(2)) : NULL);
	XSRETURN_IV(PTR2IV(r));
}

XS(XS_yeCreateArray)
{
	Entity *parent;
	dXSARGS;

	if (items > 0)
		parent = (void *)SvIV(ST(0));
	else
		parent = toFree;
	Entity *r = yeCreateArray(parent, items > 1 ? SvPVbyte_nolen(ST(1)) : NULL);
	XSRETURN_IV(PTR2IV(r));
}

XS(XS_yeCreateInt)
{
	Entity *parent;
	char *key;
	dXSARGS;

	if (items < 2) {
		parent = toFree;
		key = NULL;
	} else {
		parent = (void *)SvIV(ST(1));
		key = items > 2 ? SvPVbyte_nolen(ST(2)) : NULL;
	}

	Entity *r = yeCreateInt(SvIV(ST(0)), parent, key);
	XSRETURN_IV(PTR2IV(r));
}

XS(XS_ywidNewWidget)
{
	dXSARGS;
	void *r = ywidNewWidget((void *)SvIV(ST(0)), SvPVbyte_nolen(ST(1)));
	XSRETURN_IV(PTR2IV(r));
}


XS(XS_yeCreateFloat)
{
	Entity *parent;
	const char *key;
	dXSARGS;

	if (items < 2) {
		parent = toFree;
		key = NULL;
	} else {
		parent = (void *)SvIV(ST(1));
		key = items > 2 ? SvPVbyte_nolen(ST(2)) : NULL;
	}

	Entity *r = yeCreateFloat(SvNV(ST(0)), parent, key);
	XSRETURN_IV(PTR2IV(r));
}

XS(XS_ywTextScreenNew)
{
	dXSARGS;
	Entity *r = ywTextScreenNew(SvPVbyte_nolen(ST(0)));

	yePushBack(toFree, r, NULL);
	yeDestroy(r);
	XSRETURN_IV(PTR2IV(r));
}


XS(XS_yeCreateString)
{
	Entity *parent;
	const char *key = NULL;
	dXSARGS;

	if (items < 2) {
		parent = toFree;
	} else {
		parent = (void *)SvIV(ST(1));
		key = items > 2 ? SvPVbyte_nolen(ST(2)) : NULL;
	}
	Entity *r = yeCreateString(SvPVbyte_nolen(ST(0)),
				   parent, key);
	XSRETURN_IV(PTR2IV(r));
}

XS(XS_yeReCreateString)
{
	Entity *parent;
	const char *key = NULL;
	dXSARGS;

	if (items < 2) {
		parent = toFree;
	} else {
		parent = (void *)SvIV(ST(1));
		key = items > 2 ? SvPVbyte_nolen(ST(2)) : NULL;
	}
	Entity *r = yeReCreateString(SvPVbyte_nolen(ST(0)),
				   parent, key);
	XSRETURN_IV(PTR2IV(r));
}

XS(XS_yeReCreateInt)
{
	Entity *parent;
	const char *key = NULL;
	dXSARGS;

	if (items < 2) {
		parent = toFree;
	} else {
		parent = (void *)SvIV(ST(1));
		key = items > 2 ? SvPVbyte_nolen(ST(2)) : NULL;
	}
	Entity *r = yeReCreateInt(SvIV(ST(0)),
				   parent, key);
	XSRETURN_IV(PTR2IV(r));
}

XS(XS_yeReCreateArray)
{
	Entity *parent;
	const char *key = NULL;
	dXSARGS;

	if (items < 1) {
		parent = toFree;
	} else {
		parent = (void *)SvIV(ST(1));
		key = items > 1 ? SvPVbyte_nolen(ST(1)) : NULL;
	}
	Entity *r = yeReCreateArray(parent, key, NULL);
	XSRETURN_IV(PTR2IV(r));
}

XS(XS_yesCall)
{
	union ycall_arg args[17];
	int types[17];
	void *r;
	int i = 1;
	dXSARGS;

	for (; i < items; ++i) {
		args[i - 1].e = (void *)SvIV(ST(i));
		types[i - 1] = YS_ENTITY;
		if (!args[i - 1].e) {
			break;
		}
	}

	r = yesCallInt((void *)SvIV(ST(0)), i - 1, args, types);
	XSRETURN_IV(PTR2IV(r));
}

XS(XS_yevCreateGrp)
{
	Entity *r = NULL;
	Entity *parent;
	dXSARGS;

	parent = (void *)SvIV(ST(0));
	if (!parent)
		parent = toFree;
	switch (items) {
	case 0:
	case 1:
		croak("Usage: yevCreateGrp(parent, Keys...)");
		break;
	case 2:
		r = yevCreateGrp(parent, (int)SvIV(ST(1)));
		break;
	case 3:
		r = yevCreateGrp(parent, (int)SvIV(ST(1)),
				 (int)SvIV(ST(2)));
		break;
	case 4:
		r = yevCreateGrp(parent, (int)SvIV(ST(1)),
				 (int)SvIV(ST(2)), (int)SvIV(ST(3)));
		break;
	}

	XSRETURN_IV(PTR2IV(r));
}

#define BIND_AUTORET(call)						\
	int t = YSCRIPT_RET_TYPE(call, 1);				\
	switch (t) {							\
	case YSCRIPT_RET_VOID:						\
		call;							\
		XSRETURN(0);						\
		break;							\
	case YSCRIPT_RET_ENTITY:					\
	{								\
		IV miv = PTR2IV(YSCRIPT_VOID_CALL(call));		\
		XSRETURN_IV(miv);					\
		break;							\
	}								\
	case YSCRIPT_RET_STR:						\
		XSRETURN_PV((char *)(intptr_t)YSCRIPT_VOID_CALL(call));	\
		break;							\
	case YSCRIPT_RET_INT:						\
	case YSCRIPT_RET_LONG:						\
		XSRETURN_IV((intptr_t)YSCRIPT_VOID_CALL(call));		\
		break;							\
	case YSCRIPT_RET_UINT:						\
		XSRETURN_UV((uintptr_t)YSCRIPT_VOID_CALL(call));	\
		break;							\
	case YSCRIPT_RET_BOOL:						\
		if (YSCRIPT_VOID_CALL(call))				\
			XSRETURN_YES;					\
		else							\
			XSRETURN_NO;					\
		break;							\
	}								\

#define UNIMPLEMENTED(name)					\
	XS(XS_##name)						\
	{							\
		croak("Usage: "#name"() not implemented");	\
	}

#define BIND_V(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name());				\
	}

#define BIND_E(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0))));	\
	}

#define BIND_I(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name(SvIV(ST(0))));		\
	}

#define BIND_S(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name(SvPVbyte_nolen(ST(0))));		\
	}

#define BIND_SS(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name(SvPVbyte_nolen(ST(0)),		\
				  SvPVbyte_nolen(ST(1))));		\
	}

#define BIND_ES(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name((void *)SvIV(ST(0)),			\
				  SvPVbyte_nolen(ST(1))));		\
	}

#define BIND_ESS(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name((void *)SvIV(ST(0)),			\
				  SvPVbyte_nolen(ST(1)),		\
				  SvPVbyte_nolen(ST(2))));		\
	}

#define BIND_SE(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name(SvPVbyte_nolen(ST(0)),	\
				  (void *)SvIV(ST(1))));	\
	}

#define BIND_SES(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name(SvPVbyte_nolen(ST(0)),	\
				  (void *)SvIV(ST(1)),		\
				  SvPVbyte_nolen(ST(2))));	\
	}

#define BIND_II(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name(SvIV(ST(0)), SvIV(ST(1))));		\
	}

#define BIND_SI(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name(SvPVbyte_nolen(ST(0)), SvIV(ST(1))));	\
	}

#define BIND_EE(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  (void *)SvIV(ST(1))));	\
	}

#define BIND_EI(name, ...)				\
	XS(XS_##name)					\
	{						\
		dXSARGS;				\
		BIND_AUTORET(name((void *)SvIV(ST(0)),	\
				  SvIV(ST(1))));	\
	}

#define BIND_IIE(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name(SvIV(ST(0)), SvIV(ST(1)),		\
				  (void *)SvIV(ST(2))));		\
	}

#define BIND_III(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name(SvIV(ST(0)), SvIV(ST(1)),		\
				  SvIV(ST(2))));			\
	}

#define BIND_ISS(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name(SvIV(ST(0)), SvPVbyte_nolen(ST(1)),	\
				  SvPVbyte_nolen(ST(2))));		\
	}

#define BIND_IES(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name(SvIV(ST(0)), (void *)SvIV(ST(1)),	\
				  SvPVbyte_nolen(ST(2))));		\
	}

#define BIND_ESE(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name((void *)SvIV(ST(0)),			\
				  SvPVbyte_nolen(ST(1)),		\
				  (void *)SvIV(ST(2))));		\
	}

#define BIND_IIS(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name(SvIV(ST(0)), SvIV(ST(1)),		\
				  SvPVbyte_nolen(ST(2))));		\
	}

#define BIND_EES(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name((void *)SvIV(ST(0)), (void *)SvIV(ST(1)), \
				  SvPVbyte_nolen(ST(2))));		\
	}


#define BIND_EEI(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name((void *)SvIV(ST(0)),			\
				  (void *)SvIV(ST(1)), SvIV(ST(2))));	\
	}

#define BIND_EII(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  SvIV(ST(1)), SvIV(ST(2))));	\
	}

#define BIND_EEE(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name((void *)SvIV(ST(0)), (void *)SvIV(ST(1)), \
				  (void *)SvIV(ST(2))));		\
	}

#define BIND_SEES(name, ...)						\
	XS(XS_##name)							\
	{								\
		dXSARGS;						\
		BIND_AUTORET(name(SvPVbyte_nolen(ST(3)),		\
				  (void *)SvIV(ST(1)),			\
				  (void *)SvIV(ST(2)),			\
				  SvPVbyte_nolen(ST(3))));		\
	}

#define BIND_EIII(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  SvIV(ST(1)), SvIV(ST(2)),	\
				  SvIV(ST(3))));		\
	}

#define BIND_EIIE(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  SvIV(ST(1)), SvIV(ST(2)),	\
				  (void *)SvIV(ST(3))));	\
	}

#define BIND_EIIS(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  SvIV(ST(1)), SvIV(ST(2)),	\
				  SvPVbyte_nolen(ST(3))));	\
	}

#define BIND_EEEE(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  (void *)SvIV(ST(1)),		\
				  (void *)SvIV(ST(2)),		\
				  (void *)SvIV(ST(3))));	\
	}

#define BIND_EEIS(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  (void *)SvIV(ST(1)),		\
				  SvIV(ST(2)),			\
				  SvPVbyte_nolen(ST(3))));	\
	}

#define BIND_EIES(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  SvIV(ST(1)),			\
				  (void *)SvIV(ST(2)),		\
				  SvPVbyte_nolen(ST(3))));	\
	}

#define BIND_EEES(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  (void *)SvIV(ST(1)),		\
				  (void *)SvIV(ST(2)),		\
				  SvPVbyte_nolen(ST(3))));	\
	}

#define BIND_EEEEI(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  (void *)SvIV(ST(1)),		\
				  (void *)SvIV(ST(2)),		\
				  (void *)SvIV(ST(3)),		\
				  SvIV(ST(4))));		\
	}

#define BIND_EEESI(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  (void *)SvIV(ST(1)),		\
				  (void *)SvIV(ST(2)),		\
				  SvPVbyte_nolen(ST(3)),	\
				  SvIV(ST(4))));		\
	}

#define BIND_EIIEE(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  SvIV(ST(1)),			\
				  SvIV(ST(2)),			\
				  (void *)SvIV(ST(3)),		\
				  (void *)SvIV(ST(4))));	\
	}

#define BIND_EIIIS(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  SvIV(ST(1)),			\
				  SvIV(ST(2)),			\
				  SvIV(ST(3)),			\
				  SvPVbyte_nolen(ST(4))));	\
	}


#define BIND_EIIIIS(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  SvIV(ST(1)),			\
				  SvIV(ST(2)),			\
				  SvIV(ST(3)),			\
				  SvIV(ST(4)),			\
				  SvPVbyte_nolen(ST(5))));	\
	}

#define BIND_EIIIISI(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  SvIV(ST(1)),			\
				  SvIV(ST(2)),			\
				  SvIV(ST(3)),			\
				  SvIV(ST(4)),			\
				  SvPVbyte_nolen(ST(5)),	\
				  SvIV(ST(6))			\
				     ));			\
	}

#include "binding.c"

UNIMPLEMENTED(yeAddAt)
UNIMPLEMENTED(yeIncrAt)
UNIMPLEMENTED(yeSetIntAt)
UNIMPLEMENTED(ywPosCreate)

XS(XS_yeGet)
{
	Entity *ret;
	dXSARGS;
	if (SvTYPE(ST(1)) == SVt_IV) {
		ret = yeGet((void *)SvIV(ST(0)), SvIV(ST(1)));
	} else if (SvTYPE(ST(1)) == SVt_PV) {
		ret = yeGet((void *)SvIV(ST(0)), SvPVbyte_nolen(ST(1)));
	} else {
		croak("invalide inputetype");
		return;
	}
	XSRETURN_IV(PTR2IV(ret));
}

XS(XS_yeGetIntAt)
{
	int ret;
	dXSARGS;
	if (SvTYPE(ST(1)) == SVt_IV) {
		ret = yeGetIntAt((void *)SvIV(ST(0)), SvIV(ST(1)));
	} else if (SvTYPE(ST(1)) == SVt_PV) {
		ret = yeGetIntAt((void *)SvIV(ST(0)), SvPVbyte_nolen(ST(1)));
	} else {
		croak("invalide inputetype");
		return;
	}
	XSRETURN_IV(ret);
}

EXTERN_C void xs_init(pTHX)
{
	char *file = __FILE__;
	dXSUB_SYS;
#define BIND(name, ...)				\
	newXS("Yirl::"#name, XS_##name, file);

#define PUSH_I_GLOBAL_VAL(...)
#define PUSH_I_GLOBAL(...)
#define IN_CALL 1
#include "binding.c"
	BIND(yeCreateFunction);
	BIND(yeCreateArray);
	BIND(yeCreateString);
	BIND(yeReCreateString);
	BIND(yeReCreateInt);
	BIND(yeReCreateArray);
	BIND(yeCreateInt);
	BIND(yeCreateFloat);
	BIND(ywidNewWidget);
 	BIND(ywTextScreenNew);
 	BIND(yesCall);
	BIND(ygFileToEnt);
#undef IN_CALL

}

static int init(void *sm, void *args)
{
	struct YPerlScript *yperl = sm;

	yperl->my_perl = perl_alloc();
	perl_construct(yperl->my_perl);

	return 0;
}

static int destroy(void *sm)
{
	struct YPerlScript *yperl = sm;

	perl_destruct(yperl->my_perl);
	perl_free(yperl->my_perl);
	free(sm);
	return 0;
}

static void *call(void *sm, const char *name, int nb, union ycall_arg *args,
		  int *t_array)
{
	struct YPerlScript *yperl = sm;
	PerlInterpreter *my_perl = yperl->my_perl;
	int count;
	Entity *oldParent = toFree;
	void *res = NULL;

	toFree = yeCreateArray(NULL, NULL);
	dSP;
	ENTER;
	SAVETMPS;
	PUSHMARK(SP);
	if (nb) {
		for (int i = 0; i < nb; ++i) {
			if (t_array[i] == YS_STR)
				XPUSHs(sv_2mortal(newSVpv(args[i].str, 0)));
			else
				XPUSHs(sv_2mortal(newSViv(PTR2IV(args[i].vptr))));
		}
	}
	PUTBACK;
	count = call_pv(name, G_EVAL | G_SCALAR);
	if (SvTRUE(ERRSV)) {
		DPRINT_ERR("perl '%s' call fail: %s\n",
			   name,
			   SvPV_nolen(ERRSV));
		ygDgbAbort();
	}
	SPAGAIN;

	if (count == 1)
		res = (void *)POPi;
	PUTBACK;
	FREETMPS;
	LEAVE;
	yeDestroy(toFree);
	toFree = oldParent;
	return (void *)(intptr_t)res;
}

static int loadString(void *sm, const char *str)
{
	struct YPerlScript *yperl = sm;
	PerlInterpreter *my_perl = yperl->my_perl;
	/* don't know if this work, did that in the metro... no internet */
	eval_pv(str, TRUE);
	if (SvTRUE(ERRSV)) {
		// an error occurred
		// handle the error
	}
	return 0;
}

static int loadFile(void *sm, const char *file)
{
	struct YPerlScript *yperl = sm;
	perl_parse(yperl->my_perl, xs_init,  2,
		   (char *[]){"", (char *)file, NULL}, NULL);
}

int ysPerlInit(void)
{
	t = ysRegister(allocator);
	return t;
}

int ysPerlEnd(void)
{
	return ysUnregiste(t);
}

int ysPerlGetType(void)
{
	return t;
}


#endif /* PERL_ENABLE > 0 */
