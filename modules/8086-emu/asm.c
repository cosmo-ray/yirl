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

union instructions {
	void *label;
	struct {
		uint16_t flag;
		uint16_t constant;
	} info[2];
};

enum {
	IS_WORD = 1 << 0,
	IS_BYTE = 1 << 1,
	IS_REG = 1 << 2,
	HAVE_INDIR = 1 << 3,
	IS_NUM = 1 << 4
};

enum {
	CARRY_FALG = 1
};


/* parsers utils */
static inline int next_no_space(void)
{
	int r;

	do { r = yeStringNextTok(asm_txt, tok_info); } while (r == SPACES_T);
	cur_tok = r;
	return r;
}

#define skip(what)							\
	do {								\
		if (cur_tok != what) {					\
			DPRINT_ERR("unexpected tok: '%s'\n",		\
				   yeTokString(tok_info, cur_tok));	\
			abort();					\
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
		cur_tok = yeStringNextTok(asm_txt, tok_info);
		if (cur_tok != YTOK_WORD && cur_tok != NUMBER_T)
			return INT_MIN;
		return strtol(yeTokString(tok_info, cur_tok), NULL, 16);
	case NUMBER_T:
		return atoi(yeTokString(tok_info, cur_tok));
	case SIMPLE_QUOTE_T:
		cur_tok = yeStringNextTok(asm_txt, tok_info);
		str = yeTokString(tok_info, cur_tok);
		if (strlen(str) != 1) {
			DPRINT_ERR("tok '%s': char should be 1 letter",
				   str);
			abort();
		}
		num = str[0];
		cur_tok = yeStringNextTok(asm_txt, tok_info);
		return num;
	}
	return INT_MIN;
}

static int assign_dest(union instructions *inst, int idx)
{

	switch (cur_tok) {
	case AX_T:
	case BX_T:
	case CX_T:
	case DX_T:
	case DI_T:
	case SI_T:
		inst->info[idx].constant = cur_tok;
		goto reg_word;
	case AL_T:
	case BL_T:
	case CL_T:
	case DL_T:
	case AH_T:
	case BH_T:
	case CH_T:
	case DH_T:
		inst->info[idx].constant = cur_tok;
		goto reg_byte;
	case HEX0_T:
	case HEX1_T:
	case NUMBER_T:
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
	inst->info[idx].flag |= IS_REG;
	return 0;
}

#define unexpected() do {					\
		DPRINT_ERR("unexpected token: %s",		\
			   yeTokString(tok_info, cur_tok));	\
		abort();					\
	} while (0);

static void brace_arg(union instructions *inst, Entity *constants,
		      int sz_set, int idx)
{
	int ret;

	if (!sz_set) {
		DPRINT_ERR("size require");
	}
	printf("%s - %x\n", yeTokString(tok_info, cur_tok),
	       inst->info[0].flag);
	next_no_space();
	ret = assign_dest(inst, 0);
	printf("%d - %s - %x\n", ret, yeTokString(tok_info, cur_tok),
	       inst->info[0].flag);
	if (ret < 0) {
		Entity *const_v;

		const_v = yeGet(constants, yeTokString(tok_info, cur_tok));
		printf("try get %s\n",
		       yeTokString(tok_info, cur_tok));
		if (!const_v)
			unexpected();
		printf("store const %d - %x\n",
		       yeGetIntDirect(const_v),
		       yeGetIntDirect(const_v));
		inst->info[0].flag |= IS_NUM;
		inst->info[0].constant =
			yeGetIntDirect(const_v);
	} else if (ret != 2) {
		DPRINT_ERR("TODO");
		abort();
	}
	next_no_space();
	// check for +/*- here
	skip(CLOSE_BRACKET_T);
	printf("%d - %s - %x\n", ret, yeTokString(tok_info,
						  cur_tok),
	       inst->info[0].flag);

}

static void mk_args(union instructions *insts, int *inst_pos,
		    Entity *constants)
{
	int ret;
	int sz_set = 0;

	next_no_space();
	insts[*inst_pos].info[0].flag = 0;
	insts[*inst_pos].info[1].flag = 0;
	if (cur_tok == BYTE_T || cur_tok == WORD_T) {
		int what = cur_tok == BYTE_T ? IS_BYTE : IS_WORD;

		sz_set = 1;
		insts[*inst_pos].info[0].flag |= what;
		insts[*inst_pos].info[1].flag |= what;
		next_no_space();
	}
	ret = assign_dest(&insts[*inst_pos], 0);

	if (ret == 1) {
		brace_arg(&insts[*inst_pos], constants, sz_set, 0);
	} else if (ret) {
		DPRINT_ERR("error or not yet implemeted %d - %s",
			   *inst_pos,
			   yeTokString(tok_info, cur_tok));
		abort();
	} else {
		next_no_space();
	}
	printf("wesh %s\n", yeTokString(tok_info, cur_tok));
	skip(COMMA_T);
	ret = assign_dest(&insts[*inst_pos], 1);
	if (ret == 1)
		brace_arg(&insts[*inst_pos], constants, sz_set, 1);
	else
		next_no_space();

	printf("end %s\n", yeTokString(tok_info, cur_tok));
	// check for +/*-
	*inst_pos += 1;
	return;
}

static inline void gen_jum(union instructions *insts, int inst_pos,
			   int *inst_size, void *next_label)
{
	if (inst_pos + 10 > *inst_size) {
		*inst_size = *inst_size * 2;
		insts = realloc(insts, *inst_size * sizeof(*insts));
	}
	insts[inst_pos].label = next_label;
}

void *call_asm(int nbArgs, void **args)
{
	Entity *emu = args[0];
	const char *asm_file = yeGetStringAt(emu, "asm");
	yeAutoFree Entity *asm_txt_ = ygFileToEnt(YRAW_FILE, asm_file, NULL);
	yeAutoFree Entity *tok_info_ = yeTokInfoCreate(NULL, NULL);
	struct regs regs;
	int inst_size = 512;
	union instructions *insts = malloc(inst_size);
	int inst_pos = 0;
	int restult; /* last operation result*/
	uint16_t flags; /* last operation result*/

	asm_txt = asm_txt_;
	tok_info = tok_info_;
	YE_NEW(Array, consts, NULL);
	YE_NEW(Array, labels, NULL);

#define DEF(a, b, c) YUI_CAT(DEF_, c)(a, b)
#define DEF_string(a, b) yeCreateString(b, tok_info, NULL);
#define DEF_repeater(a, b) yeTokInfoAddRepeated(b, tok_info);
#define DEF_separated_string(a, b) yeTokInfoAddSepStr(b, tok_info);
#define DEF_separated_repeater(a, b) yeTokInfoAddSepRepStr(b, tok_info);
#include "asm-tok.h"

	while ((cur_tok = yeStringNextTok(asm_txt, tok_info)) != YTOK_END) {
		int ret;

		if (cur_tok == SPACES_T)
			continue;
		if (cur_tok == SEMI_COLON_T) {
			do {
				cur_tok = yeStringNextTok(asm_txt, tok_info);
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
				printf("push %d at %s\n", num,
				       yeGetString(lab_name));
			}
			printf("w: '%s'", yeGetString(lab_name));
			continue;
		}

		if (cur_tok == ADD_T) {
			gen_jum(insts, inst_pos, &inst_size, &&add);
			++inst_pos;
			mk_args(insts, &inst_pos, consts);
			continue;
		} else if (cur_tok == INT_T) {
			gen_jum(insts, inst_pos, &inst_size, &&interupt);
			++inst_pos;
			next_no_space();
			insts[inst_pos].info[0].constant = parse_num();
			++inst_pos;
			continue;
		}

		if (cur_tok == RETURN_T) {
			printf("\n");
			continue;
		}
	}
	printf("hummm ? %d\n", inst_pos);
	inst_pos = 0;
	printf("now time to do real job !!!!!\n%d %p %p\n",
	       inst_pos, insts[inst_pos].label, &&add);
	goto *insts[inst_pos].label;

interupt:
	{
		int int_nb = insts[inst_pos + 1].info[0].constant;

		switch (int_nb) {
		case 0x20:
			goto quit;
		default:
			DPRINT_ERR("int '%x' not yet implemented\n", int_nb);
		}
		inst_pos += 2;
		goto *insts[inst_pos].label;
	}
add:
	printf("add \n");
	inst_pos += 2;
	goto *insts[inst_pos].label;
quit:
	printf("out\n");
	free(insts);
	ygTerminate();
	return (void *)ACTION;
}
