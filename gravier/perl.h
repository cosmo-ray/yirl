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

#include "klib/khash.h"
#include "gravier-str.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef TRUE
#define FALSE 0
#endif

#ifdef GRAVIER_ENABLE_DEBUG

#define gravier_debug(args...)			\
	do {					\
		printf(args);			\
	} while (0)

#else
#define gravier_debug(args...)
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
	int flag;
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
	if (*keywork == 0 && (*in == ' ' || *in == '\n' || *in == '\t' || *in == 0 || *in == '(')) {
		return in - orig;
	}
	return 0;
}

static struct stack_val ERRSV_s;

static struct stack_val *ERRSV;

#define SvTYPE(v) (v->type)

#define pTHX void *stuff

#define dXSARGS do {				\
		gravier_debug("dXSARGS stub\n");	\
} while (0)

#define XSRETURN_IV(int_val) do {		\
	gravier_debug("XSRETURN_IV stub\n");		\
	} while (0)


#define XSRETURN_UV(int_val) do {		\
	gravier_debug("XSRETURN_UV stub\n");		\
	} while (0)

#define XSRETURN_PV(int_val) do {		\
	gravier_debug("XSRETURN_PV stub\n");		\
	} while (0)


#define XSRETURN(val) do {				\
		gravier_debug("XDRETURN(%s) stub\n", #val);	\
	} while (0)

#define XSRETURN_YES do {			\
		gravier_debug("XSRETURN_YES stub\n");	\
	} while (0)

#define XSRETURN_NO do {			\
		gravier_debug("XSRETURN_NO\n");	\
	} while (0)


#define croak(str) do {				\
		gravier_debug("croak: " str);		\
	} while (0)

#define ST(pos)					\
	({gravier_debug("ST(%d) stub\n", pos); &(struct stack_val ){.i=-1, .type=SVt_IV};})

static inline intptr_t SvIV(struct stack_val *sv) {
	gravier_debug("SvIV(%ld) stub\n", sv->i);
	return sv->i;
}

#define XPUSHs(val)				\
	gravier_debug("XPUSHs %s\n", #val)

#define ENTER					\
	gravier_debug("ENTER\n")

#define SAVETMPS				\
	gravier_debug("SAVETMPS\n")

#define PUSHMARK(val)				\
	gravier_debug("PUSHMARK %s\n", #val)

#define POPi					\
	({ gravier_debug("POPi\n"); -1LL; })

#define SPAGAIN					\
	gravier_debug("SPAGAIN\n")

#define PUTBACK					\
	gravier_debug("PUTBACK\n")

#define FREETMPS				\
	gravier_debug("FREETMPS\n")

#define LEAVE					\
	gravier_debug("LEAVE\n")

#define dSP					\
	gravier_debug("dSP\n")

#define dXSUB_SYS				\
	gravier_debug("dXSUB_SYS\n")

#define XS(name)				\
	void name(const char *func_name, const char *file, int items)

#define SvTRUE(sv) (sv->i)

#define SvPV_nolen(sv) (sv->str)

#define SvPVbyte_nolen(sv) (sv->str)

#define SvNV(sv) (sv->f)

#define PTR2IV(ptr) ((intptr_t)ptr)

typedef intptr_t IV;

#define EXTERN_C


KHASH_MAP_INIT_STR(file_list, struct file *);
KHASH_MAP_INIT_STR(func_syms, struct sym *);

enum token {
	TOK_STR_NEED_FREE  = 1 << 0
};

struct tok {
	int tok;
	int flag;
	union {
		char *as_str;
		intptr_t as_int;
	};
};

struct sym {
	struct tok t;
	union {
		struct stack_val v;
		struct {
			struct tok const_val; // can be name, literal or anything else
			struct sym *end;
		};
		struct sym *ref;
	};
};

struct file {
	khash_t(func_syms) *functions;
	char *file_content;
	struct sym *stack;
	int stack_len;
	int stack_size;
	int sym_len;
	int sym_size;
	struct sym *sym_string;
};

typedef struct {
	khash_t(file_list) *files;
	const char *first_file;
} PerlInterpreter;

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



static inline void newXS(const char *func_name,
			 void (*name)(const char *, const char *, int),
			 const char *file)
{
	gravier_debug("newXS %s %s\n", func_name, file);
	return;
}

static inline PerlInterpreter *perl_alloc(void)
{
	gravier_debug("perl_alloc\n");
	return malloc(sizeof(PerlInterpreter));
}

static inline void perl_construct(PerlInterpreter *p)
{
	gravier_debug("perl_construct\n");
	p->files = kh_init(file_list);
	p->first_file = NULL;
	ERRSV = &ERRSV_s;
}

static inline void perl_destruct(PerlInterpreter *p)
{
	struct file *vvar;
	const char *kkey;

	gravier_debug("perl_destruct\n");
	kh_foreach(p->files, kkey, vvar, {
			free(vvar->file_content);
			free(vvar);
		});
	kh_destroy(file_list, p->files);
	p->files = NULL;
}

static inline void perl_free(PerlInterpreter *p)
{
	gravier_debug("perl_free\n");
	free(p);
}

static inline int call_pv(const char *str, int flag)
{
	gravier_debug("call_pv %s\n", str);
	return 0;
}

static inline void eval_pv(const char *str, int dont_know)
{
	gravier_debug("eval_pv: %s\n", str);
}

struct tok back[128];
int nb_back;

static inline struct tok next(char **reader_ptr)
{
	static int next_tok = 0;
	char *reader = *reader_ptr;
	int ret;

	if (nb_back) {
		return back[--nb_back];
	}

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
	} else if ((ret = dumbcmp(reader, "print"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_PRINT});
	} else if ((ret = dumbcmp(reader, "foreach"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_FOREACH});
	} else if ((ret = dumbcmp(reader, "for"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_FOR});
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
	} else if (dumbcmp(reader, "eq")) {
		reader += 1;
		RET_NEXT((struct tok){.tok=TOK_EQ});
	} else if (dumbcmp(reader, "do")) {
		reader += 1;
		RET_NEXT((struct tok){.tok=TOK_DO});
	} else if (dumbcmp(reader, "<<")) {
		reader += 1;
		RET_NEXT((struct tok){.tok=TOK_L_REDIRECTION});
	} else if (dumbcmp(reader, ">>")) {
		reader += 1;
		RET_NEXT((struct tok){.tok=TOK_R_REDIRECTION});
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
		graviver_str_small_replace(r.as_str, "\\n", "\n");
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

#define ERROR(args...)	do {			\
		fprintf(stderr, args);		\
		goto exit;			\
	} while (0)


#define CHECK_SYM_SPACE(this_file, X) do {				\
		if (this_file->sym_len + X > this_file->sym_size) {	\
			this_file->sym_size *= 2;			\
			this_file->sym_string =				\
				realloc(this_file->sym_string,		\
					sizeof *this_file->sym_string * this_file->sym_size); \
		}							\
	} while (0)

#define CHECK_STACK_SPACE(this_file, X) do {				\
		if (this_file->stack_len + X > this_file->stack_size) {	\
			this_file->stack_size *= 2;			\
			this_file->stack =				\
				realloc(this_file->stack,		\
					sizeof *this_file->stack * this_file->stack_size); \
		}							\
	} while (0)


static inline _Bool tok_is_condition(int t)
{
	return t == TOK_DOUBLE_EQUAL || t == TOK_NOT_EQUAL ||
		t == TOK_SUP_EQUAL || t == TOK_SUP ||
		t == TOK_INF_EQUAL || t == TOK_INF ||
		t == TOK_EQ;
}

static inline intptr_t int_fron_sym(struct sym *sym)
{
	// really unsure if literal string should return they pointer here
	if (sym->t.tok == TOK_LITERAL_NUM || sym->t.tok == TOK_LITERAL_STR) {
		return sym->t.as_int;
	} else if (sym->t.tok == TOK_DOLAR) {
		struct sym *ref = sym->ref;

		if (!ref)
			return 0;
		// again if type is not nbr here, maybe i should return 0
		return ref->v.i;
	}
	return 0;
}

static inline const char *str_fron_sym(struct sym *sym)
{
	if (sym->t.tok == TOK_LITERAL_STR) {
		return sym->t.as_str;
	} else if (sym->t.tok == TOK_DOLAR) {
		struct sym *ref = sym->ref;

		if (!ref || ref->v.type != SVt_PV)
			return "";
		return ref->v.str;
	}
	return "";
}

static struct sym *find_stack_ref(struct file *this_file, struct tok *t)
{
	for (int i = 0; i < this_file->stack_len; ++i) {
		if (!strcmp(this_file->stack[i].t.as_str, t->as_str))
			return &this_file->stack[i];
	}
	return NULL;
}

static int parse_equal(struct file *f, char **reader)
{
	struct tok t = next(reader);
	if (t.tok == TOK_LITERAL_NUM || t.tok == TOK_LITERAL_STR) {
		f->sym_string[f->sym_len++].t = t;
	} else {
		ERROR("unimplemented\n");
	}
	return 0;
exit:
	return -1;
}

#define STORE_OPERAND(in_t, in)						\
	if ((in_t).tok == TOK_LITERAL_STR || (in_t).tok == TOK_LITERAL_NUM) { \
		in.t = in_t;						\
	} else if ((in_t).tok == TOK_DOLAR) {				\
		t = next(reader);					\
		if (t.tok != TOK_NAME) {				\
			ERROR("variable name expected\n");		\
		}							\
		in.t.tok = TOK_DOLAR;					\
		in.ref = find_stack_ref(f, &t);				\
	}

static int parse_condition(struct tok *t_ptr, struct file *f, char **reader)
{
	CHECK_SYM_SPACE(f, 2);
	struct sym operation = {0};
	struct sym l_operand = {0};
	struct sym r_operand = {0};
	struct tok t;

	STORE_OPERAND(*t_ptr, l_operand);
	t = next(reader);
	if (t.tok == TOK_CLOSE_PARENTESIS) {
		f->sym_string[f->sym_len++] = l_operand;
		*t_ptr = t;
		return 0;
	}
	if (!tok_is_condition(t.tok)) {
		ERROR("unexpected token\n");
	}

	operation.t = t;
	t = next(reader);

	STORE_OPERAND(t, r_operand);

	f->sym_string[f->sym_len++] = operation;
	f->sym_string[f->sym_len++] = l_operand;
	f->sym_string[f->sym_len++] = r_operand;
	*t_ptr = t;
	return 0;
exit:
	return -1;
}

static int operation(struct tok *t_ptr, struct file *f, char **reader)
{
	struct tok t;

	if (t_ptr->tok != TOK_DOLAR) {
		ERROR("unimplemented\n");
	}
	t = next(reader);
	if (t.tok != TOK_NAME)
		ERROR("expected name\n");

	struct sym *stack_sym = find_stack_ref(f, &t);
	if (!stack_sym) {
		ERROR("unknow variable\n");
	}
	if (t.tok != TOK_EQUAL || t.tok != TOK_PLUS_EQUAL || t.tok != TOK_MINUS_EQUAL)
		ERROR("unexpected operation\n");
	f->sym_string[f->sym_len].ref = stack_sym;
	f->sym_string[f->sym_len++].t = t;
	parse_equal(f, reader);
	return 0;

exit:
	return -1;
}

static int var_declaration(struct tok t, struct file *f, char **reader)
{
	CHECK_STACK_SPACE(f, 1);
	if (t.tok == TOK_MY) {
		t = next(reader);
		if (t.tok != TOK_DOLAR) // @array need to be handle here
			ERROR("expected dolar");
	} else if (t.tok != TOK_DOLAR) {
		return -1;
	}
	t = next(reader);
	if (t.tok != TOK_NAME)
		ERROR("expected name");
	struct sym *stack_sym = find_stack_ref(f, &t);
	if (!stack_sym)
		stack_sym = &f->stack[f->stack_len++];
	stack_sym->t = t;
	t = next(reader);
	if (t.tok == TOK_SEMICOL)
		return 0;
	if (t.tok != TOK_EQUAL)
		ERROR("unexpected token: %s\n", tok_str[t.tok]);

	f->sym_string[f->sym_len].ref = stack_sym;
	f->sym_string[f->sym_len++].t = t;
	parse_equal(f, reader);

	return 0;
exit:
	return -1;
}

static int parse_one_instruction(PerlInterpreter * my_perl, struct file *f, char **reader,
				 struct tok t)
{
	if (t.tok == TOK_MY || t.tok == TOK_DOLAR) {
		var_declaration(t, f, reader);
	} else if (t.tok == TOK_IF) {
		struct sym *if_sym = &f->sym_string[f->sym_len++];

		CHECK_SYM_SPACE(f, 8);
		if_sym->t = t;
		t = next(reader);
		if (t.tok != TOK_OPEN_PARENTESIS)
			ERROR("unexpected token\n");
		t = next(reader);
		if (parse_condition(&t, f, reader) < 0)
			goto exit;
		t = next(reader);
		if (t.tok != TOK_CLOSE_PARENTESIS)
			ERROR("unexpected token\n");
		t = next(reader);
		parse_one_instruction(my_perl, f, reader, t);
		t = next(reader);
		if (t.tok == TOK_ELSE) {
			struct sym *goto_sym = &f->sym_string[f->sym_len];
			f->sym_string[f->sym_len++].t.tok = TOK_GOTO;
			if_sym->end = &f->sym_string[f->sym_len];
			t = next(reader);
			parse_one_instruction(my_perl, f, reader, t);
			goto_sym->end = &f->sym_string[f->sym_len];
		} else {
			if_sym->end = &f->sym_string[f->sym_len];
			back[nb_back++] = t;
		}
		gravier_debug("handling if\n");
	} else if (t.tok == TOK_FOR) {
		CHECK_SYM_SPACE(f, 8);
		gravier_debug("handling for\n");
		t = next(reader);
		if (t.tok != TOK_OPEN_PARENTESIS) {
			ERROR("unexpected token");
		}
		t = next(reader);
		if (!var_declaration(t, f, reader)) {
			t = next(reader);
		}
		gravier_debug("for cur tok %s (sould be 2nd argument))\n", tok_str[t.tok]);
		t = next(reader);
		operation(&t, f, reader);
	} else if (t.tok == TOK_OPEN_BRACE) {
		// humm I have stack locality to handle here...
		while ((t = next(reader)).tok != TOK_CLOSE_BRACE) {
			if (t.tok == TOK_ENDFILE) {
				ERROR("unclose brace\n");
			}
			if (parse_one_instruction(my_perl, f, reader, t) < 0)
				goto exit;
		}
		// and stack to remove here

	} else if (t.tok == TOK_NAMESPACE) {
		gravier_debug("%s::", t.as_str);
	} else if (t.tok == TOK_SEMICOL) {
		gravier_debug("%c\n", t.tok == TOK_SEMICOL ? ';' : t.tok == TOK_OPEN_BRACE ? '{' : '}');

	} else if (t.tok == TOK_PRINT) {
		int need_close = 0;
		struct sym *print_sym = &f->sym_string[f->sym_len];

		f->sym_string[f->sym_len++].t = t;
		t = next(reader);
		if (t.tok == TOK_OPEN_PARENTESIS) {
			t = next(reader);
			need_close = 1;
		}
	print_comma:
		CHECK_SYM_SPACE(f, 4);
		if (t.tok == TOK_LITERAL_NUM || t.tok == TOK_LITERAL_STR) {
			f->sym_string[f->sym_len++].t = t;
			t = next(reader);
		} else if (t.tok == TOK_DOLAR) {
			t = next(reader);
			// check t is name
			struct sym *stack_sym = find_stack_ref(f, &t);
			if (!stack_sym) {
				ERROR("unknow variable %s", t.as_str);
			}
			f->sym_string[f->sym_len].ref = stack_sym;
			f->sym_string[f->sym_len++].t.tok = TOK_DOLAR;
			t = next(reader);
		} else {
			ERROR("unexpected %s token\n", tok_str[t.tok]);
		}

		if (t.tok == TOK_COMMA) {
			CHECK_SYM_SPACE(f, 3);
			t = next(reader);
			goto print_comma;
		}
		if (need_close && t.tok == TOK_CLOSE_PARENTESIS) {
			t = next(reader);
		} else if (need_close) {
			ERROR("unclose parensesis in print\n", tok_str[t.tok]);
		}
		print_sym->end = &f->sym_string[f->sym_len];
	} else {
		//f->sym_string[f->sym_len].t = t;
		gravier_debug("%s ", tok_str[t.tok]);
		//f->sym_len += 1;
	}
	return 0;
exit:
	return -1;
}

static int perl_parse(PerlInterpreter * my_perl, void (*xs_init)(void *stuff),  int,
			      char *av[], void *)
{
	const char *file_name = av[1];
	struct stat st;
	int fd = 0;
	struct tok t;
	int len;
	char *reader;
	int ret;

	gravier_debug("perl_parse %s\n", file_name);
	if (stat(file_name, &st) < 0 || (fd = open(file_name, O_RDONLY) ) < 0) {
		fprintf(stderr, "cannot open/stat '%s'", file_name);
		return -1;
	}
	len = st.st_size;
	char *file_str = malloc(len + 1);

	khiter_t iterator;
	iterator = kh_put(file_list, my_perl->files, file_name, &ret);
	if (ret < 0)
		return -1;

	if (!file_str || read(fd, file_str, len) < 0)
		goto exit;
	file_str[len] = 0;

	struct file *this_file = malloc(sizeof *this_file);
	kh_val(my_perl->files, iterator) = this_file;

	this_file->file_content = file_str;

	if (!my_perl->first_file) {
		my_perl->first_file = file_name;
	}


	this_file->sym_size = 128;
	this_file->sym_len = 0;
	this_file->sym_string = malloc(sizeof *this_file->sym_string * this_file->sym_size);
	this_file->stack_size = 128;
	this_file->stack = malloc(sizeof *this_file->stack * this_file->sym_size);

	//printf("file:\n%s\n", file_str);
	reader = file_str;
	int ret_parsing = TOK_ENDFILE;

	while ((t = next(&reader)).tok != ret_parsing) {
		if (parse_one_instruction(my_perl, this_file, &reader, t) < 0)
			goto exit;
	}
	this_file->sym_string[this_file->sym_len].t = t;

	return 0;
exit:
	free(file_str);
	free(this_file);
	return -1;
}

static void perl_run_file(PerlInterpreter *perl, struct file *f)
{
	struct sym *sym_string = f->sym_string;

 	while (sym_string->t.tok != TOK_ENDFILE) {
		struct tok t = sym_string->t;

		if (t.tok == TOK_PRINT) {
			gravier_debug("in print\n");
			struct sym *end = sym_string->end;
			for (++sym_string; sym_string != end; ++sym_string) {
				if (sym_string->t.tok == TOK_LITERAL_STR) {
					printf("%s", sym_string->t.as_str);
				} else if (sym_string->t.tok == TOK_LITERAL_NUM) {
					printf("%d", sym_string->t.as_int);
				} else if (sym_string->t.tok == TOK_DOLAR) {
					struct sym *ref = sym_string->ref;

					if (!ref) {
						gravier_debug("unknow variable");
						continue;
					}
					gravier_debug("A VARIABLE ! %p ", ref);
					if (ref->v.type == SVt_PV)
						printf("%s", ref->v.str);
					else
						printf("%lli", ref->v.i);
				}
			}
			continue;
		} else if (t.tok == TOK_IF) {
			struct sym *if_end = sym_string->end;
			++sym_string;
			t = sym_string->t;
			if (!tok_is_condition(t.tok)) {
				if (t.tok == TOK_LITERAL_NUM && t.as_int)
					++sym_string;
				else
					sym_string = if_end;
				continue;
			}
			struct sym *lop = &sym_string[1];
			struct sym *rop = &sym_string[2];
			_Bool cnd = 0;
			switch (t.tok) {
			case TOK_DOUBLE_EQUAL:
				cnd = int_fron_sym(lop) == int_fron_sym(rop);
				break;
			case TOK_NOT_EQUAL:
				cnd = int_fron_sym(lop) != int_fron_sym(rop);
				break;
			case TOK_SUP_EQUAL:
				cnd = int_fron_sym(lop) >= int_fron_sym(rop);
				break;
			case TOK_SUP:
				cnd = int_fron_sym(lop) > int_fron_sym(rop);
				break;
			case TOK_INF_EQUAL:
				cnd = int_fron_sym(lop) <= int_fron_sym(rop);
				break;
			case TOK_INF:
				cnd = int_fron_sym(lop) < int_fron_sym(rop);
				break;
			case TOK_EQ:
				cnd = !strcmp(str_fron_sym(lop), str_fron_sym(rop));
				break;
			default:
			}
			gravier_debug("condition result: %d\n", cnd)
			if (cnd) {
				sym_string += 3;
			} else {
				sym_string = if_end;
			}
			continue;
		} else if (t.tok == TOK_GOTO) {
			sym_string = sym_string->end;
			continue;
		} else if (t.tok == TOK_EQUAL) {
			struct sym *target_ref = sym_string->ref;

			gravier_debug("SET STUFFF on: %p\n", target_ref);
			++sym_string;
			if (sym_string->t.tok == TOK_LITERAL_STR) {
				target_ref->v.str = sym_string->t.as_str;
				target_ref->v.type = SVt_PV;
			} else if (sym_string->t.tok == TOK_LITERAL_NUM) {
				target_ref->v.i = sym_string->t.as_int;
				target_ref->v.type = SVt_IV;
			} else {
				gravier_debug("UNIMPLEMENTED %s\n",
					      tok_str[sym_string->t.tok]);
			}
		}
		gravier_debug("%s ", tok_str[sym_string->t.tok]);
		++sym_string;
	}
	gravier_debug("perl run file\n");
}

static void perl_run(PerlInterpreter *perl)
{
	gravier_debug("perl run %s\n", perl->first_file);
	khiter_t iterator = kh_get(file_list, perl->files, perl->first_file);
	if (iterator == kh_end(perl->files))
		return;
	perl_run_file(perl, kh_val(perl->files, iterator));
}

#endif
