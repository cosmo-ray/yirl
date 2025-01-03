#include "perl.h"

int main(int ac, char **av)
{
	PerlInterpreter *perl = perl_alloc();
	perl_construct(perl);
	perl_parse(perl, NULL, ac, av, NULL);
	perl_run(perl);
	perl_destruct(perl);
	perl_free(perl);
}
