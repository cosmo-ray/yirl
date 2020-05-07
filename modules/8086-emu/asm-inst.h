/*
 * this file is as a *supermacro*:
 * https://www.wanderinghorse.net/computing/papers/supermacros_cpp.html
 * did that instead of a sub-function because I can use OPERATION the way I do
 * using maco would have mean a lot of \ and hard to debug
 * using static inline function would have require more code
 * because it would require 1 sub function per operation
 */

#ifndef CHECK_SUB
#define CHECK_SUB 0
#endif

#ifndef CHECK_ADD
#define CHECK_ADD 0
#endif

#if defined (CHECK_SUB) || defined (CHECK_ADD) || defined (OTHER_CHECK)
#define DO_CHECK 1
#else
#define DO_CHECK 0
#endif

#ifndef COPY
#define COPY 0
#endif

#ifndef STACK
#define STACK -1
#endif

#define SET_FLAG_RESULT()					\
	if (DO_CHECK) {						\
		result = *d;					\
		if (CHECK_SUB) {				\
			if (orig < result)			\
				regs.flag |= CARRY_FLAG;	\
			else					\
				regs.flag &= ~CARRY_FLAG;	\
		} else if (CHECK_ADD) {				\
			if (orig > result)			\
				regs.flag &= ~CARRY_FLAG;	\
			else					\
				regs.flag |= CARRY_FLAG;	\
		}						\
	}							\


#define TRY_INDIR(s, f, v, ptype)				\
	(f & HAVE_INDIR ? *((ptype *)&s->mem[(v) | (regs.ds << 4)]) : (v))

#define DO_OPERATION(type)						\
	if (r1 >= AL_T) {						\
		r1 -= AL_T;						\
	*d OPERATION TRY_INDIR(state, f1, regs.buf_8[r1] + c1, type);	\
	} else if (r1 >= AX_T) {					\
	r1 -= AX_T;							\
	*d OPERATION TRY_INDIR(state, f1, regs.buf_16[r1] + c1,		\
			       type);					\
	} else if (STACK == 1) {					\
		*d OPERATION stack[stack_idx].w;			\
	} else {							\
		*d OPERATION TRY_INDIR(state, f1, c1, type);		\
	}								\
	if (!COPY)							\
		scan_ptr_mem(state, d, 1);				\
	SET_FLAG_RESULT();

{
	int r0 = insts[inst_pos].info[0].reg;
	int r1 = insts[inst_pos].info[1].reg;
	int f0 = insts[inst_pos].info[0].flag;
	int f1 = insts[inst_pos].info[1].flag;
	int c0 = insts[inst_pos].info[0].constant;
	int c1 = insts[inst_pos].info[1].constant;
	int is_0_reg = !!r0;

	if (f0 & IS_BYTE) {
		uint8_t *d;
		uint8_t tmp;
		int is_16_reg;
		/*
		 * we can have byte operation on indirection
		 * ex: mov byte [ax],0x1337
		 */
		if (r0 < AL_T) {
			r0 -= AX_T;
			is_16_reg = 1;
		} else {
			r0 -= AL_T;
			is_16_reg = 0;
		}

		if (f0 & HAVE_INDIR) {
			uint16_t idx;

			if (!is_0_reg)
				idx = c0;
			else if (!is_16_reg)
				idx = regs.buf_8[r0] + c0;
			else
				idx = regs.buf_16[r0] + c0;

			if (COPY == 1) {
				tmp = state->mem[idx | (regs.ds << 4)];
				d = &tmp;
				/* printf("cp: %d - %d - %d - %d - %d - %d - %d\n", */
				/*        tmp, c0, */
				/*        is_0_reg, idx, r0, regs.di, f0 & IS_BYTE); */
			} else  {
				d = &state->mem[idx | (regs.ds << 4)];
			}
		} else if (COPY == 1) {
			tmp = is_0_reg ? regs.buf_8[r0] : c0;
			d = &tmp;
		} else if (STACK == 0) {
			d = &stack[stack_idx].b;
		} else  {
			d = is_0_reg ? &regs.buf_8[r0] : (uint8_t *)&c0;
		}
		uint8_t orig = *d;

		/* printf("8 bit word/registre %x %x\n", regs.ds, regs.es); */
		DO_OPERATION(uint8_t);
	} else {
		if (r0 < AL_T)
			r0 -= AX_T;
		else
			r0 -= AL_T;
#if COPY == 1
		uint16_t tmp = is_0_reg ? regs.buf_16[r0] : c0;
		uint16_t *d = &tmp;
#elif STACK == 0
		uint16_t *d = &stack[stack_idx].w;
#else
		uint16_t *d = is_0_reg ? &regs.buf_16[r0] : (uint16_t *)&c0;
#endif
		uint16_t orig = *d;

		/* printf("16 bit %s(%d)\n", is_0_reg ? "registre" : "word", r0); */
		if (f0 & HAVE_INDIR) {
			uint8_t *tmpd = (uint8_t *)d;
			tmpd = &state->mem[*d | (regs.ds << 4)];
			if (is_0_reg)
				tmpd += (int16_t)c0;
#if COPY == 1
			tmp = *(uint16_t *)tmpd;
			d = &tmp;
#else
			d = (uint16_t *)tmpd;
			/* printf("nv: %x %p %p %d %x %p %d %li\n", *d, d, */
			/*        &state->mem[0xefc | (regs.ds << 4)], */
			/*        (int16_t)c0, regs.di, */
			/*        &state->mem[(0xefc - 2) | (regs.ds << 4)], */
			/*        state->mem[(0xefc - 2) | (regs.ds << 4)], */
			/*        d - (long)&state->mem[(0xefc - 2) | (regs.ds << 4)]); */
#endif
		}

		DO_OPERATION(uint16_t);
		SET_FLAG_RESULT();
	}
}

#undef CHECK_ADD
#undef CHECK_SUB
#undef DO_CHECK
#undef TRY_INDIR
#undef OPERATION

#ifdef OTHER_CHECK
#undef OTHER_CHECK
#endif

#undef COPY

#undef STACK
