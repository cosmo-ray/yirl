#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

XS(XS_add)
{
	dXSARGS;
	XSRETURN_IV(SvIV(ST(0)) + SvIV(ST(1)));
}

XS(XS_sub)
{
	dXSARGS;
	XSRETURN_IV(SvIV(ST(0)) - SvIV(ST(1)));
}

XS(XS_subcall)
{
	dXSARGS;

        ENTER;
        SAVETMPS;
        PUSHMARK(SP);
        if (items > 1) {
                for (int i = 1; i < items; ++i) {
                        if (SvTYPE(ST(i)) == SVt_PV)
                                XPUSHs(sv_2mortal(newSVpv(SvPVbyte_nolen(ST(i)), 0)));
                        else
                                XPUSHs(sv_2mortal(newSViv(PTR2IV(SvIV(ST(i))))));
                }
        }
        PUTBACK;
        int count = call_pv(SvPVbyte_nolen(ST(0)), G_EVAL | G_SCALAR);
        if (SvTRUE(ERRSV)) {
		printf("error in subcall");
        }
        SPAGAIN;

	intptr_t res = 0;
        if (count == 1) {
                res = POPi;
	}
        PUTBACK;
        FREETMPS;
	LEAVE;
	XSRETURN_IV(res);
}

EXTERN_C void xs_init(pTHX)
{
	char *file = __FILE__;
        dXSUB_SYS;
        newXS("ATest::subcall", XS_subcall, file);
        newXS("ATest::add", XS_add, file);
        newXS("ATest::sub", XS_sub, file);
}

int main(int ac, char **av)
{
	PerlInterpreter *perl = perl_alloc();
	perl_construct(perl);
	perl_parse(perl, xs_init, ac, av, NULL);
	perl_run(perl);
	perl_destruct(perl);
	perl_free(perl);
}
