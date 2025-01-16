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
#include <inttypes.h>
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
	SVt_NULL,
	SVt_IV,
	SVt_NV,
	SVt_PV,
	SVt_PVAV, // array
	SVt_UV
};

enum {
	G_EVAL = 1,
	G_SCALAR = 1 << 1
};

enum {
	VAL_NEED_FREE = 1
};

struct stack_val {
	union {
		intptr_t i;
		void *v;
		double f;
		char *str;
		struct stack_val *array;
	};
	int16_t type;
	int8_t flag;
	int32_t array_size;
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
	if (*keywork == 0 && (*in == ' ' || *in == '\n' || *in == '\t' || *in == 0 || *in == '(' || *in == ';')) {
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

enum {
	IDX_IS_NONE,
	IDX_IS_TOKEN,
	IDX_IS_REF,
	IDX_IS_NEXT
};

struct array_idx_info {
	union {
		struct tok tok;
		struct sym *ref;
	};
	int8_t type;
};

struct sym {
	struct tok t;
	union {
		struct stack_val v;
		struct {
			struct sym *caller;
			struct sym *f_ref;
			struct sym *end;
			struct sym *arg;
			struct sym *local_stack;
			int l_stack_len;
			int l_stack_size;
		};
		struct {
			struct sym *ref;
			struct array_idx_info idx;
		};
	};
};

struct file {
	khash_t(func_syms) *functions;
	char *file_content;
	struct sym *stack;
	int stack_len;
	int stack_size;
	struct sym *local_stack;
	int l_stack_len;
	int l_stack_size;
	int sym_len;
	int sym_size;
	struct sym *sym_string;
	struct sym *cur_func;
};

typedef struct {
	khash_t(file_list) *files;
	const char *first_file;
} PerlInterpreter;

#define LOOK_FOR_TRIPLE(first, sec, third, first_tok, second_tok, third_tok) \
	else if (*reader == first) {					\
		if (reader[1] == sec) {					\
			++reader;					\
			RET_NEXT((struct tok){.tok=second_tok});	\
		} else if (reader[1] == third) {			\
			++reader;					\
			RET_NEXT((struct tok){.tok=third_tok});	\
		}							\
		RET_NEXT((struct tok){.tok=first_tok});			\
	}

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
			free(vvar->sym_string);
			free(vvar->stack);
			free(vvar->local_stack);
			free(vvar->file_content);
			kh_destroy(func_syms, vvar->functions);
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

static struct tok back[128];
static int nb_back;

static char **reader_ptr;

static inline struct tok next(void)
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
	} else if ((ret = dumbcmp(reader, "while"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_WHILE});
	} else if ((ret = dumbcmp(reader, "else"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_ELSE});
	} else if ((ret = dumbcmp(reader, "if"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_IF});
	} else if ((ret = dumbcmp(reader, "return"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_RETURN});
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
	LOOK_FOR_TRIPLE('-', '=', '-', TOK_MINUS, TOK_MINUS_EQUAL, TOK_MINUS_MINUS)
	LOOK_FOR_TRIPLE('+', '=', '+', TOK_PLUS, TOK_PLUS_EQUAL, TOK_PLUS_PLUS)
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

#define CHECK_L_STACK_SPACE(this_file, X) do {				\
		if (this_file->l_stack_len + X > this_file->l_stack_size) { \
			this_file->l_stack_size *= 2;			\
			this_file->local_stack =			\
				realloc(this_file->local_stack,		\
					sizeof *this_file->local_stack * this_file->l_stack_size); \
		}							\
	} while (0)


#define SKIP_REQ(tok_, t) do {						\
		if ((t).tok != tok_)					\
			ERROR("unexpected token, require %s\n", tok_str[tok_]); \
		(t) = next();						\
	} while (0)


#define NEXT_N_CHECK_2(tok_, t_) do {					\
		t_ = next();						\
		if (t_.tok != tok_)					\
			ERROR("unexcected %s, expected %s\n", tok_str[t_.tok], tok_str[tok_]); \
	} while (0)

#define NEXT_N_CHECK(tok_) NEXT_N_CHECK_2(tok_, t)

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
	for (int i = this_file->l_stack_len - 1; i >= 0; --i) {
		if (!strcmp(this_file->local_stack[i].t.as_str, t->as_str))
			return &this_file->local_stack[i];
	}
	for (int i = 0; i < this_file->stack_len; ++i) {
		if (!strcmp(this_file->stack[i].t.as_str, t->as_str))
			return &this_file->stack[i];
	}
	return NULL;
}

static struct sym *find_set_stack_ref(struct file *this_file, struct tok *t)
{
	struct sym *ret = find_stack_ref(this_file, t);

	if (!ret) {
		CHECK_STACK_SPACE(this_file, 1);
		this_file->stack[this_file->stack_len].t = *t;
		this_file->stack[this_file->stack_len].idx.type = IDX_IS_NONE;
		this_file->stack[this_file->stack_len].v.type = SVt_NULL;
		this_file->stack[this_file->stack_len].v.array_size = 0;
		this_file->stack[this_file->stack_len].v.array = NULL;
		return &this_file->stack[this_file->stack_len++];
	}
	return ret;
}

static int parse_array_idx_nfo(struct file *this_file, struct tok t,
			       struct array_idx_info *idx)
{
	idx->type = IDX_IS_NONE;
	if (t.tok == TOK_OPEN_BRACKET) {
		struct tok bracket_tok = next();

		if (bracket_tok.tok == TOK_LITERAL_NUM || bracket_tok.tok == TOK_LITERAL_STR) {
			idx->type = IDX_IS_TOKEN;
			idx->tok = bracket_tok;
			bracket_tok = next();
		} else if (bracket_tok.tok == TOK_DOLAR) {

			idx->type = IDX_IS_REF;
			NEXT_N_CHECK_2(TOK_NAME, bracket_tok);
			idx->ref = find_set_stack_ref(this_file, &bracket_tok);
			bracket_tok = next();
			if (bracket_tok.tok == TOK_OPEN_BRACKET) {
				idx->type = IDX_IS_NEXT;
				return idx->type;
			}
		}
		if (bracket_tok.tok != TOK_CLOSE_BRACKET) {
			ERROR("expected ']'");
		}
	} else {
		idx->type = IDX_IS_NONE;
	}
	return idx->type;
exit:
	return -1;
}

#define STORE_OPERAND(in_t, in)						\
	if ((in_t).tok == TOK_LITERAL_STR || (in_t).tok == TOK_LITERAL_NUM) { \
		(in).t = in_t;						\
	} else if ((in_t).tok == TOK_DOLAR || (in_t).tok == TOK_DOLAR) { \
		struct array_idx_info array_idx;			\
		int tmp_tok = (in_t).tok;				\
		t = next();						\
		if (t.tok != TOK_NAME) {				\
			ERROR("variable name expected, not %s\n", tok_str[t.tok]);	\
		}							\
		(in).t.tok = tmp_tok;					\
		(in).ref = find_set_stack_ref(f, &t);			\
	}

static inline int unequal_to_equal(int tok)
{
	switch (tok) {
	case TOK_PLUS:
		return TOK_PLUS_EQUAL;
	case TOK_MINUS:
		return TOK_MINUS_EQUAL;
	case TOK_DIV:
		return TOK_DIV_EQUAL;
	case TOK_MULT:
		return TOK_MULT_EQUAL;
	}
	return 0;
}

static int parse_equal_(struct file *f, char **reader, struct sym *operand, int stack_tmp,
			struct tok t)
{
	CHECK_STACK_SPACE(f, stack_tmp);
	CHECK_SYM_SPACE(f, 4);
	int p = f->stack_len + stack_tmp;
	f->sym_string[f->sym_len++] = (struct sym){.ref=&f->stack[p], .t=TOK_EQUAL};
	f->sym_string[f->sym_len++] = *operand;
	int tok_op = t.tok;
	struct sym second;
	t = next();
	int in_parentesis = 0;
	if (t.tok == TOK_OPEN_PARENTESIS) {
		t = next();
		in_parentesis = 1;
	}
	STORE_OPERAND(t, second);
	// check for mul / div here and other here
	t = next();

recheck:
	if (t.tok == TOK_DIV || t.tok == TOK_MULT || in_parentesis) {
		parse_equal_(f, reader, &second, stack_tmp + 1, t);
		if (in_parentesis) {
			NEXT_N_CHECK(TOK_CLOSE_PARENTESIS);
			in_parentesis = 0;
			t = next();
			goto recheck;
		}
	} else {
		back[nb_back++] = t;
	}

	f->sym_string[f->sym_len++] = (struct sym){.ref=&f->stack[p], .t=unequal_to_equal(tok_op)};
	f->sym_string[f->sym_len++] = second;

	*operand = (struct sym){.ref=&f->stack[p], .t=TOK_DOLAR};
	return 0;
exit:
	return -1;
}

static int parse_equal(struct file *f, char **reader, struct sym equal_sym)
{
	struct tok t = next();

	struct sym operand;
	STORE_OPERAND(t, operand);
again:
	t = next();
	if (t.tok == TOK_PLUS || t.tok == TOK_MINUS || t.tok == TOK_DIV || t.tok == TOK_MULT) {
		parse_equal_(f, reader, &operand, 1, t);
		goto again;
	} else {
		back[nb_back++] = t;
	}
	f->sym_string[f->sym_len++] = equal_sym;
	f->sym_string[f->sym_len++] = operand;
	return 0;
exit:
	return -1;
}

static int parse_condition(struct tok *t_ptr, struct file *f, char **reader)
{
	CHECK_SYM_SPACE(f, 2);
	struct sym operation = {0};
	struct sym l_operand = {0};
	struct sym r_operand = {0};
	struct tok t;
	int have_not = 0;

	while (t_ptr->tok == TOK_NOT) {
		have_not = !have_not;
		*t_ptr = next();
	}
	if (have_not) {
		f->sym_string[f->sym_len++].t.tok = TOK_NOT;
	}
	STORE_OPERAND(*t_ptr, l_operand);
	t = next();
	if (t.tok == TOK_CLOSE_PARENTESIS || t.tok == TOK_SEMICOL) {
		f->sym_string[f->sym_len++] = l_operand;
		*t_ptr = t;
		back[nb_back++] = t;
		return 0;
	}
	if (!tok_is_condition(t.tok)) {
		ERROR("unexpected token\n");
	}

	operation.t = t;
	t = next();

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

	if (t_ptr->tok == TOK_PLUS_PLUS || t_ptr->tok == TOK_MINUS_MINUS) {
		f->sym_string[f->sym_len].t = *t_ptr;
		t = next();
		if (t.tok != TOK_DOLAR) {
			ERROR("unimplemented\n");
		}
		t = next();
		if (t.tok != TOK_NAME)
			ERROR("expected name in icrement/decrement\n");

		struct array_idx_info array_idx;
		struct sym *stack_sym = find_set_stack_ref(f, &t);
		if (!stack_sym) {
			ERROR("unknow variable\n");
		}

		f->sym_string[f->sym_len++].ref = stack_sym;
		return 0;
	}
	if (t_ptr->tok == TOK_NAME) {
		khiter_t iterator = kh_get(func_syms, f->functions, t_ptr->as_str);

		if (iterator == kh_end(f->functions))
			ERROR("unknow function %s\n", t_ptr->as_str);
		struct sym *function = kh_val(f->functions, iterator);
		int need_close = 0;
		f->sym_string[f->sym_len].f_ref = function;
		t = next();
		if (t.tok == TOK_OPEN_PARENTESIS) {
			need_close = 1;
		} else {
			back[nb_back++] = t;
		}
		// implement arguent push here
		if (need_close) {
			t = next();
			SKIP_REQ(TOK_CLOSE_PARENTESIS, t);
		}
		f->sym_string[f->sym_len++].t.tok = TOK_SUB;
		return 0;
	}
	if (t_ptr->tok != TOK_DOLAR && t_ptr->tok != TOK_AT) {
		ERROR("unimplemented operation: %s\n", tok_str[t_ptr->tok]);
	}
	t = next();
	if (t.tok != TOK_NAME)
		ERROR("expected name\n in operation");

	struct array_idx_info array_idx;
	struct sym *stack_sym = find_set_stack_ref(f, &t);
	if (!stack_sym) {
		ERROR("unknow variable\n");
	}
	t = next();
	if (parse_array_idx_nfo(f, t, &array_idx) > IDX_IS_NONE) {
		t = next();
	}

	if (t.tok != TOK_EQUAL && t.tok != TOK_PLUS_EQUAL && t.tok != TOK_MINUS_EQUAL)
		ERROR("unexpected operation %s\n", tok_str[t.tok]);
	struct sym equal_sym = {.ref = stack_sym, .t = t};
	if (array_idx.type >= IDX_IS_NONE)
		equal_sym.idx = array_idx;
	if (t_ptr->tok == TOK_AT) {
		NEXT_N_CHECK(TOK_OPEN_PARENTESIS);
		CHECK_SYM_SPACE(f, 1);
		f->sym_string[f->sym_len++] = (struct sym){.ref=stack_sym, .t=TOK_ARRAY_RESSET};
		t = next();
		while (t.tok != TOK_CLOSE_PARENTESIS) {
			f->sym_string[f->sym_len++] = (struct sym){.ref=stack_sym, .t=TOK_ARRAY_PUSH};
			STORE_OPERAND(t, f->sym_string[f->sym_len]);
			f->sym_len++;

			t = next();
			if (t.tok != TOK_COMMA && t.tok != TOK_CLOSE_PARENTESIS)
				ERROR("unexpected token in array init");
			if (t.tok == TOK_COMMA)
				t = next();
		}
	} else {
		parse_equal(f, reader, equal_sym);
	}
	return 0;

exit:
	return -1;
}

static int var_declaration(struct tok t, struct file *f, char **reader)
{
	if (t.tok == TOK_MY) {
		t = next();
		SKIP_REQ(TOK_DOLAR, t);
		if (t.tok != TOK_NAME) {
			ERROR("unexpected token, require %s\n", tok_str[TOK_NAME]); \
		}
		CHECK_L_STACK_SPACE(f, 1);
		f->local_stack[f->l_stack_len++].t = t;
		back[nb_back++] = t;
		t.tok = TOK_DOLAR;
	}

	if (t.tok != TOK_DOLAR) {
		goto exit;
	}

	operation(&t, f, reader);

	return 0;
exit:
	return -1;
}

#define MAX_ELSIF 258

static int parse_one_instruction(PerlInterpreter * my_perl, struct file *f, char **reader,
				 struct tok t)
{
	if (t.tok == TOK_MY || t.tok == TOK_DOLAR) {
		var_declaration(t, f, reader);
	} else if (t.tok == TOK_RETURN) {
		f->sym_string[f->sym_len].t = t;
		f->sym_string[f->sym_len++].caller = f->cur_func;
	} else if (t.tok == TOK_SUB) {
		int ret = 0;
		struct sym *goto_end = &f->sym_string[f->sym_len];
		f->sym_string[f->sym_len++].t.tok = TOK_GOTO;

		struct sym *function_begin = &f->sym_string[f->sym_len];
		function_begin->caller = f->cur_func;
		f->cur_func = function_begin;
		f->sym_string[f->sym_len++].t.tok = TOK_SUB;

		NEXT_N_CHECK(TOK_NAME);

		khiter_t iterator;
		iterator = kh_put(func_syms, f->functions, t.as_str, &ret);
		if (ret < 0)
			ERROR("me hash table fail me, so sad :(\n");
		kh_val(f->functions, iterator) = function_begin;

		//int begin_local_stack = f->l_stack_len;

		NEXT_N_CHECK(TOK_OPEN_BRACE);
		while ((t = next()).tok != TOK_CLOSE_BRACE) {
			if (t.tok == TOK_ENDFILE) {
				ERROR("unclose brace\n");
			}
			if (parse_one_instruction(my_perl, f, reader, t) < 0)
				goto exit;
		}

		f->sym_string[f->sym_len].caller = function_begin;
		f->sym_string[f->sym_len++].t.tok = TOK_RETURN;
		//f->l_stack_len = begin_local_stack;
		f->cur_func = f->cur_func->caller;
		goto_end->end = &f->sym_string[f->sym_len];
	} else if (t.tok == TOK_IF) {
		struct sym *elsif_goto_syms[MAX_ELSIF];
		int nb_elseif = 0;
		struct sym *if_sym;
		{
		an_elsif:
			if_sym = &f->sym_string[f->sym_len++];

			CHECK_SYM_SPACE(f, 8);
			if_sym->t = t;
			t = next();
			SKIP_REQ(TOK_OPEN_PARENTESIS, t);
			if (parse_condition(&t, f, reader) < 0)
				goto exit;
			t = next();
			SKIP_REQ(TOK_CLOSE_PARENTESIS, t);
			parse_one_instruction(my_perl, f, reader, t);
			t = next();
			if (t.tok == TOK_ELSE) {
				struct sym *goto_sym = &f->sym_string[f->sym_len];
				f->sym_string[f->sym_len++].t.tok = TOK_GOTO;
				if_sym->end = &f->sym_string[f->sym_len];
				t = next();
				parse_one_instruction(my_perl, f, reader, t);
				goto_sym->end = &f->sym_string[f->sym_len];
			} else if (t.tok == TOK_ELSIF) {
				elsif_goto_syms[nb_elseif++] = &f->sym_string[f->sym_len];
				f->sym_string[f->sym_len++].t.tok = TOK_GOTO;
				if_sym->end = &f->sym_string[f->sym_len];
				goto an_elsif;
			} else {
				if_sym->end = &f->sym_string[f->sym_len];
				back[nb_back++] = t;
			}
			if (nb_elseif) {
				for (int i = 0; i < nb_elseif; ++i) {
					elsif_goto_syms[i]->end = &f->sym_string[f->sym_len];
				}
			}
		}
	} else if (t.tok == TOK_WHILE) {
		CHECK_SYM_SPACE(f, 8);
		t = next();

		// Not need to ne push before we next
		SKIP_REQ(TOK_OPEN_PARENTESIS, t);

		struct sym *begin_while = &f->sym_string[f->sym_len++];
		begin_while->t.tok = TOK_IF;

		parse_condition(&t, f, reader);
		t = next();

		SKIP_REQ(TOK_CLOSE_PARENTESIS, t);

		parse_one_instruction(my_perl, f, reader, t);

		f->sym_string[f->sym_len].end = begin_while;
		f->sym_string[f->sym_len++].t.tok = TOK_GOTO;
		begin_while->end = &f->sym_string[f->sym_len];
	} else if (t.tok == TOK_FOR) {
		CHECK_SYM_SPACE(f, 8);
		gravier_debug("handling for\n");
		t = next();
		if (t.tok != TOK_OPEN_PARENTESIS) {
			ERROR("unexpected token");
		}
		t = next();
		if (!var_declaration(t, f, reader)) {
			t = next();
		}
		gravier_debug("for cur tok %s (sould be 2nd argument))\n", tok_str[t.tok]);
		struct sym *check_at_end = &f->sym_string[f->sym_len];
		f->sym_string[f->sym_len++].t.tok = TOK_GOTO;
		struct sym *begin_for = &f->sym_string[f->sym_len];
		struct tok for_toks_cnd[128];
		int nb_for_toks_cnd = 0;
		struct tok for_toks_op[128];
		int nb_for_toks_op = 0;
		while ((t = next()).tok != TOK_SEMICOL) {
			for_toks_cnd[nb_for_toks_cnd++] = t;
		}
		while ((t = next()).tok != TOK_CLOSE_PARENTESIS) {
			for_toks_op[nb_for_toks_op++] = t;
		}
		parse_one_instruction(my_perl, f, reader, next());

		for (int i = nb_for_toks_op - 1; i >= 0; --i) {
			back[nb_back++] = for_toks_op[i];
		}
		t = next();
		operation(&t, f, reader);

		check_at_end->end = &f->sym_string[f->sym_len];
		struct sym *out_loop = &f->sym_string[f->sym_len];

		f->sym_string[f->sym_len++].t.tok = TOK_IF;

		for (int i = nb_for_toks_cnd - 1; i >= 0; --i) {
			back[nb_back++] = for_toks_cnd[i];
		}
		t = next();
		parse_condition(&t, f, reader);

		f->sym_string[f->sym_len].t.tok = TOK_GOTO;
		f->sym_string[f->sym_len++].end = begin_for;
		out_loop->end = &f->sym_string[f->sym_len];
		//operation(&t, f, reader);
	} else if (t.tok == TOK_OPEN_BRACE) {
		// humm I have stack locality to handle here...
		int begin_local_stack = f->l_stack_len;

		while ((t = next()).tok != TOK_CLOSE_BRACE) {
			if (t.tok == TOK_ENDFILE) {
				ERROR("unclose brace\n");
			}
			if (parse_one_instruction(my_perl, f, reader, t) < 0)
				goto exit;
		}

		f->l_stack_len = begin_local_stack;
		// and stack to remove here

	} else if (t.tok == TOK_NAMESPACE) {
		gravier_debug("%s::", t.as_str);
	} else if (t.tok == TOK_SEMICOL) {
		gravier_debug("%c\n", t.tok == TOK_SEMICOL ? ';' : t.tok == TOK_OPEN_BRACE ? '{' : '}');

	} else if (t.tok == TOK_PRINT) {
		int need_close = 0;
		/* volatile because gcc seems to set garbage value un print_sym */
		struct sym *print_sym = &f->sym_string[f->sym_len];

		f->sym_string[f->sym_len++].t = t;
		t = next();
		if (t.tok == TOK_OPEN_PARENTESIS) {
			t = next();
			need_close = 1;
		}
	print_comma:
		CHECK_SYM_SPACE(f, 4);
		if (t.tok == TOK_LITERAL_NUM || t.tok == TOK_LITERAL_STR) {
			f->sym_string[f->sym_len++].t = t;
			t = next();
		} else if (t.tok == TOK_DOLAR) {
			t = next();
			// check t is name
			struct array_idx_info array_idx;
			struct sym *stack_sym = find_set_stack_ref(f, &t);
			f->sym_string[f->sym_len].ref = stack_sym;
			f->sym_string[f->sym_len].t.tok = TOK_DOLAR;
			t = next();
			if (parse_array_idx_nfo(f, t, &array_idx) > IDX_IS_NONE) {
				f->sym_string[f->sym_len].idx = array_idx;
				t = next();
			}
			f->sym_len++;
		} else {
			ERROR("unexpected %s token\n", tok_str[t.tok]);
		}

		if (t.tok == TOK_COMMA) {
			t = next();
			goto print_comma;
		}
		if (need_close && t.tok == TOK_CLOSE_PARENTESIS) {
			t = next();
		} else if (need_close) {
			ERROR("unclose parensesis in print\n");
		}
		print_sym->end = &f->sym_string[f->sym_len];
	} else {
		operation(&t, f, reader);
		//f->sym_string[f->sym_len].t = t;
		gravier_debug("%s ", tok_str[t.tok]);
		//f->sym_len += 1;
	}
	return 0;
exit:
	return -1;
}

static int perl_parse(PerlInterpreter * my_perl, void (*xs_init)(void *stuff), int ac,
			      char *av[], void *env)
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
	this_file->stack_len = 0;
	this_file->stack = malloc(sizeof *this_file->stack * this_file->stack_size);
	this_file->l_stack_len = 0;
	this_file->l_stack_size = 128;
	this_file->local_stack = malloc(sizeof *this_file->local_stack * this_file->l_stack_size);
	this_file->functions = kh_init(func_syms);
	this_file->cur_func = NULL;

	//printf("file:\n%s\n", file_str);
	reader = file_str;
	reader_ptr = &reader;
	int ret_parsing = TOK_ENDFILE;

	while ((t = next()).tok != ret_parsing) {
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

#define MATH_OP(tok_, left, right)				\
	if ((left).type == SVt_NV || tok_ == TOK_DIV_EQUAL) {	\
		if ((left).type == SVt_IV)			\
			(left).f = (double)(left).i;		\
		(left).type = SVt_NV;				\
		switch (tok_) {					\
		case TOK_PLUS_EQUAL:				\
			(left).f += (double)right; break;	\
		case TOK_MINUS_EQUAL:				\
			(left).f -= (double)right; break;	\
		case TOK_MULT_EQUAL:				\
			(left).f *= (double)right; break;	\
		case TOK_DIV_EQUAL:				\
			(left).f /= (double)right; break;	\
		}						\
	} else {						\
		(left).type = SVt_IV;				\
		switch (tok_) {					\
		case TOK_PLUS_EQUAL:				\
			(left).i += right; break;		\
		case TOK_MINUS_EQUAL:				\
			(left).i -= right; break;		\
		case TOK_MULT_EQUAL:				\
			(left).i *= right; break;		\
		}						\
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
					printf("%"PRIiPTR, sym_string->t.as_int);
				} else if (sym_string->t.tok == TOK_DOLAR) {
					struct sym *ref = sym_string->ref;
					struct stack_val *sv;

					if (!ref) {
						gravier_debug("unknow variable\n");
						continue;
					}
					sv = &ref->v;
					gravier_debug("A VARIABLE ! %p ", ref);
				pr_array_at:
					if (sv->type == SVt_PV)
						printf("%s", sv->str);
					else if (sv->type == SVt_IV)
						printf("%"PRIiPTR, sv->i);
					else if (sv->type == SVt_NV)
						printf("%.15f", sv->f);
					else if (sv->type == SVt_PVAV) {
						struct array_idx_info *idx = &sym_string->idx;

						if (idx->type == IDX_IS_TOKEN) {
							int i_idx = idx->tok.as_int;
							sv = &sv->array[i_idx];
							goto pr_array_at;
						}
					}
				}
			}
			continue;
		} else if (t.tok == TOK_ARRAY_RESSET) {
			struct sym *array = sym_string->ref;

			if (array->v.array_size) {
				free(array->v.array);
				array->v.array = NULL;
			}
			array->v.type = SVt_PVAV;
			array->v.array_size = 0;
		} else if (t.tok == TOK_ARRAY_PUSH) {
			struct sym *array = sym_string->ref;
			int p = array->v.array_size;

			array->v.type = SVt_PVAV;
			array->v.array_size = array->v.array_size + 1;
			array->v.array = realloc(array->v.array,
					       array->v.array_size * sizeof *array->v.array);
			struct stack_val *elem = &array->v.array[p];
			elem->type = SVt_NULL;
			++sym_string;
			if (sym_string->t.tok == TOK_LITERAL_STR) {
				elem->str = sym_string->t.as_str;
				elem->type = SVt_PV;
			} else if (sym_string->t.tok == TOK_LITERAL_NUM) {
				elem->i = sym_string->t.as_int;
				elem->type = SVt_IV;
			} else if (sym_string->t.tok == TOK_DOLAR) {
				*elem = sym_string->ref->v;
			} else {
				gravier_debug("UNIMPLEMENTED %s\n",
					      tok_str[sym_string->t.tok]);
			}
		} else if (t.tok == TOK_SUB) {
			struct sym *to_call = sym_string->f_ref;
			to_call->end = &sym_string[1];
			sym_string = to_call + 1;
			continue;
		} else if (t.tok == TOK_RETURN) {
			sym_string = sym_string->caller->end;
			continue;
		} else if (t.tok == TOK_IF || t.tok == TOK_ELSIF) {
			struct sym *if_end = sym_string->end;
			int have_not = 0;
			++sym_string;
			t = sym_string->t;
			if (t.tok == TOK_NOT) {
				have_not = 1;
				++sym_string;
				t = sym_string->t;
			}
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
			}
			gravier_debug("condition result: %d\n", cnd);
			if (have_not)
				cnd = !cnd;
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
			struct stack_val *sv = &target_ref->v;
			struct array_idx_info *op_idx = &sym_string->idx;

			gravier_debug("SET STUFFF on: %p\n", target_ref);
			++sym_string;

		eq_array_at:
			if ((sv->type == SVt_PVAV || sv->type == SVt_NULL) &&
			    op_idx && op_idx->type != IDX_IS_NONE) {
				struct array_idx_info *idx = op_idx;

				op_idx = NULL;
				if (idx->type == IDX_IS_TOKEN) {
					int i_idx = idx->tok.as_int;
					if (i_idx >= sv->array_size) {
						sv->type = SVt_PVAV;
						sv->array_size = i_idx + 1;
						sv->array = realloc(sv->array,
								    sv->array_size * sizeof *sv->array);
					}
					sv = &sv->array[i_idx];
					goto eq_array_at;
				}

			} else if (sym_string->t.tok == TOK_LITERAL_STR) {
				sv->str = sym_string->t.as_str;
				sv->type = SVt_PV;
			} else if (sym_string->t.tok == TOK_LITERAL_NUM) {
				sv->i = sym_string->t.as_int;
				sv->type = SVt_IV;
			} else if (sym_string->t.tok == TOK_DOLAR) {
				*sv = sym_string->ref->v;
			} else {
				gravier_debug("UNIMPLEMENTED %s\n",
					      tok_str[sym_string->t.tok]);
			}
		} else if (t.tok == TOK_PLUS_PLUS || t.tok == TOK_MINUS_MINUS) {
			struct sym *target_ref = sym_string->ref;
			if (t.tok == TOK_PLUS_PLUS)
				target_ref->v.i += 1;
			else
				target_ref->v.i -= 1;
			target_ref->v.type = SVt_IV;
		} else if (t.tok == TOK_PLUS_EQUAL || t.tok == TOK_MINUS_EQUAL ||
			TOK_DIV_EQUAL || TOK_DIV_EQUAL) {
			struct sym *target_ref = sym_string->ref;

			++sym_string;
			if (sym_string->t.tok == TOK_LITERAL_NUM) {
				MATH_OP(t.tok, target_ref->v,
					sym_string->t.as_int);
				target_ref->v.type = SVt_IV;
			} else if (sym_string->t.tok == TOK_DOLAR) {
				if (sym_string->ref->v.type == SVt_IV) {
					MATH_OP(t.tok, target_ref->v,
						sym_string->ref->v.i);
				}
			} else {
				gravier_debug("UNIMPLEMENTED %s\n",
					      tok_str[sym_string->t.tok]);
			}
		} else {
			printf("%s unimplemented\n", tok_str[t.tok]);
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
