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
#include <yirl/all.h>

static int t = -1;

struct YPerlScript {
	YScriptOps ops;
	PerlInterpreter *my_perl;
};

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
		// an error occurred
		// handle the error
	}
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
	perl_parse(yperl->my_perl, NULL,  2,
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
