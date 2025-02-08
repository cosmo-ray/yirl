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

EXTERN_C void xs_init(pTHX)
{
	char *file = __FILE__;
        dXSUB_SYS;
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
