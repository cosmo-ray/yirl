/**        DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                   Version 2, December 2004
 *
 * Copyright (C) 2019 Matthias Gatto <uso.cosmo.ray@gmail.com>
 *
 * Everyone is permitted to copy and distribute verbatim or modified
 * copies of this license document, and changing it is allowed as long
 * as the name is changed.
 *
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *  0. You just DO WHAT THE FUCK YOU WANT TO.
 */


#define DEF(a, b, c) YUI_CAT(a, _T),
static enum asm_toks {
	YTOK_STR_BASE,
#include "asm-tok.h"
};


static Entity *asm_txt;
static Entity *tok_info;
int cur_tok;
int line_cnt;

struct inst_param {
	uint8_t flag;
	uint8_t reg;
	uint16_t constant;
};

union instructions {
	void *label;
	struct inst_param info[2];
};

enum {
	IS_WORD = 1 << 0,
	IS_BYTE = 1 << 1,
	HAVE_INDIR = 1 << 2,
	IS_NUM = 1 << 3,
	ADD = 1 << 4,
	MUL = 1 << 5,
	DIV = 1 << 6
};

// sub in informel, because I use add
#define SUB 1

enum {
	CARRY_FLAG = 1,
	DIR_FLAG = 2
};

#define unexpected() do {					\
		DPRINT_ERR("unexpected token: %s",		\
			   yeTokString(tok_info, cur_tok));	\
		abort();					\
	} while (0);

static inline int next(void)
{
	int r = yeStringNextTok(asm_txt, tok_info);

	if (r == RETURN_T)
		++line_cnt;
	return r;
}

/* parsers utils */
static inline int next_no_space(void)
{
	int r;

	do { r = next(); } while (r == SPACES_T);
	cur_tok = r;
	return r;
}

#define skip(what)							\
	do {								\
		if (cur_tok != what) {					\
			unexpected();					\
		}							\
		next_no_space();					\
	} while (0);							\

static inline int parse_num(void)
{
	char *str;
	int num;

	switch (cur_tok) {
	case HEX0_T:
	case HEX1_T:
		cur_tok = next();
		if (cur_tok != YTOK_WORD && cur_tok != NUMBER_T)
			return INT_MIN;
		return strtol(yeTokString(tok_info, cur_tok), NULL, 16);
	case NUMBER_T:
		return atoi(yeTokString(tok_info, cur_tok));
	case SIMPLE_QUOTE_T:
		cur_tok = next();
		str = yeTokString(tok_info, cur_tok);
		if (strlen(str) != 1) {
			DPRINT_ERR("tok '%s': char should be 1 letter",
				   str);
			abort();
		}
		num = str[0];
		cur_tok = next();
		return num;
	}
	unexpected();
	return 0;
}

int skip_next_num()
{
	int r;

	next_no_space();
	r = parse_num();
	next_no_space();
	return r;
}

static int check_math(int base, int *op)
{
	if (cur_tok == STAR_T) {
		int num = skip_next_num();
		if (op) {
			*op = MUL;
			return num;
		}
		return check_math(base * num, NULL);
	} else if (cur_tok == PLUS_T) {
		int num = skip_next_num();
		num = check_math(num, NULL);
		if (op) {
			printf("n op\n");
			*op = ADD;
			return num;
		}
		return base + num;
	} else if (cur_tok == MINUS_T) {
		int num = skip_next_num();
		num = check_math(num, NULL);
		if (op) {
			*op = SUB;
			return num;
		}
		return base - num;
	}
	return base;
}

static void do_math(union instructions *inst, int idx)
{
	int c = inst->info[idx].constant;
	int r = inst->info[idx].reg;
	int f = 0;
	int *fp = r ? &f : NULL;

	inst->info[idx].constant = check_math(c, fp);
	if (f == SUB) {
		inst->info[idx].constant = -inst->info[idx].constant;
		f = ADD;
	}
	inst->info[idx].flag |= f;
}

static int assign_dest(union instructions *inst, int idx)
{

	switch (cur_tok) {
	case AX_T: case BX_T: case CX_T: case DX_T:
	case DI_T: case SI_T: case DS_T: case ES_T:
		inst->info[idx].reg = cur_tok;
		goto reg_word;
	case AL_T: case BL_T: case CL_T: case DL_T:
	case AH_T: case BH_T: case CH_T: case DH_T:
		inst->info[idx].reg = cur_tok;
		goto reg_byte;
	case HEX0_T: case HEX1_T: case NUMBER_T:
	case SIMPLE_QUOTE_T:
		inst->info[idx].flag |= IS_NUM;
		inst->info[idx].constant = parse_num();
		return 2;
	case OPEN_BRACKET_T:
		inst->info[idx].flag |= HAVE_INDIR;
		return 1;
	default:
		return -1;
	}
reg_word:
	inst->info[idx].flag |= IS_WORD;
	goto reg;
reg_byte:
	inst->info[idx].flag |= IS_BYTE;
reg:
	return 0;
}

static void brace_arg(union instructions *inst, Entity *constants,
		      int sz_set, int idx)
{
	int ret;

	if (!sz_set) {
		DPRINT_ERR("size require");
	}
	next_no_space();
	ret = assign_dest(inst, idx);
	if (ret < 0) {
		Entity *const_v;

		const_v = yeGet(constants, yeTokString(tok_info, cur_tok));
		if (!const_v)
			unexpected();
		inst->info[idx].flag |= IS_NUM;
		inst->info[idx].constant =
			yeGetIntDirect(const_v);
	}
	next_no_space();
	do_math(inst, idx);
	skip(CLOSE_BRACKET_T);

}

static void mk_args(union instructions *insts, int inst_pos,
		    Entity *constants, _Bool single_arg)
{
	int ret;
	int sz_set = 0;

	next_no_space();
	insts[inst_pos].info[0] = (struct inst_param){0};
	insts[inst_pos].info[1] = (struct inst_param){0};
	printf("init: %d\n", insts[inst_pos].info[0].reg);
	if (cur_tok == BYTE_T || cur_tok == WORD_T) {
		int what = cur_tok == BYTE_T ? IS_BYTE : IS_WORD;

		sz_set = 1;
		insts[inst_pos].info[0].flag |= what;
		insts[inst_pos].info[1].flag |= what;
		next_no_space();
	}
	ret = assign_dest(&insts[inst_pos], 0);

	if (ret == 1) {
		brace_arg(&insts[inst_pos], constants, sz_set, 0);
	} else if (ret) {
		DPRINT_ERR("error or not yet implemeted %d - %s",
			   inst_pos,
			   yeTokString(tok_info, cur_tok));
		abort();
	} else {
		next_no_space();
	}
	if (single_arg)
		return;
	skip(COMMA_T);
	ret = assign_dest(&insts[inst_pos], 1);
	if (ret == 1)
		brace_arg(&insts[inst_pos], constants, sz_set, 1);
	else
		next_no_space();

	do_math(&insts[inst_pos], 1);
	return;
}

#define gen_jum(insts, inst_pos, inst_size, next_label)		\
	do {							\
		if (inst_pos + 10 > inst_size) {			\
			inst_size = inst_size * 2;			\
			insts = realloc(insts, inst_size * sizeof(*insts)); \
		}							\
		insts[inst_pos].label = next_label;			\
	} while (0)							\

void *call_asm(int nbArgs, void **args)
{
	Entity *emu = args[0];
	struct state_8086 *state = yeGetDataAt(emu, "state");
	const char *asm_file = yeGetStringAt(emu, "asm");
	yeAutoFree Entity *asm_txt_ = ygFileToEnt(YRAW_FILE, asm_file, NULL);
	yeAutoFree Entity *tok_info_ = yeTokInfoCreate(NULL, NULL);
	struct regs regs;
	int inst_size = 512;
	union instructions *insts = malloc(inst_size * (sizeof *insts));
	int inst_pos = 0;
	int result; /* last operation result*/
	int ret_stack[128];
	int ret_idx = 0;

	line_cnt = 1;  /* lines are retarded... SHOULD START AT 0 */
	asm_txt = asm_txt_;
	tok_info = tok_info_;
	YE_NEW(Array, consts, NULL);
	YE_NEW(Array, labels, NULL);
	YE_NEW(Array, forward_labels, NULL);

#define DEF(a, b, c) YUI_CAT(DEF_, c)(a, b)
#define DEF_string(a, b) yeCreateString(b, tok_info, NULL);
#define DEF_repeater(a, b) yeTokInfoAddRepeated(b, tok_info);
#define DEF_separated_string(a, b) yeTokInfoAddSepStr(b, tok_info);
#define DEF_separated_repeater(a, b) yeTokInfoAddSepRepStr(b, tok_info);
#include "asm-tok.h"

	while ((cur_tok = next()) != YTOK_END) {
		int ret;

	continue_no_next_tok:
		if (cur_tok == SPACES_T)
			continue;
		if (cur_tok == SEMI_COLON_T) {
			do {
				cur_tok = next();
				if (cur_tok == YIRL_DEBUG_T) {
					gen_jum(insts, inst_pos, inst_size, &&yirl_debug);
					++inst_pos;
					insts[inst_pos].info[0].constant = line_cnt;
					++inst_pos;

				}
			} while (cur_tok != YTOK_END && cur_tok != RETURN_T);
			continue;
		}

		if (cur_tok == YTOK_WORD) {
			int ok = 0;

			YE_NEW(string, lab_name, yeTokString(tok_info, cur_tok));
			cur_tok = next_no_space();
			if (cur_tok == COLON_T) {
				yeCreateInt(inst_pos, labels,
					    yeGetString(lab_name));
				cur_tok = next_no_space();
			}
			if (cur_tok == EQU_T) {
				int num;
				cur_tok = next_no_space();

				num = parse_num();
				if (num == INT_MIN) {
					DPRINT_ERR("problem with '%s' parsing",
						   yeGetString(lab_name));
					abort();

				}
				yeCreateInt(num, consts, yeGetString(lab_name));
				printf("-------\npush const %d at %s\n-------\n", num,
				       yeGetString(lab_name));
			}
			printf("w: '%s'", yeGetString(lab_name));
			continue;
		}

		if (cur_tok == ADD_T) {
			gen_jum(insts, inst_pos, inst_size, &&add);
			++inst_pos;
			mk_args(insts, inst_pos, consts, 0);
			++inst_pos;
		} else if (cur_tok == SUB_T) {
			gen_jum(insts, inst_pos, inst_size, &&sub);
			++inst_pos;
			mk_args(insts, inst_pos, consts, 0);
			++inst_pos;
		} else if (cur_tok == MOV_T) {
			gen_jum(insts, inst_pos, inst_size, &&mov);
			++inst_pos;
			mk_args(insts, inst_pos, consts, 0);
			++inst_pos;
		} else if (cur_tok == INC_T || cur_tok == DEC_T) {
			if (cur_tok == INC_T)
				gen_jum(insts, inst_pos, inst_size, &&add);
			else
				gen_jum(insts, inst_pos, inst_size, &&sub);
			++inst_pos;
			mk_args(insts, inst_pos, consts, 1);
			insts[inst_pos].info[1].flag |= IS_NUM;
			insts[inst_pos].info[1].constant = 1;
			++inst_pos;
		} else if (cur_tok == XOR_T) {
			/* xor le sherif, sherif de l'espace */
			/* xor son domaine, c'est notre GALAXYYYYYYY */
			/* Homme ou robot, il change de paux */
			/* Quand de l'espace, vien la menace... */
			/* ^ That's Gavan opening in french */
			/* Gavan name came from the french actor jean Gabin */
			/* I find it odd than Gavan was rename X-or in france */
			gen_jum(insts, inst_pos, inst_size, &&xor);
			++inst_pos;
			mk_args(insts, inst_pos, consts, 0);
			++inst_pos;
		} else if (cur_tok == INT_T) {
			gen_jum(insts, inst_pos, inst_size, &&interupt);
			++inst_pos;
			next_no_space();
			insts[inst_pos].info[0].constant = parse_num();
			++inst_pos;
		} else if (cur_tok == CMP_T) {
			gen_jum(insts, inst_pos, inst_size, &&cmp);
			++inst_pos;
			mk_args(insts, inst_pos, consts, 0);
			++inst_pos;
		} else if (cur_tok == CLD_T) {
			gen_jum(insts, inst_pos, inst_size, &&cld);
			++inst_pos;
		} else if (cur_tok == STOSW_T) {
			gen_jum(insts, inst_pos, inst_size, &&stosw);
			++inst_pos;
		} else if (cur_tok == RET_T) {
			gen_jum(insts, inst_pos, inst_size, &&ret);
			++inst_pos;
		} else if (cur_tok == LOOP_T) {
			gen_jum(insts, inst_pos, inst_size, &&loop);
			goto create_jmp;
		} else if (cur_tok == CALL_T) {
			gen_jum(insts, inst_pos, inst_size, &&call);
			goto create_jmp;
		} else if (cur_tok == JCXZ_T) {
			gen_jum(insts, inst_pos, inst_size, &&jcxz);
			goto create_jmp;
		} else if (cur_tok == JNC_T) {
			gen_jum(insts, inst_pos, inst_size, &&jnc);
			goto create_jmp;
		} else if (cur_tok == JC_T) {
			gen_jum(insts, inst_pos, inst_size, &&jc);
			goto create_jmp;
		} else if (cur_tok == JNZ_T) {
			gen_jum(insts, inst_pos, inst_size, &&jnz);
			goto create_jmp;
		} else if (cur_tok == JZ_T) {
			gen_jum(insts, inst_pos, inst_size, &&jz);
			goto create_jmp;
		} else if (cur_tok == JMP_T) {
			Entity *jmp_dest;
			const char *l_name;

			gen_jum(insts, inst_pos, inst_size, &&jmp);
		create_jmp:

			++inst_pos;
			next_no_space();
			l_name = yeTokString(tok_info, cur_tok);
			jmp_dest = yeGet(labels, l_name);
			if (yeType(jmp_dest) != YINT) {
				yeCreateInt(inst_pos, forward_labels, l_name);
			} else {
				insts[inst_pos].info[0].constant =
					yeGetIntDirect(jmp_dest);
			}
			++inst_pos;
		}

		if (cur_tok == RETURN_T) {
			continue;
		}
	}
	for (int i = 0; i < yeLen(forward_labels); ++i) {
		const char *l_name = yeGetKeyAt(forward_labels, i);
		int inst_i = yeGetIntAt(forward_labels, i);
		Entity *jmp_dest = yeGet(labels, l_name);

		if (yeType(jmp_dest) != YINT) {
			DPRINT_ERR("can't find label: %s", l_name);
			abort();

		}
		insts[inst_i].info[0].constant = yeGetIntDirect(jmp_dest);
	}
	printf("hummm ? %d\n", inst_pos);
	inst_pos = 0;
	printf("now time to do real job !!!!!\n%d %p %p\n",
	       inst_pos, insts[inst_pos].label, &&add);
	goto *insts[inst_pos].label;

interupt:
	{
		int int_nb = insts[inst_pos + 1].info[0].constant;
		Entity *eve;
		Entity *events;

		printf("int 0x%x\n", int_nb);
		switch (int_nb) {
		case 0x10:
			printf("new video mode %d\n", regs.ax);
			set_video_mode(state, regs.ax);
			break;
		case 0x16:
			events = ywidGenericPollEvent();

			YEVE_FOREACH(eve, events) {
				if (ywidEveType(eve) == YKEY_DOWN) {
					regs.al = ywidEveKey(eve);
				}
			}
			printf("input: %d\n", regs.al);
			break;
		case 0x1a:
			/* 18.5 hz is a computer tick, and 18.2 hz == 54945 us */
			regs.dx = YTimerGet(&state->timer) / 54945;
			printf("time %d\n", regs.dx);
			break;
		case 0x20:
			goto quit;
		default:
			DPRINT_ERR("int '%x' not yet implemented\n", int_nb);
		}
		inst_pos += 2;
		goto *insts[inst_pos].label;
	}
yirl_debug:
	printf("YIRL DEBUG line %d\n", insts[inst_pos + 1].info[0].constant);
	inst_pos += 2;
	goto *insts[inst_pos].label;
stosb:
	state->mem[regs.es + (regs.ds << 4)] = regs.al;
	if (regs.flag & DIR_FLAG)
		regs.di += 1;
	else
		regs.di -= 1;
	++inst_pos;
	goto *insts[inst_pos].label;
stosw:
	state->mem[regs.es + (regs.ds << 4)] = regs.ah;
	state->mem[1 + regs.es + (regs.ds << 4)] = regs.al;
	if (regs.flag & DIR_FLAG)
		regs.di += 2;
	else
		regs.di -= 2;
	++inst_pos;
	goto *insts[inst_pos].label;
cld:
	regs.flag &= ~DIR_FLAG;
	++inst_pos;
	goto *insts[inst_pos].label;
loop:
	if (regs.flag & DIR_FLAG)
		++regs.cx;
	else
		--regs.cx;
	if (regs.cx) {
		inst_pos = insts[inst_pos + 1].info[0].constant;
	} else {
		inst_pos += 2;
	}
	goto *insts[inst_pos].label;
jcxz:
	if (!regs.cx)
		inst_pos = insts[inst_pos + 1].info[0].constant;
	else
		inst_pos += 2;
	goto *insts[inst_pos].label;
jnc:
	if (regs.flag & CARRY_FLAG)
		inst_pos += 2;
	else
		inst_pos = insts[inst_pos + 1].info[0].constant;
	goto *insts[inst_pos].label;
jc:
	if (regs.flag & CARRY_FLAG)
		inst_pos = insts[inst_pos + 1].info[0].constant;
	else
		inst_pos += 2;
	goto *insts[inst_pos].label;
jnz:
	printf("jnz\n");
	if (result)
		inst_pos = insts[inst_pos + 1].info[0].constant;
	else
		inst_pos += 2;
	goto *insts[inst_pos].label;
jz:
	printf("jz\n");
	if (!result)
		inst_pos = insts[inst_pos + 1].info[0].constant;
	else
		inst_pos += 2;
	goto *insts[inst_pos].label;
ret:
	printf("ret to %d\n", ret_stack[ret_idx - 1]);
	inst_pos = ret_stack[--ret_idx];
	goto *insts[inst_pos].label;
call:
	printf("call %d\n", ret_idx);
	ret_stack[ret_idx++] = inst_pos + 2;
	/* fallthough */
jmp:
	inst_pos = insts[inst_pos + 1].info[0].constant;
	goto *insts[inst_pos].label;
cmp:
	++inst_pos;
#define COPY
#define CHECK_SUB 1
#define OPERATION -=
#include "asm-inst.h"
	++inst_pos;
	printf("cmp (%d)%x (%d)%x\n",
	       insts[inst_pos].info[0].reg,
	       insts[inst_pos].info[0].constant,
	       insts[inst_pos].info[1].reg,
	       insts[inst_pos].info[1].constant);
	goto *insts[inst_pos].label;
sub:
	++inst_pos;
#define OPERATION -=
#define CHECK_ADD 1
#include "asm-inst.h"
	printf("add (%d)%x (%d)%x\n",
	       insts[inst_pos].info[0].reg,
	       insts[inst_pos].info[0].constant,
	       insts[inst_pos].info[1].reg,
	       insts[inst_pos].info[1].constant);
	++inst_pos;
	goto *insts[inst_pos].label;
add:
	++inst_pos;
#define OPERATION +=
#define CHECK_ADD 1
#include "asm-inst.h"
	printf("add (%d)%x (%d)%x\n",
	       insts[inst_pos].info[0].reg,
	       insts[inst_pos].info[0].constant,
	       insts[inst_pos].info[1].reg,
	       insts[inst_pos].info[1].constant);
	++inst_pos;
	goto *insts[inst_pos].label;
mov:
	++inst_pos;
#define OPERATION =
#include "asm-inst.h"
	printf("mov (%d)%x (%d)%x\n",
	       insts[inst_pos].info[0].reg,
	       insts[inst_pos].info[0].constant,
	       insts[inst_pos].info[1].reg,
	       insts[inst_pos].info[1].constant);
	++inst_pos;
	goto *insts[inst_pos].label;
xor:
	++inst_pos;
#define OTHER_CHECK 1
#define OPERATION ^=
#include "asm-inst.h"
	printf("xor (%d)%x (%d)%x\n",
	       insts[inst_pos].info[0].reg,
	       insts[inst_pos].info[0].constant,
	       insts[inst_pos].info[1].reg,
	       insts[inst_pos].info[1].constant);
	++inst_pos;
	goto *insts[inst_pos].label;

quit:
	printf("out\n");
	free(insts);
	ygTerminate();
	printf("really out\n");
	return (void *)ACTION;
}
