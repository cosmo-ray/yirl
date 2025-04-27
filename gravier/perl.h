

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
#include <time.h>
#include "klib/khash.h"
#include "gravier-str.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef TRUE
#define FALSE 0
#endif

#ifdef _WIN32

static char *strndup(const char *str, size_t chars)
{
    char *buffer;
    size_t n;

    buffer = (char *) malloc(chars +1);
    if (buffer)
    {
        for (n = 0; ((n < chars) && (str[n] != 0)) ; n++) buffer[n] = str[n];
        buffer[n] = 0;
    }

    return buffer;
}

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
	VAL_NEED_FREE = 1,
	VAL_NEED_STEAL = 2
};

struct stack_val {
	union {
		intptr_t i;
		void *v;
		double f;
		char *str;
		struct stack_val *array;
	};
	int8_t type;
	int8_t flag;
	int32_t array_size;
};

static int line_cnt = 0;

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

static struct stack_val call_args;

static struct stack_val ERRSV_s;

static struct stack_val *ERRSV;

#define SvTYPE(v) (v->type)

#define pTHX void *stuff

#define dXSARGS								\
	struct stack_val none = {.v = NULL, .flag = 0, .type=SVt_NULL};	\
	gravier_debug("dXSARGS stub\n");				\

#define XSRETURN_IV(int_val) do {				      \
		free_var(&cur_pi->return_val.v);		      \
		cur_pi->return_val.v = (struct stack_val){.i=int_val, \
			.flag=0, .type=SVt_IV};			      \
		return 1;					      \
	} while (0)


#define XSRETURN_UV(int_val) do {					\
		free_var(&cur_pi->return_val.v);			\
		cur_pi->return_val.v = (struct stack_val){.i=int_val,	\
			.flag=0, .type=SVt_IV};				\
		return 1;						\
	} while (0)

#define XSRETURN_PV(_val) do {						\
		free_var(&cur_pi->return_val.v);			\
		cur_pi->return_val.v = (struct stack_val){.str=strdup(_val), \
			.flag=VAL_NEED_STEAL, .type=SVt_PV};		\
		return 1;						\
	} while (0)


#define XSRETURN(val) do {					\
		return 0;					\
	} while (0)

#define XSRETURN_YES do {						\
		free_var(&cur_pi->return_val.v);			\
		cur_pi->return_val.v = (struct stack_val){.i=1,		\
			.flag=0, .type=SVt_PV};				\
		return 1;						\
	} while (0)

#define XSRETURN_NO do {			\
		free_var(&cur_pi->return_val.v);			\
		cur_pi->return_val.v = (struct stack_val){.i=0,		\
			.flag=0, .type=SVt_PV};				\
		return 1;						\
	} while (0)


#define croak(str) do {					\
		gravier_debug("croak: " str);		\
	} while (0)

#define ST(pos)								\
	({								\
		struct stack_val *ret;					\
		gravier_debug("ST(%d) stub\n", pos);			\
		if (pos >= func_sym->local_stack[0].v.array_size)	\
			ret = &none;					\
		else							\
			ret = &func_sym->local_stack[0].v.array[pos];	\
		ret;							\
	})
//&(struct stack_val ){.i=-1, .type=SVt_IV};	\

static inline intptr_t SvIV(struct stack_val *sv) {
	gravier_debug("SvIV(%ld) stub\n", sv->i);
	return sv->i;
}

static void free_var(struct stack_val *v);

static void array_free(struct stack_val *array)
{
	for (int i = 0; i < array->array_size; ++i) {
		free_var(&array->array[i]);
	}
	if (array->array_size) {
		free(array->array);
		array->array = NULL;
	}
	array->type = SVt_PVAV;
	array->array_size = 0;
}

static void free_var(struct stack_val *v)
{
	if (v->type == SVt_PV && v->flag & VAL_NEED_FREE) {
		free(v->str);
		v->str = NULL;
	} else if (v->type == SVt_PVAV) {
		array_free(v);
	}
	v->flag = 0;
}

static inline struct stack_val sv_2mortal(struct stack_val v) {
	v.flag = VAL_NEED_STEAL;
	return v;
}

static inline struct stack_val newSVpv(const char *str, int l)
{
	struct stack_val ret = {.type=SVt_PV, .flag = VAL_NEED_FREE};

	if (!l) {
		ret.str = strdup(str);
	} else {
		ret.str = strndup(str, l);
	}
	return ret;
}

static inline struct stack_val newSViv(intptr_t i)
{
	struct stack_val ret = {.flag = 0, .type=SVt_IV, .i=i};

	return ret;
}

static inline void XPUSHs(struct stack_val val)
{
	int p = call_args.array_size;

	call_args.type = SVt_PVAV;
	call_args.array_size = call_args.array_size + 1;
	call_args.array = realloc(call_args.array,
				 call_args.array_size * sizeof *call_args.array);
	call_args.array[p] = val;
}

#define ENTER					\
	gravier_debug("ENTER\n")

#define SAVETMPS				\
	gravier_debug("SAVETMPS\n")

#define PUSHMARK(val)				\
	gravier_debug("PUSHMARK %s\n", #val)

#define POPi					\
	({ gravier_debug("POPi\n"); cur_pi->return_val.v.i; })

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
	int name(struct sym *func_sym, int items)

#define SvTRUE(sv) (sv->i)

#define SvPV_nolen(sv) (sv->str)

#define SvPVbyte_nolen(sv) (sv->str)

#define SvNV(sv) (sv->f)

#define PTR2IV(ptr) ((intptr_t)ptr)

typedef intptr_t IV;

#define EXTERN_C

KHASH_MAP_INIT_STR(file_list, struct file *);
KHASH_MAP_INIT_STR(func_syms, struct sym *);
KHASH_MAP_INIT_STR(forward_func_h, struct forward_func *);

static int parse_equal(struct file *f, char **reader, struct sym equal_sym);

enum token {
	TOK_STR_NEED_FREE  = 1 << 0
};

struct tok {
	int tok;
	int flag;
	union {
		char *as_str;
		intptr_t as_int;
		double as_double;
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
			union {
				struct sym *f_ref;
				khiter_t f_iter;
				_Bool have_return;
				int (*nat_func)(struct sym *, int);
			};
			union {
				struct sym *end;
				struct file *package;
			};
			struct sym *local_stack;
			int l_stack_len;
			int l_stack_size;
		};
		struct {
			struct sym *ref;
			struct array_idx_info idx;
			_Bool oposite;
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
	struct sym return_val;
	struct file *cur_pkg;
	khash_t(forward_func_h) *forward_dec;
	char **tmp_files;
	int nb_tmp_files;
} PerlInterpreter;

static PerlInterpreter *cur_pi;

static int run_this(struct sym *sym_string, int return_at_return);

#define GR_ERROR(args...)	do {			\
		fprintf(stderr, args);		\
		goto exit;			\
	} while (0)


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


#define ALLOC_L_STACK_SPACE(this_file, X) do {				\
		this_file->l_stack_len = 0;				\
		this_file->l_stack_size = X;				\
		this_file->local_stack =				\
			malloc(sizeof *this_file->local_stack * this_file->l_stack_size); \
	} while (0)


static inline void newXS_(const char *func_name,
			 int (*nat_func)(struct sym *func_sym, int items),
			 struct file *this_mod)
{
	int ret = 0;
	khiter_t iterator = kh_put(func_syms, this_mod->functions, func_name, &ret);

	if (ret < 0)
		GR_ERROR("me hash table fail me, so sad :(\n");
	struct sym *function = malloc(sizeof *function);
	function->nat_func = nat_func;
	function->t.tok = TOK_NATIVE_FUNC;
	ALLOC_L_STACK_SPACE(function, 1);
	kh_val(this_mod->functions, iterator) = function;

exit:
	return;
}

static inline void newXS(const char *oname,
			 int (*nat_func)(struct sym *func_sym, int items),
			 const char *file)
{
	char *namespace = strstr(oname, "::");
	int ret = 0;
	if (!namespace)
		return;
	const char *func_name = namespace + 2;
	namespace = strndup(oname, namespace - oname);

	struct file *this_mod;
	khiter_t iterator = kh_get(file_list, cur_pi->files, namespace);
	khiter_t end = kh_end(cur_pi->files);
	if (iterator == end) {
		iterator = kh_put(file_list, cur_pi->files, namespace, &ret);
		if (ret < 0)
			goto exit;
		this_mod = malloc(sizeof *this_mod);
		kh_val(cur_pi->files, iterator) = this_mod;
			this_mod->sym_size = 0;
			this_mod->sym_len = 0;
			this_mod->sym_string = NULL;
			this_mod->stack_size = 0;
			this_mod->stack_len = 0;
			this_mod->stack = NULL;
			this_mod->l_stack_len = 0;
			this_mod->l_stack_size = 0;
			this_mod->local_stack = NULL;
			this_mod->functions = kh_init(func_syms);
			this_mod->cur_func = NULL;
	} else {
		this_mod = kh_val(cur_pi->files, iterator);
	}

	newXS_(func_name, nat_func, this_mod);
exit:
	return;
}

static inline PerlInterpreter *perl_alloc(void)
{
	gravier_debug("perl_alloc\n");
	return malloc(sizeof(PerlInterpreter));
}

static void stack_val_init(struct stack_val *s)
{
	s->type = SVt_NULL;
	s->i = 0;
	s->flag = 0;
	s->array_size = 0;
}

static inline void perl_construct(PerlInterpreter *p)
{
	gravier_debug("perl_construct\n");
	p->files = kh_init(file_list);
	p->first_file = NULL;
	p->tmp_files = NULL;
	p->nb_tmp_files = 0;
	stack_val_init(&p->return_val.v);
	srand(time(NULL));
	ERRSV = &ERRSV_s;
}

static inline void perl_destruct(PerlInterpreter *p)
{
	struct file *vvar;
	const char *kkey;
	const char *fdkey;
	struct forward_func *fdvar;

	gravier_debug("perl_destruct\n");

	kh_foreach(p->files, kkey, vvar, {
			const char *fkey;
			struct sym *fvar;

			kh_foreach(vvar->functions, fkey, fvar, {
					free(fvar->local_stack);
					if (fvar->t.tok == TOK_NATIVE_FUNC ||
					    fvar->t.tok == TOK_INDIRECT_FUNC) {
						free(fvar);
					}
				});

			for (int i = 0; i < vvar->l_stack_len; ++i) {
				free_var(&vvar->local_stack[i].v);
			}

			for (int i = 0; i < vvar->stack_len; ++i) {
				free_var(&vvar->stack[i].v);
			}

			free(vvar->sym_string);
			free(vvar->stack);
			free(vvar->local_stack);
			free(vvar->file_content);
			kh_destroy(func_syms, vvar->functions);
			free(vvar);
		});

	free_var(&p->return_val.v);
	kh_destroy(file_list, p->files);
	p->files = NULL;
	kh_destroy(forward_func_h, p->forward_dec);
	p->forward_dec = NULL;
	for (int i = 0; i < p->nb_tmp_files; ++i) {
		free(p->tmp_files[i]);
	}
	free(p->tmp_files);
}

static inline void perl_free(PerlInterpreter *p)
{
	gravier_debug("perl_free\n");
	free(p);
}

#define NEW_LOCAL_VAL()							\
	({								\
		struct sym *cur_func = cur_pi->cur_pkg->cur_func;	\
		struct sym *ret_ = cur_func ?				\
			&cur_func->local_stack[cur_func->l_stack_len++] : \
			&cur_pi->cur_pkg->local_stack[cur_pi->cur_pkg->l_stack_len++]; \
		ret_;							\
	})

#define NEW_LOCAL_VAL_INIT(name)					\
	({								\
		struct sym *ret_ = NEW_LOCAL_VAL();			\
		sym_val_init(ret_, (struct tok) {.tok=TOK_DOLAR, .as_str=name}); \
		ret_;							\
	})

static inline void push_free_var(struct sym *to_free, struct sym *syms, int *nb_syms)
{
	syms[*nb_syms] = (struct sym){.t.tok=TOK_FREE_VAR, .ref=to_free};
	*nb_syms += 1;
}

#define DEC_LOCAL_STACK(to_, rm_syms_dest, sym_iterator) do {		\
		struct sym *cur_func = cur_pi->cur_pkg->cur_func;	\
		if (cur_func) {						\
			for (int i = to_;				\
			     i <  cur_func->l_stack_len; ++i)		\
				push_free_var(&cur_func->local_stack[i], \
					      rm_syms_dest, sym_iterator); \
			cur_func->l_stack_len = (to_);			\
		} else {						\
			for (int i = to_;				\
			     i <  cur_pi->cur_pkg->l_stack_len; ++i)	\
				push_free_var(&cur_pi->cur_pkg->local_stack[i], \
					      rm_syms_dest, sym_iterator); \
			cur_pi->cur_pkg->l_stack_len = (to_);		\
		}							\
} while (0)


static struct sym *find_stack_ref_in_sym(struct sym *cur_func, const char *t_as_str)
{
	struct sym *cur_l_stack = cur_func->local_stack;

	for (int i = cur_func->l_stack_len - 1; i >= 0; --i) {
		if (!strcmp(cur_func->local_stack[i].t.as_str, t_as_str))
			return &cur_func->local_stack[i];
	}
	return NULL;
}

static inline int call_pv(const char *str, int flag)
{
	if (!str)
		return -1;

	char *namespace = strstr(str, "::");
	struct file * cur_pkg = cur_pi->cur_pkg;
	const char *name = str;

	if (namespace) {
		name = namespace + 2;
		namespace = strndup(str, namespace - str);
		khiter_t iterator = kh_get(file_list, cur_pi->files, namespace);
		free(namespace);
		khiter_t end = kh_end(cur_pi->files);

		if (iterator == end)
			return -1;
		cur_pkg = kh_val(cur_pi->files, iterator);
	}
	khiter_t iterator = kh_get(func_syms, cur_pkg->functions, name);
	khiter_t end = kh_end(cur_pi->files);

	if (iterator == end)
		return -1;
	struct sym *func  = kh_val(cur_pkg->functions, iterator);
	if (call_args.array_size) {
		struct sym *underscore;
		if (func->l_stack_len > 0 &&
		    (underscore = find_stack_ref_in_sym(func, "_"))) {
			array_free(&underscore->v);
		} else {
			fprintf(stderr, "could not find place for args in function stack\n");
			return -1;
		}
		underscore->v = call_args;
		call_args.array_size = 0;
		call_args.array = NULL;
	}
	int ret = run_this(func + 1, 1);
	return ret;
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
	for (; *reader && is_skipable(*reader); ++reader) {
		if (*reader == '\n')
			++line_cnt;
	}

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
	} else if ((ret = dumbcmp(reader, "or"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_STR_OR});
	} else if ((ret = dumbcmp(reader, "and"))) {
		reader += ret - 1;
		RET_NEXT((struct tok){.tok=TOK_STR_AND});
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
		struct tok r = {.tok=TOK_SUB, .as_str="sub"};
		RET_NEXT(r);
	} else if (dumbcmp(reader, "eq")) {
		reader += 1;
		RET_NEXT((struct tok){.tok=TOK_EQ});
	} else if (dumbcmp(reader, "do")) {
		reader += 1;
		RET_NEXT((struct tok){.tok=TOK_DO});
	} else if (dumbcmp(reader, "<<")) {
		RET_NEXT((struct tok){.tok=TOK_L_REDIRECTION});
	} else if (*reader == '<' && reader[1] == '<') {
		char end[128];
		int end_i = 0;
		reader += 2;

		for (; isalpha(*reader); ++reader)
			end[end_i++] = *reader;
		end[end_i] = 0;
		if (*reader != ';' && reader[1] != '\n') {
			fprintf(stderr, "unclose HERESTRING ex: $s = <<EOC;\\n\n");
			return (struct tok){.tok=TOK_ENDFILE};
		}
		reader += 2; // skip ';\n'
		line_cnt += 1;
		struct tok r = {.tok=TOK_LITERAL_STR, .as_str=reader};
		reader = strstr(reader, end);
		if (!reader) {
			r.tok = TOK_ENDFILE;
			return r;
		}
		*reader = 0;
		for (int j  = 0; reader[j]; ++j) {
			if (reader[j] == '\n')
				++line_cnt;
		}
		reader += end_i;
		next_tok = TOK_SEMICOL;
		graviver_str_small_replace(r.as_str, "\\\\", "\\");
		RET_NEXT(r);
	} else if (dumbcmp(reader, ">>")) {
		reader += 1;
		RET_NEXT((struct tok){.tok=TOK_R_REDIRECTION});
	} else if (*reader == 0) {
		RET_NEXT((struct tok){.tok=TOK_ENDFILE});
	} else if (*reader == ';') {
		RET_NEXT((struct tok){.tok=TOK_SEMICOL});
	} else if (*reader == '=' && reader[1] == '~') {
		reader += 2;
		struct tok r = {.tok=TOK_REGEX};

		while (*reader == ' ' || *reader == '\t')
			++reader;

		r.as_str = reader;
		while (*reader && *reader != ';' && *reader != '\n')
			++reader;

		if (*reader == ';')
			next_tok = TOK_SEMICOL;
		else if (!(*reader))
			next_tok = TOK_ENDFILE;
		*reader = 0;
		RET_NEXT(r);
	}
	LOOK_FOR_DOUBLE('!', '=', TOK_NOT, TOK_NOT_EQUAL)
	LOOK_FOR_DOUBLE('*', '=', TOK_MULT, TOK_MULT_EQUAL)
	LOOK_FOR_DOUBLE('|', '|', TOK_PIPE, TOK_DOUBLE_PIPE)
	LOOK_FOR_DOUBLE('&', '&', TOK_AND, TOK_DOUBLE_AND)
	LOOK_FOR_DOUBLE('=', '=', TOK_EQUAL, TOK_DOUBLE_EQUAL)
	LOOK_FOR_DOUBLE('>', '=', TOK_SUP, TOK_SUP_EQUAL)
	LOOK_FOR_DOUBLE('<', '=', TOK_INF, TOK_INF_EQUAL)
	LOOK_FOR_DOUBLE('.', '=', TOK_DOT, TOK_DOT_EQUAL)
	LOOK_FOR_TRIPLE('-', '=', '-', TOK_MINUS, TOK_MINUS_EQUAL, TOK_MINUS_MINUS)
	LOOK_FOR_TRIPLE('+', '=', '+', TOK_PLUS, TOK_PLUS_EQUAL, TOK_PLUS_PLUS)
	else if (*reader == '$') {
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
		graviver_str_small_replace(r.as_str, "\\033", "\033");
		graviver_str_small_replace(r.as_str, "\\t", "\t");
		RET_NEXT(r);
	} else if (isdigit(*reader)) {
		struct tok r = {.tok=TOK_LITERAL_NUM};
		char **end_toll = &reader;
		char **end_tod = &reader;

		long long int i = strtoll(reader, end_toll, 0);
		double d = strtod(reader, end_tod);
		if (!end_tod && !end_toll) {
			r.tok = TOK_ENDFILE;
			return r;
		}
		if (*end_tod > *end_toll) {
			r.tok=TOK_LITERAL_FLOAT;
			reader = *end_toll;
			r.as_double = d;
			--reader;
		} else {
			reader = *end_tod;
			r.as_int = i;
			--reader;
		}
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
			} else if (reader[0] == '.') {
				if (reader[1] == '=') {
					add = 1;
					next_tok = TOK_DOT_EQUAL;
				} else {
					next_tok = TOK_DOT;
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
			} else if (*reader == '\n') {
				++line_cnt;
			}
		}
		*reader = 0;
		reader += add;
	out_alpla:
		RET_NEXT(r);
	}
	RET_NEXT((struct tok){.tok=TOK_UNKNOW});
}

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

#define PUSH_L_STACK_CLEAR(f, cnt, end)					\
	for (int i = cnt->l_stack_len - 1; i >= end; --i) {		\
		if (cnt->local_stack[i].v.type == SVt_PVAV) {		\
			f->sym_string[f->sym_len++] = (struct sym){	\
				.ref=&cnt->local_stack[i],		\
				.t=TOK_ARRAY_RESSET			\
			};						\
		}							\
	}



#define SKIP_REQ(tok_, t) do {						\
		if ((t).tok != tok_)					\
			GR_ERROR("%s:%d: unexpected token, require %s\n",	\
			      __FILE__, __LINE__, tok_str[tok_]);	\
		(t) = next();						\
	} while (0)


#define NEXT_N_CHECK_2(tok_, t_) do {					\
		t_ = next();						\
		if (t_.tok != tok_)					\
			GR_ERROR("%s:%d: unexcected %s, expected %s\n", __FILE__, __LINE__, \
			      tok_str[t_.tok], tok_str[tok_]);	     \
	} while (0)

#define NEXT_N_CHECK(tok_) NEXT_N_CHECK_2(tok_, t)

/* in C they're call Relational Operators */
static inline _Bool tok_is_condition(int t)
{
	return t == TOK_DOUBLE_EQUAL || t == TOK_NOT_EQUAL ||
		t == TOK_SUP_EQUAL || t == TOK_SUP ||
		t == TOK_INF_EQUAL || t == TOK_INF ||
		t == TOK_EQ;
}

static inline _Bool tok_is_logical_operator(int t)
{
	 return t == TOK_DOUBLE_AND || t == TOK_DOUBLE_PIPE;

}

static inline _Bool tok_is_str_logical_operator(int t)
{
	 return t == TOK_STR_AND || t == TOK_STR_OR;
}

static inline _Bool tok_is_any_logical_operator(int t)
{
	return tok_is_logical_operator(t) || tok_is_str_logical_operator(t);

}

static inline intptr_t int_fron_sym(struct sym *sym)
{
	// really unsure if literal string should return they pointer here
	if (sym->t.tok == TOK_LITERAL_NUM || sym->t.tok == TOK_LITERAL_STR) {
		return sym->t.as_int;
	} else if (sym->t.tok == TOK_LITERAL_FLOAT) {
		return sym->t.as_double;
	} else if (sym->t.tok == TOK_DOLAR) {
		struct sym *ref = sym->ref;

		if (!ref)
			return 0;
		// again if type is not nbr here, maybe i should return 0
		if (ref->v.type == SVt_NV)
			return ref->v.f;
		return ref->v.i;
	} else if (sym->t.tok == TOK_ARRAY_SIZE) {
		struct sym *ref = sym->ref;

		if (!ref)
			return 0;
		// again if type is not nbr here, maybe i should return 0
		return ref->v.array_size;
	}
	return 0;
}

static inline _Bool is_sym_double(struct sym *sym)
{
	return sym->t.tok == TOK_LITERAL_FLOAT ||
		(sym->t.tok == TOK_DOLAR && sym->ref->v.type == SVt_NV);
}

static inline double double_fron_sym(struct sym *sym)
{
	if (sym->t.tok == TOK_LITERAL_NUM || sym->t.tok == TOK_LITERAL_STR) {
		return sym->t.as_int;
	} else if (sym->t.tok == TOK_LITERAL_FLOAT) {
		return sym->t.as_double;
	} else if (sym->t.tok == TOK_DOLAR) {
		struct sym *ref = sym->ref;

		if (!ref)
			return 0;
		// again if type is not nbr here, maybe i should return 0
		if (ref->v.type == SVt_NV)
			return ref->v.f;
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
	struct sym *cur_func = this_file->cur_func;

	while (cur_func) {
		struct sym *ret = find_stack_ref_in_sym(cur_func, t->as_str);
		if (ret)
			return ret;
		cur_func = cur_func->caller;
	}

	for (int i = this_file->l_stack_len - 1; i >= 0; --i) {
		const char *s = this_file->local_stack[i].t.as_str;

		if (s && !strcmp(s, t->as_str))
			return &this_file->local_stack[i];
	}

	for (int i = 0; i < this_file->stack_len; ++i) {
		if (!strcmp(this_file->stack[i].t.as_str, t->as_str))
			return &this_file->stack[i];
	}
	return NULL;
}

static void sym_val_init(struct sym *s, struct tok t)
{
	stack_val_init(&s->v);
	s->idx.type = IDX_IS_NONE;
	s->t = t;
}

static struct sym *find_set_stack_ref(struct file *this_file, struct tok *t)
{
	struct sym *ret = find_stack_ref(this_file, t);

	if (!ret) {
		CHECK_STACK_SPACE(this_file, 1);
		sym_val_init(&this_file->stack[this_file->stack_len], *t);
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
			GR_ERROR("expected ']'");
		}
	} else {
		idx->type = IDX_IS_NONE;
	}
	return idx->type;
exit:
	return -1;
}

#define STORE_OPERAND_DOLAR(in_t, in)					\
	{								\
		int tmp_tok = (in_t).tok;				\
		t = next();						\
		if (t.tok != TOK_NAME) {				\
			GR_ERROR("variable name expected, not %s\n", tok_str[t.tok]); \
		}							\
		(in).t.tok = tmp_tok;					\
		(in).ref = find_set_stack_ref(f, &t);			\
	}								\


#define STORE_OPERAND(in_t, in)						\
	if ((in_t).tok == TOK_MINUS) {					\
		t = next();						\
		if ((in_t).tok == TOK_LITERAL_NUM)			\
			(in).t = (struct tok){.tok=(in_t).tok, .as_int=-(in_t).as_int}; \
		else {							\
			(in).oposite = 1;				\
			STORE_OPERAND_DOLAR(in_t, in);			\
		}							\
	} else if ((in_t).tok == TOK_LITERAL_STR || (in_t).tok == TOK_LITERAL_NUM) { \
		(in).t = (in_t);					\
	} else if ((in_t).tok == TOK_DOLAR || (in_t).tok == TOK_AT) {	\
		(in).oposite = 0;					\
		STORE_OPERAND_DOLAR(in_t, in);				\
	}

static inline int unequal_to_equal(int tok)
{
	switch (tok) {
	case TOK_DOT:
		return TOK_DOT_EQUAL;
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

static int parse_func_call(struct tok t, struct file *f, struct sym *syms, int *nb_syms)
{
	struct file *namespace = f;
	if (t.tok == TOK_NAMESPACE) {
		const char *namespace_name = t.as_str;
		khiter_t iterator = kh_get(file_list, cur_pi->files, namespace_name);
		khiter_t end = kh_end(cur_pi->files);
		if (iterator == end) {
			GR_ERROR("can not find '%s' namespace\n", namespace_name);
		}
		namespace = kh_val(cur_pi->files, iterator);
		t = next();
	}

	char *func_name = t.as_str;
	khiter_t iterator = kh_get(func_syms, namespace->functions, func_name);
	struct sym *func_syms;
	struct sym *function;
	int is_indirect_func = 0;

	if (iterator == kh_end(namespace->functions)) {
		int ret;
		iterator = kh_put(func_syms, namespace->functions, func_name, &ret);
		if (ret < 0)
			GR_ERROR("me hash table fail me, so sad :(\n");
		gravier_debug("forward declaration of %s\n", t.as_str);
		function = malloc(sizeof *function);
		ALLOC_L_STACK_SPACE(function, 2048);
		function->t.tok = TOK_INDIRECT_FUNC;
		kh_val(namespace->functions, iterator) = function;
		is_indirect_func = 1;
	} else {
		function = kh_val(namespace->functions, iterator);
	}
	int end_call_tok = TOK_SEMICOL;
	gravier_debug("%s - %p\n", func_name, function);
	if (function->l_stack_len < 1) {
		function->l_stack_len = 1;
		function->local_stack[0].t = (struct tok){.tok=TOK_NAME, .as_str="_"};
		function->local_stack[0].v.type = SVt_PVAV;
		function->local_stack[0].v.array_size = 0;
		function->local_stack[0].v.array = NULL;

	}
	struct sym *underscore = &function->local_stack[0];
	t = next();
	if (t.tok == TOK_OPEN_PARENTESIS) {
		end_call_tok = TOK_CLOSE_PARENTESIS;
		t = next();
		if (t.tok != TOK_CLOSE_PARENTESIS)
			back[nb_back++] = t;
	} else {
		back[nb_back++] = t;
	}

	int i = 0;
	struct sym assignement[64];
	int nb_assignement = 0;
	int nb_annoying_assign = 0;
	if (t.tok != end_call_tok) {
		do {
			struct sym equal_sym = {
				.ref = underscore,
				.t = {.tok=TOK_EQUAL},
				.idx={
					.type=IDX_IS_TOKEN,
					.tok={.tok=TOK_LITERAL_NUM, .as_int=i++}}};
			int base = f->sym_len;
			parse_equal(f, reader_ptr, equal_sym);
			for (int i = base; i < f->sym_len - 2; i++, *nb_syms += 1) {
				syms[*nb_syms] = f->sym_string[i];
			}
			assignement[nb_assignement++] = f->sym_string[f->sym_len - 2];
			if (f->sym_string[f->sym_len - 1].t.tok == TOK_DOLAR &&
			    !strcmp("?eqtmp?", f->sym_string[f->sym_len - 1].ref->t.as_str)) {
				struct sym *s_ptr = NEW_LOCAL_VAL_INIT("?inarg?");
				nb_annoying_assign++;
				syms[*nb_syms] = (struct sym) {.t.tok=TOK_EQUAL, .ref=s_ptr};
				syms[*nb_syms + 1] = f->sym_string[f->sym_len - 1];
				assignement[nb_assignement++] = (struct sym){.ref=s_ptr, .t.tok=TOK_DOLAR,  .t.as_str="?assignement?", .oposite=0};;
				*nb_syms += 2;
			} else {
				assignement[nb_assignement++] = f->sym_string[f->sym_len - 1];
			}
			f->sym_len = base;
			t = next();
		} while (t.tok == TOK_COMMA);
	}
	// implement arguent push here
	if (t.tok != end_call_tok) {
		GR_ERROR("%d: unclose function, got %s, expected %s\n", line_cnt, tok_str[t.tok],
			tok_str[end_call_tok]);
	}

	for (int i = 0; i < nb_assignement; ++i) {
		syms[*nb_syms] = assignement[i];
		*nb_syms += 1;
	}
	if (is_indirect_func) {
		syms[*nb_syms].f_iter = iterator;
		syms[*nb_syms].package = namespace;
		/* not sure function.tok.t is equal to tok_sub or tok_name */
		syms[*nb_syms].t.tok = TOK_INDIRECT_FUNC;
	} else {
		syms[*nb_syms].f_ref = function;
		/* not sure function.tok.t is equal to tok_sub or tok_name */
		syms[*nb_syms].t.tok = TOK_SUB;
	}
	syms[*nb_syms].t.as_str = func_name;
	*nb_syms += 1;
	return 0;
exit:
	return -1;
}

static int parse_equal_(struct file *f, char **reader, struct sym *operand,
			struct tok t)
{
	int stack_tmp = 1;
	struct sym *s_ptr = NEW_LOCAL_VAL_INIT("?eqtmp_?");

	f->sym_string[f->sym_len++] = (struct sym){.ref=s_ptr, .t=TOK_EQUAL};
	f->sym_string[f->sym_len++] = *operand;
	int tok_op = t.tok;
	struct array_idx_info array_idx;
	struct sym syms[64];
	int nb_syms = 0;
	struct sym second;
	t = next();
	int in_parentesis = 0;

	if (t.tok == TOK_OPEN_PARENTESIS) {
		t = next();
		in_parentesis = 1;
	}

	if (t.tok == TOK_NAMESPACE || t.tok == TOK_NAME) {
		parse_func_call(t, f, syms, &nb_syms);
		for (int i = 0; i < nb_syms; ++i, f->sym_len++) {
			f->sym_string[f->sym_len] = syms[i];
		}
		struct sym *s_ptr_2 = NEW_LOCAL_VAL_INIT("?eqtmp_+?");
		++stack_tmp;
		second = (struct sym){.ref=s_ptr_2, .t.tok=TOK_DOLAR, .t.as_str="?high_eqtmp_?", .oposite=0};
		f->sym_string[f->sym_len++] = (struct sym){.ref=s_ptr_2, .t=TOK_EQUAL};
		f->sym_string[f->sym_len++] = (struct sym){.ref=&cur_pi->return_val,
			.t=TOK_DOLAR, .oposite=0};
	} else {
		STORE_OPERAND(t, second);
	}
	t = next();
	if (parse_array_idx_nfo(f, t, &array_idx) > IDX_IS_NONE) {
		second.idx = array_idx;
		t = next();
	} else {
		second.idx.type = IDX_IS_NONE;
	}


recheck:
	if (t.tok == TOK_DIV || t.tok == TOK_MULT || in_parentesis) {
		parse_equal_(f, reader, &second, t);
		if (in_parentesis) {
			NEXT_N_CHECK(TOK_CLOSE_PARENTESIS);
			in_parentesis = 0;
			t = next();
			goto recheck;
		}
	} else {
		back[nb_back++] = t;
	}

	f->sym_string[f->sym_len++] = (struct sym){.ref=s_ptr, .t=unequal_to_equal(tok_op)};
	f->sym_string[f->sym_len++] = second;

	*operand = (struct sym){.ref=s_ptr, .t=TOK_DOLAR, .oposite=0};
	return 0;
exit:
	return -1;
}

static int parse_equal(struct file *f, char **reader, struct sym equal_sym)
{
	struct tok t = next();

	struct array_idx_info array_idx;
	struct sym operand;
	struct sym regex = {.t.tok=0};
	struct sym syms[64];
	int stack_tmp = 1;
	int nb_syms = 0;
	int ret_val = -1;
	struct sym *s_ptr = NEW_LOCAL_VAL_INIT("?eqtmp?");

	if (t.tok == TOK_NAMESPACE || t.tok == TOK_NAME) {
		parse_func_call(t, f, syms, &nb_syms);
		for (int i = 0; i < nb_syms; ++i, f->sym_len++) {
			f->sym_string[f->sym_len] = syms[i];
		}
		operand = (struct sym){.ref=s_ptr, .t=TOK_DOLAR, .oposite=0};
		f->sym_string[f->sym_len++] = (struct sym){.ref=s_ptr,
			.t=TOK_EQUAL};
		f->sym_string[f->sym_len++] = (struct sym){.ref=&cur_pi->return_val,
			.t=TOK_DOLAR, .oposite=0};
		s_ptr = NEW_LOCAL_VAL_INIT("?eqtmp+?");
		++stack_tmp;
	} else {
		STORE_OPERAND(t, operand);
	}
	t = next();
	if (parse_array_idx_nfo(f, t, &array_idx) > IDX_IS_NONE) {
		operand.idx = array_idx;
		t = next();
	} else {
		operand.idx.type = IDX_IS_NONE;
	}
again:
	if (t.tok == TOK_PLUS || t.tok == TOK_MINUS || t.tok == TOK_DIV || t.tok == TOK_MULT ||
		t.tok == TOK_DOT) {
		parse_equal_(f, reader, &operand,  t);
		t = next();
		goto again;
	} else if (t.tok == TOK_REGEX) {
		regex.t= t;
	} else {
		back[nb_back++] = t;
	}
	f->sym_string[f->sym_len++] = equal_sym;
	if (regex.t.tok == TOK_REGEX)
		f->sym_string[f->sym_len++] = regex;
	f->sym_string[f->sym_len++] = operand;

	ret_val = 0;
exit:
	return ret_val;
}

#define CALL(t_, sym_)							\
	do {								\
		struct sym syms[64];					\
		int nb_syms = 0;					\
									\
		parse_func_call((t_), f, syms, &nb_syms);		\
		for (int i = 0; i < nb_syms; ++i, f->sym_len++) {	\
			f->sym_string[f->sym_len] = syms[i];		\
		}							\
		struct sym *lstack_ref = NEW_LOCAL_VAL_INIT("?callarg?"); \
		*dec_lstack += 1;					\
		f->sym_string[f->sym_len++] = (struct sym){.ref=lstack_ref, \
			.t=TOK_EQUAL};					\
		f->sym_string[f->sym_len++] = (struct sym){.ref=&cur_pi->return_val, \
			.t=TOK_DOLAR, .oposite=0};			\
		sym_ = (struct sym) {.oposite=0, .ref=lstack_ref, .t=TOK_DOLAR}; \
	} while (0)


static void parse_condition_(struct tok t, struct file *f, struct sym *syms, int *syms_l_ptr, int *dec_lstack)
{
	char **reader = reader_ptr;
	struct sym operation = {0};
	struct sym l_operand = {0};
	struct sym r_operand = {0};

	int syms_l = *syms_l_ptr;
	int have_not = 0;

	while (t.tok == TOK_NOT) {
		have_not = !have_not;
		t = next();
	}
	if (t.tok == TOK_NAMESPACE || t.tok == TOK_NAME) {
		CALL(t, l_operand);
	} else {
		STORE_OPERAND(t, l_operand);
	}

	t = next();
	if (t.tok == TOK_CLOSE_PARENTESIS || t.tok == TOK_SEMICOL ||
	    tok_is_any_logical_operator(t.tok)) {
		if (have_not) {
			syms[syms_l++].t.tok = TOK_NOT;
		}

		syms[syms_l++] = l_operand;
		if (tok_is_any_logical_operator(t.tok)) {
			syms[syms_l++].t.tok = t.tok;
			t = next();
			*syms_l_ptr = syms_l;

			return parse_condition_(t, f, syms, syms_l_ptr, dec_lstack);
		} else {
			back[nb_back++] = t;
			goto exit;
		}
	}
	if (!tok_is_condition(t.tok)) {
		GR_ERROR("%d: unexpected token %s\n", line_cnt, tok_str[t.tok]);
	}

	operation.t = t;
	t = next();

	if (t.tok == TOK_NAMESPACE || t.tok == TOK_NAME) {
		CALL(t, r_operand);
	} else {
		STORE_OPERAND(t, r_operand);
	}

	if (have_not) {
		syms[syms_l++].t.tok = TOK_NOT;
	}
	syms[syms_l++] = operation;
	syms[syms_l++] = l_operand;
	syms[syms_l++] = r_operand;

	t = next();
	if (tok_is_any_logical_operator(t.tok)) {
		syms[syms_l++].t.tok = t.tok;
		t = next();
		*syms_l_ptr = syms_l;
		return parse_condition_(t, f, syms, syms_l_ptr, dec_lstack);
	}
	back[nb_back++] = t;
exit:
	*syms_l_ptr = syms_l;
}

static struct sym *parse_condition(struct tok *t_ptr, struct file *f, char **reader, int tok)
{
	struct sym syms[125];
	int syms_l = 0;
	int dec_lstack = 0;

	parse_condition_(*t_ptr, f, syms, &syms_l, &dec_lstack);
	struct sym *ret = &f->sym_string[f->sym_len];
	f->sym_string[f->sym_len++].t.tok = tok;
	for (int i = 0; i < syms_l; ++i)
		f->sym_string[f->sym_len++] = syms[i];
	return ret;
exit:
	return NULL;
}

static int operation(struct tok *t_ptr, struct file *f, char **reader)
{
	struct tok t;
	int ret = -1;

	if (t_ptr->tok == TOK_PLUS_PLUS || t_ptr->tok == TOK_MINUS_MINUS) {
		f->sym_string[f->sym_len].t = *t_ptr;
		t = next();
		if (t.tok != TOK_DOLAR) {
			GR_ERROR("unimplemented\n");
		}
		t = next();
		if (t.tok != TOK_NAME)
			GR_ERROR("expected name in icrement/decrement\n");

		struct array_idx_info array_idx;
		struct sym *stack_sym = find_set_stack_ref(f, &t);
		if (!stack_sym) {
			GR_ERROR("unknow variable\n");
		}

		f->sym_string[f->sym_len++].ref = stack_sym;
		return 0;
	}
	if (t_ptr->tok == TOK_NAME || t_ptr->tok == TOK_NAMESPACE) {
		struct sym syms[64];
		int nb_syms = 0;
		int ret = parse_func_call(*t_ptr, f, syms, &nb_syms);
		for (int i = 0; i < nb_syms; ++i, f->sym_len++) {
			f->sym_string[f->sym_len] = syms[i];
		}

		return ret;
	}
	if (t_ptr->tok != TOK_DOLAR && t_ptr->tok != TOK_AT) {
		GR_ERROR("%d: unimplemented operation: %s\n",
		      line_cnt, tok_str[t_ptr->tok]);
	}
	t = next();
	if (t.tok != TOK_NAME)
		GR_ERROR("%d: expected name\n in operation", line_cnt);

	struct array_idx_info array_idx;
	struct sym *stack_sym = find_set_stack_ref(f, &t);
	if (!stack_sym) {
		GR_ERROR("unknow variable\n");
	}
	t = next();
	if (t.tok == TOK_SEMICOL)
		return 0;
	if (parse_array_idx_nfo(f, t, &array_idx) > IDX_IS_NONE) {
		t = next();
	}

	if (t.tok != TOK_EQUAL && t.tok != TOK_PLUS_EQUAL && t.tok != TOK_MINUS_EQUAL &&
		t.tok != TOK_DOT_EQUAL)
		GR_ERROR("%d: unexpected operation %s\n", line_cnt, tok_str[t.tok]);
	struct sym equal_sym = {.ref = stack_sym, .t = t};
	if (array_idx.type >= IDX_IS_NONE) {
		equal_sym.idx = array_idx;
	}
	if (array_idx.type == IDX_IS_NONE && t_ptr->tok == TOK_AT) {
		t = next();
		if (t.tok == TOK_NAME || t.tok == TOK_NAMESPACE) {
			back[nb_back++] = t;
			parse_equal(f, reader, equal_sym);
			return 0;
		} else if (t.tok != TOK_OPEN_PARENTESIS) {
			GR_ERROR("weirdness in array declaration\n");
		}

		f->sym_string[f->sym_len++] = (struct sym){.ref=stack_sym, .t=TOK_ARRAY_RESSET};
		t = next();

		while (t.tok != TOK_CLOSE_PARENTESIS) {
			struct sym elem;
			struct sym *ar = NULL;
			if (t.tok == TOK_OPEN_PARENTESIS) {
				t = next();
				ar = NEW_LOCAL_VAL_INIT("?ar-tmp?");

				f->sym_string[f->sym_len++] = (struct sym){.ref=ar, .t=TOK_ARRAY_RESSET};
				while (t.tok != TOK_CLOSE_PARENTESIS) {
					f->sym_string[f->sym_len++] = (struct sym){.ref=ar, .t=TOK_ARRAY_PUSH};
					struct sym sub_el;
					STORE_OPERAND(t, sub_el);
					f->sym_string[f->sym_len++] = sub_el;
					t = next();
					if (t.tok != TOK_COMMA && t.tok != TOK_CLOSE_PARENTESIS)
						GR_ERROR("unexpected token in array init");
					if (t.tok == TOK_COMMA)
						t = next();
				}
				elem = (struct sym){.t={.tok=TOK_DOLAR}, .ref=ar, .oposite=0};
			} else {
				STORE_OPERAND(t, elem);
			}
			f->sym_string[f->sym_len++] = (struct sym){.ref=stack_sym, .t=TOK_ARRAY_PUSH};
			f->sym_string[f->sym_len++] = elem;
			if (ar) {
				f->sym_string[f->sym_len++] = (struct sym){.ref=ar, .t=TOK_ARRAY_RESSET};
			}

			t = next();
			if (t.tok != TOK_COMMA && t.tok != TOK_CLOSE_PARENTESIS)
				GR_ERROR("unexpected token in array init");
			if (t.tok == TOK_COMMA)
				t = next();
		}
	} else {
		parse_equal(f, reader, equal_sym);
	}

	ret = 0;
exit:
	return ret;
}

static int var_declaration(struct tok t, struct file *f, char **reader)
{
	if (t.tok == TOK_MY) {
		t = next();
		SKIP_REQ(TOK_DOLAR, t);
		if (t.tok != TOK_NAME) {
			GR_ERROR("unexpected token, require %s\n", tok_str[TOK_NAME]); \
		}
		struct sym *v = NEW_LOCAL_VAL();
		v->t = t;
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
	CHECK_SYM_SPACE(f, 258);
	if (t.tok == TOK_MY || t.tok == TOK_DOLAR) {
		var_declaration(t, f, reader);
	} else if (t.tok == TOK_DO) {
		t = next();

		const char *file_name = t.as_str;
		struct stat st;
		int fd;

		if (stat(file_name, &st) < 0 || (fd = open(file_name, O_RDONLY) ) < 0) {
			fprintf(stderr, "cannot open/stat '%s'", file_name);
			return -1;
		}
		int len = st.st_size;
		char *file_str = malloc(len + 1);
		cur_pi->nb_tmp_files += 1;
		cur_pi->tmp_files = realloc(cur_pi->tmp_files,
					    cur_pi->nb_tmp_files * sizeof *cur_pi->tmp_files);
		cur_pi->tmp_files[cur_pi->nb_tmp_files - 1] = file_str;
		char *to_free = file_str;
		if (!file_str || read(fd, file_str, len) < 0) {
			close(fd);
			goto exit;
		}
		file_str[len] = 0;
		char **old_reader = reader_ptr;
		reader = &file_str;
		reader_ptr = reader;
		int line_backup = line_cnt;
		line_cnt = 1;

#define CLEAN_DO() do {					\
			reader = old_reader;		\
			reader_ptr = old_reader;	\
			line_cnt = line_backup;		\
			close(fd);			\
	} while (0)

		// I need to free to free, after perl exit.
		//			free(to_free);


		while ((t = next()).tok != TOK_ENDFILE) {
			if (parse_one_instruction(my_perl, f, reader, t) < 0) {
				CLEAN_DO();
				goto exit;
			}
		}

		CLEAN_DO();
#undef CLEAN_DO

	} else if (t.tok == TOK_RETURN) {
		struct sym ret_sym = {.t = t, .caller = f->cur_func};
		t = next();
		ret_sym.have_return = (t.tok != TOK_SEMICOL);
		if (t.tok == TOK_SEMICOL) {
			f->sym_string[f->sym_len++] = ret_sym;
			return 0;
		}
		back[nb_back++] = t;
		struct sym *ret_val = NEW_LOCAL_VAL();
		struct sym ret_equal = {.t = {.tok=TOK_EQUAL}, .ref = ret_val};
		sym_val_init(ret_val, (struct tok) {.tok=TOK_DOLAR, .as_str="?ret_tmp?"});
		parse_equal(f, reader_ptr, ret_equal);
		f->sym_string[f->sym_len++] = ret_sym;
		f->sym_string[f->sym_len++] = (struct sym){.t={.tok=TOK_DOLAR},
			.ref=ret_val, .oposite=0};
	} else if (t.tok == TOK_SUB) {
		int ret = 0;
		struct sym *goto_end = &f->sym_string[f->sym_len];
		f->sym_string[f->sym_len++].t.tok = TOK_GOTO;

		struct sym *function_begin = &f->sym_string[f->sym_len];
		function_begin->caller = f->cur_func;
		f->cur_func = function_begin;
		f->sym_string[f->sym_len++].t.tok = TOK_SUB;

		NEXT_N_CHECK(TOK_NAME);

		khiter_t iterator = kh_get(func_syms, f->functions, t.as_str);
		if (iterator != kh_end(f->functions)) {
			struct sym *fd = kh_val(f->functions, iterator);
			if (fd->local_stack) {
				function_begin->l_stack_len = fd->l_stack_len;
				function_begin->l_stack_size = fd->l_stack_size;
				function_begin->local_stack = fd->local_stack;
				if (fd->t.tok == TOK_INDIRECT_FUNC)
					free(fd);
			}
		} else {
			ALLOC_L_STACK_SPACE(function_begin, 2048);
			iterator = kh_put(func_syms, f->functions, t.as_str, &ret);
			if (ret < 0)
				GR_ERROR("me hash table fail me, so sad :(\n");
		}
		kh_val(f->functions, iterator) = function_begin;

		struct sym *func_l_stack = function_begin->local_stack;
		func_l_stack[function_begin->l_stack_len].t = (struct tok){.tok=TOK_NAME, .as_str="_"};
		func_l_stack[function_begin->l_stack_len].v.type = SVt_PVAV;
		func_l_stack[function_begin->l_stack_len].v.array_size = 0;
		func_l_stack[function_begin->l_stack_len++].v.array = NULL;

		t = next();
		if (t.tok == TOK_OPEN_PARENTESIS) {
			NEXT_N_CHECK(TOK_CLOSE_PARENTESIS);
			t = next();
		}
		if (t.tok != TOK_OPEN_BRACE) {
			GR_ERROR("%s:%d: unexcected %s, expected '{'\n", __FILE__, __LINE__,
			      tok_str[t.tok]);
		}

		while ((t = next()).tok != TOK_CLOSE_BRACE) {
			if (t.tok == TOK_ENDFILE) {
				GR_ERROR("unclose brace\n");
			}
			if (parse_one_instruction(my_perl, f, reader, t) < 0)
				goto exit;
		}

		/* need to reset array in local stack here */
		PUSH_L_STACK_CLEAR(f, function_begin, 0);
		f->sym_string[f->sym_len].caller = function_begin;
		f->sym_string[f->sym_len].have_return = 0;
		f->sym_string[f->sym_len++].t.tok = TOK_RETURN;
		f->cur_func = f->cur_func->caller;
		goto_end->end = &f->sym_string[f->sym_len];
	} else if (t.tok == TOK_IF) {
		struct sym *elsif_goto_syms[MAX_ELSIF];
		int nb_elseif = 0;
		struct sym *if_sym;
		int tok;
		{
		an_elsif:

			tok = t.tok;
			t = next();
			SKIP_REQ(TOK_OPEN_PARENTESIS, t);
			if_sym = parse_condition(&t, f, reader, tok);
			if (!if_sym)
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
	} else if (t.tok == TOK_FOREACH) {
		t = next();

		// Not need to ne push before we next
		SKIP_REQ(TOK_OPEN_PARENTESIS, t);
		SKIP_REQ(TOK_AT, t);
		struct sym *array = find_set_stack_ref(f, &t);
		t = next();
		SKIP_REQ(TOK_CLOSE_PARENTESIS, t);
		// increase local_stack by one
		struct sym *tmp_i = NEW_LOCAL_VAL_INIT("?foreach_iter?");
		struct sym *underscore = NEW_LOCAL_VAL_INIT("?foreach_underscore?");
		underscore->t = (struct tok){.tok=TOK_NAME, .as_str="_"};
		// TOK_EQUAL local_stack[f->l_stack_len - 1]
		f->sym_string[f->sym_len++] = (struct sym) {.t={.tok=TOK_EQUAL},
			.ref=tmp_i};
		// 0
		f->sym_string[f->sym_len++].t = (struct tok){.tok=TOK_LITERAL_NUM, .as_int=0};
		// struct sym *end = TOK_IF
		struct sym *if_sym = &f->sym_string[f->sym_len];
		f->sym_string[f->sym_len++] = (struct sym){.t={.tok=TOK_IF}};
		// TOK_INF
		f->sym_string[f->sym_len++] = (struct sym){.t={.tok=TOK_INF}};
		// local_stack[f->l_stack_len - 1]
		f->sym_string[f->sym_len++] = (struct sym){.t={.tok=TOK_DOLAR}, .ref=tmp_i,
			.oposite=0};
		// TOK_ARRAY_SIZE array
		f->sym_string[f->sym_len++] = (struct sym){.t={.tok=TOK_ARRAY_SIZE},
			.ref=array};
		// TOK_EQUAL "_"
		f->sym_string[f->sym_len++] = (struct sym) {.t={.tok=TOK_EQUAL},
			.ref=underscore};
		// DOLAR array idx = TOK_DOLAR local_stack[f->l_stack_len - 1]
		f->sym_string[f->sym_len++] = (struct sym) {.t={.tok=TOK_DOLAR}, .ref=array,
			.idx={
				.type=IDX_IS_REF,
				.ref=tmp_i
			}, .oposite=0
		};
		// parse_one_instruction
		parse_one_instruction(my_perl, f, reader, t);
		f->sym_string[f->sym_len++] = (struct sym) {.t={.tok=TOK_EQUAL},
			.ref=array, .idx={
				.type=IDX_IS_REF,
				.ref=tmp_i
			}
		};
		f->sym_string[f->sym_len++] = (struct sym) {.t={.tok=TOK_DOLAR},
			.ref=underscore, .oposite=0
		};

		// TOK_PLUS_PLUS local_stack[f->l_stack_len - 1]
		f->sym_string[f->sym_len++] = (struct sym) {.t={.tok=TOK_PLUS_PLUS}, .ref=tmp_i};
		f->sym_string[f->sym_len++] = (struct sym) {.t={.tok=TOK_GOTO}, .end=if_sym};
		// decrease local stack
		push_free_var(underscore, f->sym_string, &f->sym_len);
		if (f->cur_func) {
			f->cur_func->l_stack_len -= 2;
		} else {
			f->l_stack_len -= 2;
		}
		// end = f->sym_string
		if_sym->end = &f->sym_string[f->sym_len];
	} else if (t.tok == TOK_WHILE) {
		t = next();

		// Not need to ne push before we next
		SKIP_REQ(TOK_OPEN_PARENTESIS, t);

		struct sym *begin_while = parse_condition(&t, f, reader, TOK_IF);
		t = next();

		SKIP_REQ(TOK_CLOSE_PARENTESIS, t);

		parse_one_instruction(my_perl, f, reader, t);

		f->sym_string[f->sym_len].end = begin_while;
		f->sym_string[f->sym_len++].t.tok = TOK_GOTO;
		begin_while->end = &f->sym_string[f->sym_len];
	} else if (t.tok == TOK_FOR) {
		gravier_debug("handling for\n");
		t = next();
		if (t.tok != TOK_OPEN_PARENTESIS) {
			GR_ERROR("unexpected token");
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

		for (int i = nb_for_toks_cnd - 1; i >= 0; --i) {
			back[nb_back++] = for_toks_cnd[i];
		}
		t = next();
		struct sym *out_loop = parse_condition(&t, f, reader, TOK_IF);

		f->sym_string[f->sym_len].t.tok = TOK_GOTO;
		f->sym_string[f->sym_len++].end = begin_for;
		out_loop->end = &f->sym_string[f->sym_len];
		//operation(&t, f, reader);
	} else if (t.tok == TOK_OPEN_BRACE) {
		// humm I have stack locality to handle here...
		int begin_local_stack = f->l_stack_len;
		if (cur_pi->cur_pkg->cur_func)
			begin_local_stack = cur_pi->cur_pkg->cur_func->l_stack_len;

		while ((t = next()).tok != TOK_CLOSE_BRACE) {
			if (t.tok == TOK_ENDFILE) {
				GR_ERROR("unclose brace\n");
			}
			if (parse_one_instruction(my_perl, f, reader, t) < 0)
				goto exit;
		}

		DEC_LOCAL_STACK(begin_local_stack, f->sym_string, &f->sym_len);
		// and stack to remove here

	} else if (t.tok == TOK_SEMICOL) {
		// nothing ...
	} else if (t.tok == TOK_PRINT) {
		int need_close = 0;
		int stack_tmp = 1;
		struct sym *s_ptr = NEW_LOCAL_VAL_INIT("?pri?");

		int base = f->sym_len;

		f->sym_string[f->sym_len++].t = t;
		t = next();
		if (t.tok == TOK_OPEN_PARENTESIS) {
			t = next();
			need_close = 1;
		}
	print_comma:
		if (t.tok == TOK_LITERAL_NUM || t.tok == TOK_LITERAL_STR) {
			f->sym_string[f->sym_len++].t = t;
			t = next();
		} else if (t.tok == TOK_DOLAR || t.tok == TOK_AT) {
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
		} else if (t.tok == TOK_NAME || t.tok == TOK_NAMESPACE) {
			static struct sym syms[128];
			int nb_syms = 0;

			stack_tmp++;
			parse_func_call(t, f, syms, &nb_syms);

			syms[nb_syms++] = (struct sym){.ref=s_ptr, .t=TOK_EQUAL};
			syms[nb_syms++] = (struct sym){.ref=&cur_pi->return_val, .t=TOK_DOLAR,
				.oposite=0};

			int move_len = f->sym_len - base;
			memmove(&f->sym_string[base + nb_syms], &f->sym_string[base],
				sizeof *f->sym_string * move_len);
			for (int i = base; i < base + nb_syms; i++) {
				f->sym_string[i] = syms[i - base];
			}
			base += nb_syms;
			f->sym_len += nb_syms;
			f->sym_string[f->sym_len++] = (struct sym){
				.ref=s_ptr, .t=TOK_DOLAR, .oposite=0
			};
			s_ptr = NEW_LOCAL_VAL_INIT("?pri+?");
			t = next();
		} else {
			GR_ERROR("unexpected %s token\n", tok_str[t.tok]);
		}

		if (t.tok == TOK_COMMA) {
			t = next();
			goto print_comma;
		}
		if (need_close && t.tok == TOK_CLOSE_PARENTESIS) {
			t = next();
		} else if (need_close) {
			GR_ERROR("unclose parensesis in print\n");
		}
		f->sym_string[base].end = &f->sym_string[f->sym_len];
	} else {
		operation(&t, f, reader);
		//f->sym_string[f->sym_len].t = t;
		//f->sym_len += 1;
	}
	return 0;
exit:
	return -1;
}


XS(XS_rand)
{
	dXSARGS;
	intptr_t r = SvIV(ST(0));
	XSRETURN_IV(rand() % r);
}

XS(XS_uc)
{
	dXSARGS;
	struct stack_val *val_0 = ST(0);


	if (val_0->type != SVt_PV) {
		XSRETURN_NO;
	}
	char *cpy = strdup(val_0->str);
	free_var(&cur_pi->return_val.v);
	cur_pi->return_val.v = (struct stack_val){.str=cpy,
		.flag=VAL_NEED_STEAL, .type=SVt_PV};
	for (char *tmp = cpy; *cpy; ++cpy) {
		*cpy = toupper(*cpy);
	}
	return 1;

}

XS(XS_scalar)
{
	dXSARGS;
	struct stack_val *val_0 = ST(0);

	if (val_0->type == SVt_PV) {
		XSRETURN_PV(val_0->str);
	} else if (val_0->type == SVt_IV) {
		XSRETURN_IV(val_0->i);
	} else if (val_0->type == SVt_PVAV) {
		XSRETURN_IV(val_0->array_size);
	}
	XSRETURN_NO;
}

XS(XS_int)
{
	dXSARGS;
	struct stack_val *val_0 = ST(0);

	if (val_0->type == SVt_PV) {
		XSRETURN_IV(atoi(val_0->str));
	} else if (val_0->type == SVt_IV) {
		XSRETURN_IV(val_0->i);
	} else if (val_0->type == SVt_NV) {
		XSRETURN_IV((intptr_t)val_0->f);
	}
	XSRETURN_NO;
}

XS(XS_split)
{
	dXSARGS;
	char *split = SvPVbyte_nolen(ST(0));
	char *str = SvPVbyte_nolen(ST(1));
	int split_l = strlen(split);

	if (!split_l) {
		XSRETURN_NO;
	}

	char *tmp, *tmp2;
	int i = 1;
	tmp2 = str;
	while ((tmp = strstr(tmp2, split))) {
		++i;
		tmp2 = tmp + split_l;
	}
	free_var(&cur_pi->return_val.v);
	cur_pi->return_val.v.type = SVt_PVAV;
	cur_pi->return_val.v.array_size = i;
	cur_pi->return_val.v.array = malloc(i * sizeof *cur_pi->return_val.v.array);

	i = 0;
	for (;(tmp = strstr(str, split)); ++i) {
		cur_pi->return_val.v.array[i].type = SVt_PV;
		cur_pi->return_val.v.array[i].str = strndup(str, tmp - str);
		cur_pi->return_val.v.array[i].flag = VAL_NEED_STEAL;
		str = tmp + split_l;
	}
	cur_pi->return_val.v.array[i].type = SVt_PV;
	cur_pi->return_val.v.array[i].str = strdup(str);
	cur_pi->return_val.v.array[i].flag = VAL_NEED_STEAL;
	return 1;
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

	line_cnt = 1;
	cur_pi = my_perl;

	if (xs_init)
		xs_init(NULL);

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

	my_perl->cur_pkg = this_file;
	kh_val(my_perl->files, iterator) = this_file;

	this_file->file_content = file_str;

	if (!my_perl->first_file) {
		my_perl->first_file = file_name;
	}


	this_file->sym_size = (1 << 16);
	this_file->sym_len = 0;
	this_file->sym_string = malloc(sizeof *this_file->sym_string * this_file->sym_size);
	this_file->stack_size = 2058;
	this_file->stack_len = 0;
	this_file->stack = malloc(sizeof *this_file->stack * this_file->stack_size);
	ALLOC_L_STACK_SPACE(this_file, 2058);
	this_file->functions = kh_init(func_syms);
	sym_val_init(&cur_pi->return_val, (struct tok) {.tok=TOK_DOLAR, .as_str="??RETURN_TMP??"});
	cur_pi->forward_dec = kh_init(forward_func_h);
	this_file->cur_func = NULL;

        newXS_("int", XS_int, this_file);
        newXS_("split", XS_split, this_file);
        newXS_("scalar", XS_scalar, this_file);
        newXS_("uc", XS_uc, this_file);
        newXS_("rand", XS_rand, this_file);

	//printf("file:\n%s\n", file_str);
	reader = file_str;
	reader_ptr = &reader;
	int ret_parsing = TOK_ENDFILE;

	while ((t = next()).tok != ret_parsing) {
		if (parse_one_instruction(my_perl, this_file, &reader, t) < 0)
			goto exit;
	}
	this_file->sym_string[this_file->sym_len].t = t;

	const char *fdkey;
	struct forward_func *fdvar;

	return 0;
exit:
	free(file_str);
	free(this_file);
	return -1;
}

#define MATH_OP(tok_, left, right)				\
	if ((left).type == SVt_NV || tok_ == TOK_DIV_EQUAL) {	\
		(left).flag = 0;				\
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
		(left).flag = 0;				\
		switch (tok_) {					\
		case TOK_PLUS_EQUAL:				\
			(left).i += right; break;		\
		case TOK_MINUS_EQUAL:				\
			(left).i -= right; break;		\
		case TOK_MULT_EQUAL:				\
			(left).i *= right; break;		\
		}						\
	}


static void print_var(struct stack_val *sv, struct sym *sym_string)
{
pr_array_at:
	if (sv->type == SVt_PV)
		printf("%s", sv->str);
	else if (sv->type == SVt_IV)
		printf("%"PRIiPTR, sv->i);
	else if (sv->type == SVt_NV)
		printf("%.15f", sv->f);
	else if (sv->type == SVt_PVAV) {
		struct array_idx_info *idx = &sym_string->idx;
		int i_idx = 0;

		if (idx->type == IDX_IS_TOKEN) {
			i_idx = idx->tok.as_int;
		} else if (idx->type == IDX_IS_REF) {
			struct sym *idx_ref = idx->ref;

			if (idx_ref) {
				i_idx = idx_ref->v.i;
			}
		} else {
			for (int i = 0; i < sv->array_size; ++i) {
				print_var(&sv->array[i], sym_string);
			}
			return;
		}
		if (i_idx < sv->array_size) {
			sv = &sv->array[i_idx];
			goto pr_array_at;
		}
	}

}


static int exec_not(int cnd, int have_not)
{
	if (have_not)
		return !cnd;
	return cnd;
}

static int exec_relational_operator(int tok, struct sym *lop, struct sym *rop, int have_not)
{
	switch (tok) {
	case TOK_DOUBLE_EQUAL:
		if (is_sym_double(lop) || is_sym_double(rop))
			return exec_not(double_fron_sym(lop) == double_fron_sym(rop), have_not);
		return exec_not(int_fron_sym(lop) == int_fron_sym(rop), have_not);
	case TOK_NOT_EQUAL:
		if (is_sym_double(lop) || is_sym_double(rop))
			return exec_not(double_fron_sym(lop) != double_fron_sym(rop), have_not);
		return exec_not(int_fron_sym(lop) != int_fron_sym(rop), have_not);
	case TOK_SUP_EQUAL:
		if (is_sym_double(lop) || is_sym_double(rop))
			return exec_not(double_fron_sym(lop) >= double_fron_sym(rop), have_not);		return exec_not(int_fron_sym(lop) >= int_fron_sym(rop), have_not);
	case TOK_SUP:
		if (is_sym_double(lop) || is_sym_double(rop))
			return exec_not(double_fron_sym(lop) > double_fron_sym(rop), have_not);		return exec_not(int_fron_sym(lop) > int_fron_sym(rop), have_not);
	case TOK_INF_EQUAL:
		if (is_sym_double(lop) || is_sym_double(rop))
			return exec_not(double_fron_sym(lop) <= double_fron_sym(rop), have_not);		return exec_not(int_fron_sym(lop) <= int_fron_sym(rop), have_not);
	case TOK_INF:
		if (is_sym_double(lop) || is_sym_double(rop))
			return exec_not(double_fron_sym(lop) < double_fron_sym(rop), have_not);		return exec_not(int_fron_sym(lop) < int_fron_sym(rop), have_not);
	case TOK_EQ:
		return exec_not(!strcmp(str_fron_sym(lop), str_fron_sym(rop)), have_not);
	}
	return 0;
}

static void exec_dolar_equal(struct stack_val *sv, struct stack_val *that,
			     struct array_idx_info *idx, int oposite)
{
	if (idx && idx->type == IDX_IS_TOKEN) {
		int i_idx = idx->tok.as_int;

		return exec_dolar_equal(sv, &that->array[i_idx], NULL, oposite);
	} else if (idx && idx->type == IDX_IS_REF) {
		struct sym *idx_ref = idx->ref;
		int i_idx = 0;

		if (idx_ref) {
			if (idx_ref->v.type == SVt_NV)
				i_idx = idx_ref->v.f;
			else
				i_idx = idx_ref->v.i;
		}
		return exec_dolar_equal(sv, &that->array[i_idx], NULL, oposite);
	} else {
		if (sv != that)
			free_var(sv);
		if (!that) {
			sv->type = SVt_NULL;
			return;
		}
		*sv = *that;
		if (that->type == SVt_PVAV) {
			sv->array = malloc(sv->array_size * sizeof *sv->array);
			memset(sv->array, 0, sv->array_size * sizeof *sv->array);
			for (int i = 0; i < sv->array_size; ++i) {
				exec_dolar_equal(&sv->array[i], &that->array[i], NULL,
						 oposite);
			}
		} else if (that->type == SVt_IV && oposite) {
			sv->i *= -1;
		}
	}

	if ((sv->flag & (VAL_NEED_FREE | VAL_NEED_STEAL)) && sv->type == SVt_PV) {
		int steal = sv->flag & VAL_NEED_STEAL;

		if (!steal) {
			char *dest = strdup(sv->str);
			if (sv == that && (that->flag & VAL_NEED_FREE))
				free(that->str);
			sv->str = dest;
		} else {
			that->str = NULL;
			that->flag = 0;
		}
		sv->flag = VAL_NEED_FREE;
	} else {
		sv->flag = 0;
	}
}

static struct sym *gogo_regex_engine(struct stack_val *sv, struct sym *sym_string)
{
	struct tok regex = sym_string->t;
	char *target = NULL;
	int search_patern = regex.as_str[0];
	char separator = regex.as_str[1];
	char *operation;
	char *needle;
	char *replace;
	char *dest;
	char *tmp;
	int dest_l = 0;
	int i;

	if (search_patern != 's') {
		fprintf(stderr, "regex on %c not supported\n", search_patern);
		return sym_string + 1;
	}

	++sym_string;
	if (sym_string->t.tok == TOK_LITERAL_STR) {
		target = sym_string->t.as_str;
	} else if (sym_string->t.tok == TOK_DOLAR) {
		target = sym_string->ref->v.str;
	}
	needle = strdup(regex.as_str + 2);
	replace = strchr(needle, separator);
	if (!replace) {
		goto regex_out;
	}
	*replace = 0;
	++replace;
	operation = strchr(replace, separator);
	if (!operation) {
		goto regex_out;
	}
	*operation = 0;
	++operation;
	tmp = strstr(target, needle);
	if (!tmp)
		goto regex_out;

	dest_l = strlen(target);
	dest_l = dest_l << 1;
	dest = malloc(dest_l);

	for (i = 0; target != tmp; ++i, ++target) {
		dest[i] = *target;
	}
	for (int j = 0; replace[j]; ++j, ++i) {
		if (replace[j] == '\\') {
			++j;
			if (replace[j] == 'n')
				dest[i] = '\n';
			else if (target[j] == 't')
				dest[i] = '\t';
			else
				dest[i] = '\\';
		} else {
			dest[i] = replace[j];
		}
	}
	target += strlen(needle);
	for (; *target; ++target, ++i) {
		dest[i] = *target;
	}
	dest[i] = 0;
	free_var(sv);
	sv->type = SVt_PV;
	sv->str = dest;
	sv->flag = VAL_NEED_FREE;

regex_out:
	free(needle);
	return sym_string;

}

static struct sym *exec_equal(struct stack_val *sv, struct sym *sym_string,
			      struct array_idx_info *op_idx)
{

	if (sym_string->t.tok == TOK_REGEX) {
		return gogo_regex_engine(sv, sym_string);
	}
eq_array_at:
	if ((sv->type == SVt_PVAV || sv->type == SVt_NULL) &&
	    op_idx && op_idx->type != IDX_IS_NONE) {
		struct array_idx_info *idx = op_idx;

		op_idx = NULL;
		int i_idx = 0;
		if (idx->type == IDX_IS_TOKEN) {
			i_idx = idx->tok.as_int;
		} else if (idx->type == IDX_IS_REF) {
			struct sym *idx_ref = idx->ref;

			if (idx_ref) {
				if (idx_ref->v.type == SVt_NV)
					i_idx = idx_ref->v.f;
				else
					i_idx = idx_ref->v.i;
			}
		}
		if (i_idx >= sv->array_size) {
			sv->type = SVt_PVAV;
			sv->array_size = i_idx + 1;
			sv->array = realloc(
				sv->array,
				sv->array_size * sizeof *sv->array
				);
		}
		sv = &sv->array[i_idx];
		goto eq_array_at;
	} else if (sym_string->t.tok == TOK_LITERAL_STR) {
		free_var(sv);
		sv->flag = 0;
		sv->str = sym_string->t.as_str;
		sv->type = SVt_PV;
	} else if (sym_string->t.tok == TOK_LITERAL_NUM) {
		free_var(sv);
		sv->i = sym_string->t.as_int;
		sv->flag = 0;
		sv->type = SVt_IV;
	} else if (sym_string->t.tok == TOK_DOLAR || sym_string->t.tok == TOK_AT) {
		exec_dolar_equal(sv, &sym_string->ref->v, &sym_string->idx,
				 sym_string->oposite);
	} else {
		gravier_debug("UNIMPLEMENTED %s\n",
			      tok_str[sym_string->t.tok]);
	}
	return sym_string;
}

static int run_this(struct sym *sym_string, int return_at_return)
{
 	while (sym_string->t.tok != TOK_ENDFILE) {
		struct tok t = sym_string->t;

		switch (t.tok) {

		case TOK_PRINT:
		{
			gravier_debug("in print\n");
			struct sym *end = sym_string->end;
			for (++sym_string; sym_string != end; ++sym_string) {
				if (sym_string->t.tok == TOK_LITERAL_STR) {
					printf("%s", sym_string->t.as_str);
				} else if (sym_string->t.tok == TOK_LITERAL_NUM) {
					printf("%"PRIiPTR, sym_string->t.as_int);
				} else if (sym_string->t.tok == TOK_LITERAL_FLOAT) {
					printf("%"PRIiPTR, sym_string->t.as_int);
				} else if (sym_string->t.tok == TOK_DOLAR) {
					struct sym *ref = sym_string->ref;
					struct stack_val *sv;

					if (!ref) {
						gravier_debug("unknow variable\n");
						continue;
					}
					sv = &ref->v;
					print_var(sv, sym_string);
				}
			}
			fflush(stdout);
			continue;
		}
		break;
		case TOK_ARRAY_RESSET:
			array_free(&sym_string->ref->v);
			break;
		case TOK_FREE_VAR:
			free_var(&sym_string->ref->v);
			break;
		case TOK_ARRAY_PUSH:
		{
			struct sym *array = sym_string->ref;
			int p = array->v.array_size;

			array->v.type = SVt_PVAV;
			array->v.array_size = array->v.array_size + 1;
			array->v.array = realloc(array->v.array,
						 array->v.array_size * sizeof *array->v.array);
			struct stack_val *elem = &array->v.array[p];
			stack_val_init(elem);
			++sym_string;
			exec_equal(elem, sym_string, NULL);
		}
		break;

		case TOK_SUB:
		case TOK_INDIRECT_FUNC:
		{
			struct sym *to_call;
			if (t.tok == TOK_SUB) {
				to_call = sym_string->f_ref;
			} else {
				khiter_t it = sym_string->f_iter;

				to_call = kh_val(sym_string->package->functions, it);
				if (to_call->t.tok == TOK_INDIRECT_FUNC) {
					fprintf(stderr, "Undefined subroutine '%s'\n", kh_key(sym_string->package->functions, it));
					for (int i = 0; i < to_call->l_stack_len; ++i) {
						free_var(&to_call->local_stack[i].v);
					}
					return 1;
				}
			}
			if (to_call->t.tok == TOK_NATIVE_FUNC) {
				to_call->nat_func(to_call, to_call->local_stack[0].v.array_size);
				array_free(&to_call->local_stack[0].v);
			} else {
				if (return_at_return)
					++return_at_return;
				to_call->end = &sym_string[1];
				sym_string = to_call + 1;
				continue;
			}
		}
		break;

		case TOK_RETURN: {
			struct sym *caller = sym_string->caller;
			struct sym *end = caller->end;
			int have_return = sym_string->have_return;
			if (have_return) {
				++sym_string;
				exec_equal(&cur_pi->return_val.v, sym_string, NULL);
			}
			for (int i = 0; i < caller->l_stack_len; ++i) {
				free_var(&caller->local_stack[i].v);
			}
			if (return_at_return) {
				if (return_at_return == 1)
					return have_return;
				else
					--return_at_return;
			}
			sym_string = end;
			continue;
		}
		break;
		case  TOK_IF:
		case TOK_ELSIF:
		{
			struct sym *if_end = sym_string->end;
			int have_not = 0;
			++sym_string;
			t = sym_string->t;
			if (t.tok == TOK_NOT) {
				have_not = 1;
				++sym_string;
				t = sym_string->t;
			}
			int nb = 0;
			_Bool cnd;
			struct sym *rop;
			struct sym *lop = sym_string;

			if (!tok_is_condition(t.tok)) {
				cnd = exec_not(int_fron_sym(lop), have_not);

				++nb;
			} else {
				lop = &sym_string[1];
				rop = &sym_string[2];

				nb = 3;
				cnd = exec_relational_operator(t.tok, lop, rop, have_not);
			}

			while (tok_is_any_logical_operator(sym_string[nb].t.tok)) {
				int logical_op_tok = sym_string[nb].t.tok;
				have_not = 0;

				if (sym_string[nb + 1].t.tok == TOK_NOT) {
					++nb;
					have_not = !have_not;
				}

				struct sym *cnd_sym = &sym_string[nb + 1];
				if (tok_is_condition(cnd_sym->t.tok)) {
					lop = &sym_string[nb + 2];
					rop = &sym_string[nb + 3];

					if (logical_op_tok == TOK_DOUBLE_PIPE ||
					    logical_op_tok == TOK_STR_OR) {
						cnd = cnd || exec_relational_operator(cnd_sym->t.tok, lop, rop, have_not);
					} else {
						cnd = cnd && exec_relational_operator(cnd_sym->t.tok, lop, rop, have_not);
					}
					nb += 4;
				} else {
					lop = &sym_string[nb + 1];
					if (logical_op_tok == TOK_DOUBLE_PIPE ||
					    logical_op_tok == TOK_STR_OR) {
						cnd = cnd || exec_not(int_fron_sym(lop), have_not);
					} else {
						cnd = cnd && exec_not(int_fron_sym(lop), have_not);

					}
					nb += 2;
				}
			}
			gravier_debug("condition result: %d\n", cnd);
			if (cnd) {
				sym_string += nb;
			} else {
				sym_string = if_end;
			}
			continue;
		}
		break;
		case TOK_GOTO:
			sym_string = sym_string->end;
			continue;
		case TOK_EQUAL:
		{
			struct sym *target_ref = sym_string->ref;
			struct array_idx_info *op_idx = &sym_string->idx;

			gravier_debug("SET STUFFF on: %p\n", target_ref);
			++sym_string;

			sym_string = exec_equal(&target_ref->v, sym_string, op_idx);
		}
		break;
		case TOK_PLUS_PLUS:
		case TOK_MINUS_MINUS:
		{
			struct sym *target_ref = sym_string->ref;
			if (t.tok == TOK_PLUS_PLUS)
				target_ref->v.i += 1;
			else
				target_ref->v.i -= 1;
			target_ref->v.flag = 0;
			target_ref->v.type = SVt_IV;
		}
		break;
		case TOK_DOT_EQUAL: {
			struct sym *target_ref = sym_string->ref;
			struct array_idx_info *op_idx = &sym_string->idx;
			static char num_tmp[128];
			static char num_tmp2[128];
			char *first = "";
			char *second = "";
			int first_need_free = (target_ref->v.flag & (VAL_NEED_FREE | VAL_NEED_STEAL));

			++sym_string;
			if (target_ref->v.type == SVt_NULL) {
				sym_string = exec_equal(&target_ref->v, sym_string, op_idx);
				++sym_string;
				continue;
			} else if (target_ref->v.type == SVt_IV) {
				sprintf(num_tmp, "%"PRIiPTR, target_ref->v.i);
				first = num_tmp;
			} else {
				first = target_ref->v.str;
			}

			if (sym_string->t.tok == TOK_LITERAL_STR) {
				second = sym_string->t.as_str;
			} else if (sym_string->t.tok == TOK_LITERAL_NUM) {
				sprintf(num_tmp2, "%"PRIiPTR, sym_string->t.as_int);
				second = num_tmp2;
			} else if (sym_string->t.tok == TOK_DOLAR) {
				struct stack_val *sv = &sym_string->ref->v;

			dot_again:
				if (sv->type == SVt_IV) {
					sprintf(num_tmp2, "%"PRIiPTR, sv->i);
					second = num_tmp2;
				} else if (sv->type == SVt_PVAV) {
					struct array_idx_info *idx = &sym_string->idx;

					if (idx->type == IDX_IS_TOKEN) {
						int i_idx = idx->tok.as_int;

						sv = &sv->array[i_idx];
						goto dot_again;
					} else {
						struct sym *idx_ref = idx->ref;
						int i_idx = 0;

						if (idx_ref) {
							i_idx = idx_ref->v.i;
						}
						sv = &sv->array[i_idx];
						goto dot_again;
					}
				} else {
					second = sv->str;
				}
			}

			int first_len = strlen(first);
			int str_len = first_len + strlen(second);
			if (!str_len) {
				target_ref->v.str = "";
				++sym_string;
				continue;
			}

			str_len += 1;
			target_ref->v.type = SVt_PV;
			target_ref->v.flag |= VAL_NEED_FREE;
			target_ref->v.str = malloc(str_len);
			strcpy(target_ref->v.str, first);
			strcpy(target_ref->v.str + first_len, second);
			if (first_need_free)
				free(first);
		}
		break;
		case TOK_PLUS_EQUAL:
		case TOK_MINUS_EQUAL:
		case TOK_DIV_EQUAL:
		case TOK_MULT_EQUAL:
		{
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
				} else if (sym_string->ref->v.type == SVt_PVAV) {
					struct array_idx_info *idx = &sym_string->idx;
					if (idx->type == IDX_IS_TOKEN) {
						int i_idx = idx->tok.as_int;

						MATH_OP(t.tok, target_ref->v,
							sym_string->ref->v.array[i_idx].i);
					} else if (idx->type == IDX_IS_REF) {
						struct sym *idx_ref = idx->ref;
						int i_idx = 0;

						if (idx_ref) {
							i_idx = idx_ref->v.i;
						}
						MATH_OP(t.tok, target_ref->v,
							sym_string->ref->v.array[i_idx].i);
					}
				}
			} else {
				gravier_debug("UNIMPLEMENTED %s\n",
					      tok_str[sym_string->t.tok]);
			}
			break;
		}
		default:
			fprintf(stderr, "%s unimplemented\n", tok_str[t.tok]);
		}
		gravier_debug("%s ", tok_str[sym_string->t.tok]);
		++sym_string;
	}
	gravier_debug("perl run file\n");
	return 0;
}

static void perl_run_file(PerlInterpreter *perl, struct file *f)
{
	struct sym *sym_string = f->sym_string;

	run_this(sym_string, 0);
}

static void perl_run(PerlInterpreter *perl)
{
	gravier_debug("perl run %s\n", perl->first_file);
	cur_pi = perl;
	khiter_t iterator = kh_get(file_list, perl->files, perl->first_file);
	if (iterator == kh_end(perl->files))
		return;
	perl_run_file(perl, kh_val(perl->files, iterator));
}

#endif
