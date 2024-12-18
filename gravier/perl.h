#ifndef PERL_H
#define PERL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef TRUE
#define FALSE 0
#endif

enum {
#define TOK(v) TOK_##v,
#include "perl-tok.h"
	TOK_UNKNOW
};

static char *tok_str[] = {
#define TOK(v) #v ,
	#include "perl-tok.h"
	"unknow"
};

enum {
	SVt_IV,
	SVt_UV,
	SVt_PV
};

enum {
	G_EVAL = 1,
	G_SCALAR = 1 << 1
};

struct stack_val {
	union {
		intptr_t i;
		void *v;
		double f;
		char *str;
	};
	int type;
};

static inline int is_skipable(char c)
{
	return c == ' ' || c == '\n' || c == '\t';
}

int dumbcmp(char *in, const char *keywork) {
	char *orig = in;

	for (; *in && *keywork; ++in, ++keywork) {
		if (*in != *keywork)
			return 0;
	}
	if (*keywork == 0 && (*in == ' ' || *in == '\n' || *in == '\t' || *in == 0)) {
		return in - orig;
	}
	return 0;
}

static struct stack_val ERRSV_s;

static struct stack_val *ERRSV;

#define SvTYPE(v) (v->type)

#define pTHX void *stuff

#define dXSARGS do {				\
		printf("dXSARGS stub\n");	\
} while (0)

#define XSRETURN_IV(int_val) do {		\
	printf("XSRETURN_IV stub\n");		\
	} while (0)


#define XSRETURN_UV(int_val) do {		\
	printf("XSRETURN_UV stub\n");		\
	} while (0)

#define XSRETURN_PV(int_val) do {		\
	printf("XSRETURN_PV stub\n");		\
	} while (0)


#define XSRETURN(val) do {				\
		printf("XDRETURN(%s) stub\n", #val);	\
	} while (0)

#define XSRETURN_YES do {			\
		printf("XSRETURN_YES stub\n");	\
	} while (0)

#define XSRETURN_NO do {			\
		printf("XSRETURN_NO\n");	\
	} while (0)


#define croak(str) do {				\
		printf("croak: " str);		\
	} while (0)

#define ST(pos)					\
	({printf("ST(%d) stub\n", pos); &(struct stack_val ){.i=-1, .type=SVt_IV};})

static inline intptr_t SvIV(struct stack_val *sv) {
	printf("SvIV(%ld) stub\n", sv->i);
	return sv->i;
}

#define XPUSHs(val)				\
	printf("XPUSHs %s\n", #val)

#define ENTER					\
	printf("ENTER\n")

#define SAVETMPS				\
	printf("SAVETMPS\n")

#define PUSHMARK(val)				\
	printf("PUSHMARK %s\n", #val)

#define POPi					\
	({ printf("POPi\n"); -1LL; })

#define SPAGAIN					\
	printf("SPAGAIN\n")

#define PUTBACK					\
	printf("PUTBACK\n")

#define FREETMPS				\
	printf("FREETMPS\n")

#define LEAVE					\
	printf("LEAVE\n")

#define dSP					\
	printf("dSP\n")

#define dXSUB_SYS				\
	printf("dXSUB_SYS\n")

#define XS(name)				\
	void name(const char *func_name, const char *file, int items)

#define SvTRUE(sv) (sv->i)

#define SvPV_nolen(sv) (sv->str)

#define SvPVbyte_nolen(sv) (sv->str)

#define SvNV(sv) (sv->f)

#define PTR2IV(ptr) ((intptr_t)ptr)

typedef intptr_t IV;

#define EXTERN_C

typedef struct {
} PerlInterpreter;


static inline void newXS(const char *func_name,
			 void (*name)(const char *, const char *, int),
			 const char *file)
{
	printf("newXS %s %s\n", func_name, file);
	return;
}

static inline PerlInterpreter *perl_alloc(void)
{
	printf("perl_alloc\n");
	return malloc(sizeof(PerlInterpreter));
}

static inline void perl_construct(PerlInterpreter *p)
{
	printf("perl_construct\n");
	ERRSV = &ERRSV_s;
}

static inline void perl_destruct(PerlInterpreter *p)
{
	printf("perl_destruct\n");
}

static inline void perl_free(PerlInterpreter *p)
{
	printf("perl_free\n");
	free(p);
}

static inline int call_pv(const char *str, int flag)
{
	printf("call_pv %s\n", str);
	return 0;
}

static inline void eval_pv(const char *str, int dont_know)
{
	printf("eval_pv: %s\n", str);
}

struct tok {
	int tok;
	union {
		char *as_str;
		intptr_t as_int;
	};
};

#define LOOK_FOR_DOUBLE(first, sec, first_tok, second_tok)		\
	else if (*reader == first) {					\
		if (reader[1] == sec) {					\
			++reader;					\
			RET_NEXT((struct tok){.tok=second_tok});	\
		}							\
		RET_NEXT((struct tok){.tok=first_tok});			\
	}

#define RET_NEXT(val) do {			\
		*reader_ptr = reader + 1;	\
		return val;			\
	} while (0)

static inline struct tok next(char **reader_ptr)
{
	static int next_tok = 0;
	char *reader = *reader_ptr;
	int ret;

	if (next_tok) {
		struct tok r = {.tok=next_tok};
		next_tok = 0;
		return r;
	}

again:
	for (; *reader && is_skipable(*reader); ++reader);

	*reader_ptr = reader;

	if (*reader == '#') {
		reader = strchr(reader, '\n');
		if (!reader)
			RET_NEXT((struct tok){.tok=TOK_ENDFILE});
		goto again;
	}

	if ((ret = dumbcmp(reader, "my"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_MY});
	} else if ((ret = dumbcmp(reader, "else"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_ELSE});
	} else if ((ret = dumbcmp(reader, "if"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_IF});
	} else if ((ret = dumbcmp(reader, "elsif"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_ELSIF});
	} else if (dumbcmp(reader, "sub")) {
		reader += 2;
		RET_NEXT((struct tok){.tok=TOK_SUB});
	} else if (*reader == 0) {
		RET_NEXT((struct tok){.tok=TOK_ENDFILE});
	} else if (*reader == ';') {
		RET_NEXT((struct tok){.tok=TOK_SEMICOL});
	}
	LOOK_FOR_DOUBLE('!', '=', TOK_NOT, TOK_NOT_EQUAL)
	LOOK_FOR_DOUBLE('*', '=', TOK_MULT, TOK_MULT_EQUAL)
	LOOK_FOR_DOUBLE('|', '|', TOK_PIPE, TOK_DOUBLE_PIPE)
	LOOK_FOR_DOUBLE('&', '&', TOK_AND, TOK_DOUBLE_AND)
	LOOK_FOR_DOUBLE('=', '=', TOK_EQUAL, TOK_DOUBLE_EQUAL)
	LOOK_FOR_DOUBLE('>', '=', TOK_SUP, TOK_SUP_EQUAL)
	LOOK_FOR_DOUBLE('<', '=', TOK_INF, TOK_INF_EQUAL)
	LOOK_FOR_DOUBLE('-', '=', TOK_MINUS, TOK_MINUS_EQUAL)
	LOOK_FOR_DOUBLE('+', '=', TOK_PLUS, TOK_PLUS_EQUAL)
	else if (*reader == '.') {
		RET_NEXT((struct tok){.tok=TOK_DOT});
	} else if (*reader == '$') {
		RET_NEXT((struct tok){.tok=TOK_DOLAR});
	} else if (*reader == '/') {
		RET_NEXT((struct tok){.tok=TOK_DIV});
	} else if (*reader == '@') {
		RET_NEXT((struct tok){.tok=TOK_AT});
	} else if (*reader == ',') {
		RET_NEXT((struct tok){.tok=TOK_COMMA});
	} else if (*reader == '(') {
		RET_NEXT((struct tok){.tok=TOK_OPEN_PARENTESIS});
	} else if (*reader == '{') {
		RET_NEXT((struct tok){.tok=TOK_OPEN_BRACE});
	} else if (*reader == '[') {
		RET_NEXT((struct tok){.tok=TOK_OPEN_BRACKET});
	} else if (*reader == ')') {
		RET_NEXT((struct tok){.tok=TOK_CLOSE_PARENTESIS});
	} else if (*reader == '}') {
		RET_NEXT((struct tok){.tok=TOK_CLOSE_BRACE});
	} else if (*reader == ']') {
		RET_NEXT((struct tok){.tok=TOK_CLOSE_BRACKET});
	} else if (*reader == '"' || *reader == '\'') {
		char end_lit = *reader;

		++reader;
		struct tok r = {.tok=TOK_LITERAL_STR, .as_str=reader};
		reader = strchr(reader, end_lit);
		if (!reader) {
			r.tok = TOK_ENDFILE;
			return r;
		}
		*reader = 0;
		RET_NEXT(r);
	} else if (isdigit(*reader)) {
		struct tok r = {.tok=TOK_LITERAL_NUM};

		long long int i = strtoll(reader, &reader, 0);
		if (!reader) {
			r.tok = TOK_ENDFILE;
			return r;
		}
		r.as_int = i;
		--reader;
		RET_NEXT(r);
	} else if (isalpha(*reader) || *reader == '_') {
		int is_namespace = 0;
		int add = 0;

		for (++reader; isalnum(*reader) || *reader == '_' ; ++reader);
		struct tok r = {.tok=TOK_NAME, .as_str=*reader_ptr};
		if (reader[0] == ':' && reader[1] == ':') {
			r.tok = TOK_NAMESPACE;
			add = 1;
		} else {
			if (*reader == '(') {
				next_tok = TOK_OPEN_PARENTESIS;
			} else if (*reader == ')') {
				next_tok = TOK_CLOSE_PARENTESIS;
			} else if (reader[0] == '-') {
				if (reader[1] == '=') {
					add = 1;
					next_tok = TOK_MINUS_EQUAL;
				} else {
					next_tok = TOK_MINUS;
				}
			} else if (reader[0] == '+') {
				if (reader[1] == '=') {
					add = 1;
					next_tok = TOK_PLUS_EQUAL;
				} else {
					next_tok = TOK_PLUS;
				}
			} else if (reader[0] == '=') {
				if (reader[1] == '=') {
					add = 1;
					next_tok = TOK_DOUBLE_EQUAL;
				} else {
					next_tok = TOK_EQUAL;
				}
			} else if (*reader == ',') {
				next_tok = TOK_COMMA;
			} else if (*reader == '[') {
				next_tok = TOK_OPEN_BRACKET;
			} else if (*reader == ']') {
				next_tok = TOK_CLOSE_BRACKET;
			} else if (*reader == ';') {
				next_tok = TOK_SEMICOL;
			} else if (*reader == '{') {
				next_tok = TOK_OPEN_BRACE;
			} else if (*reader == '}') {
				next_tok = TOK_CLOSE_BRACE;
			} else if (*reader == 0) {
				next_tok = TOK_ENDFILE;
				--reader;
				goto out_alpla;
			} else if (!is_skipable(*reader)) {
				fprintf(stderr, "unexpected character: '%c\n", *reader);
			}
		}
		*reader = 0;
		reader += add;
	out_alpla:
		RET_NEXT(r);
	}
	RET_NEXT((struct tok){.tok=TOK_UNKNOW});
}

static inline int perl_parse(PerlInterpreter * my_perl, void (*xs_init)(void *stuff),  int,
			      char *av[], void *)
{
	const char *file_name = av[1];
	struct stat st;
	int fd = 0;
	struct tok t;
	int len;
	char *reader;
	int ret;

	printf("perl_parse %s\n", file_name);
	if (stat(file_name, &st) < 0 || (fd = open(file_name, O_RDONLY) ) < 0) {
		fprintf(stderr, "cannot open/stat '%s'", file_name);
		return -1;
	}
	len = st.st_size;
	char *file_str = malloc(len + 1);

	if (!file_str || read(fd, file_str, len) < 0)
		goto exit;
	file_str[len] = 0;
	//printf("file:\n%s\n", file_str);
	reader = file_str;

	while ((t = next(&reader)).tok != TOK_ENDFILE) {
		if (t.tok == TOK_NAME) {
			printf("%s ", t.as_str);
		} else if (t.tok == TOK_NAMESPACE) {
			printf("%s::", t.as_str);
		} else if (t.tok == TOK_SEMICOL || t.tok == TOK_OPEN_BRACE || t.tok == TOK_CLOSE_BRACE) {
			putchar(t.tok == TOK_SEMICOL ? ';' : t.tok == TOK_OPEN_BRACE ? '{' : '}');
			putchar('\n');
		} else {
			printf("%s ", tok_str[t.tok]);
		}
	}

exit:
	free(file_str);
	return 0;
}



#endif
