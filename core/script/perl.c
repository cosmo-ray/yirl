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

#include <EXTERN.h> 
#include <perl.h>
#include <XSUB.h>
#include <yirl/all.h>

static int t = -1;

struct YPerlScript {
	YScriptOps ops;
	PerlInterpreter *my_perl;
};

XS(XS_yevCreateGrp)
{
	Entity *r = NULL;
	dXSARGS;
	switch (items) {
	case 0:
	case 1:
		croak("Usage: yevCreateGrp(parent, Keys...)");
		break;
	case 2:
		r = yevCreateGrp((void *)SvIV(ST(0)), (int)SvIV(ST(1)));
		break;
	case 3:
		r = yevCreateGrp((void *)SvIV(ST(0)), (int)SvIV(ST(1)),
				 (int)SvIV(ST(2)));
		break;
	case 4:
		printf("call creategrp: %p %d %d %d", (void *)SvIV(ST(0)),
		       (int)SvIV(ST(1)),
		       (int)SvIV(ST(2)), (int)SvIV(ST(3)));
		r = yevCreateGrp((void *)SvIV(ST(0)), (int)SvIV(ST(1)),
				 (int)SvIV(ST(2)), (int)SvIV(ST(3)));
		break;
	}


	yePrint(r);
	SV *ret = newSV(0);
	sv_setref_pv(ret, "Yirl::Entity", r);
	XSRETURN(1);	
}

#define BIND_AUTORET(call)			\
	int t = YSCRIPT_RET_TYPE(call);		\
	switch (t) {				\
	case YSCRIPT_RET_VOID:			\
		call;				\
		XSRETURN(0);			\
		break;				\
	case YSCRIPT_RET_ENTITY:		\
	case YSCRIPT_RET_OTHER:			\
		croak("return non implemented");	\
		break;				\
	}					\

#define BIND_E(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		printf("in XS_"#name"\n");			\
		BIND_AUTORET(name((void *)SvIV(ST(0))));	\
	}

#define BIND_EE(name, ...)					\
	XS(XS_##name)						\
	{							\
		dXSARGS;					\
		BIND_AUTORET(name((void *)SvIV(ST(0)),		\
				  (void *)SvIV(ST(1))));	\
	}

BIND_E(yePrint)

EXTERN_C void xs_init(pTHX)
{
	char *file = __FILE__;
	dXSUB_SYS;
#define BIND(name, ...)				\
	newXS("Yirl::"#name, XS_##name, file);	\
	printf("binding Yirl::"#name"\n");
#define PUSH_I_GLOBAL_VAL(...)
#define PUSH_I_GLOBAL(...)
#define IN_CALL 1
/* #include "binding.c" */
	BIND(yePrint);
	BIND(yevCreateGrp);
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
	int res = 0;
	// add boil plate

	dSP;
	ENTER;
	SAVETMPS;
	PUSHMARK(SP);
	if (nb) {
		for (int i = 0; i < nb; ++i) {
			if (t_array[i] == YS_STR)
				XPUSHs(sv_2mortal(newSVpv(args->str, 0)));
			else
				XPUSHs(sv_2mortal(newSViv((IV)args->vptr)));
		}
	}
	PUTBACK;
	res = call_pv(name, G_EVAL | G_SCALAR);
	if (SvTRUE(ERRSV)) {
		DPRINT_ERR("perl '%s' call fail: %s\n",
			   name,
			   SvPV_nolen(ERRSV));
		ygDgbAbort();
	}
	printf("res: %p\n", res);
	SPAGAIN;
	PUTBACK;
	FREETMPS;
	LEAVE;
	/* perl run */
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
