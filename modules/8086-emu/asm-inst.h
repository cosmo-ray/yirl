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

#define SET_FLAG_RESULT()					\
	if (DO_CHECK) {						\
		result = *d;					\
		if (CHECK_SUB) {				\
			if (orig > result)			\
				regs.flag |= CARRY_FLAG;	\
			else					\
				regs.flag &= ~CARRY_FLAG;	\
		} else if (CHECK_ADD) {				\
			if (orig < result)			\
				regs.flag &= ~CARRY_FLAG;	\
			else					\
				regs.flag |= CARRY_FLAG;	\
		}						\
	}							\


{
	int r0 = insts[inst_pos].info[0].reg;
	int r1 = insts[inst_pos].info[1].reg;
	int f0 = insts[inst_pos].info[0].flag;
	int f1 = insts[inst_pos].info[1].flag;
	int c0 = insts[inst_pos].info[0].constant;
	int c1 = insts[inst_pos].info[1].constant;
	int is_0_reg = !!r0;

#define try_indir(s, f, v)				\
	f & HAVE_INDIR ? s->mem[(v) | (regs.ds << 4)] : (v)

	if (f0 & IS_BYTE) {
		r0 -= AH_T;
#ifdef COPY
		uint8_t tmp = is_0_reg ? regs.buf_8[r0] : c0;
		uint8_t *d = &tmp;
#else
		uint8_t *d = r0 ? &regs.buf_8[r0] : (uint8_t *)&c0;
#endif
		uint8_t orig = *d;

		printf("8 bit word/registre %x %x\n", regs.ds, regs.es);
		if (f0 & HAVE_INDIR)
			d = &state->mem[*d | (regs.ds << 4)];

		if (r1) {
			r1 -= AH_T;
			*d OPERATION try_indir(state, f1, regs.buf_8[r1] + c1);
		} else {
			*d OPERATION try_indir(state, f1, c1);
		}
#ifndef COPY
		scan_ptr_mem(state, d, 1);
#endif
		SET_FLAG_RESULT()
	} else {
		r0 -= AX_T;
#ifdef COPY
		uint16_t tmp = is_0_reg ? regs.buf_16[r0] : c0;
		uint16_t *d = &tmp;
#else
		uint16_t *d = is_0_reg ? &regs.buf_16[r0] : &c0;
#endif
		uint16_t orig = *d;

		printf("16 bit %s(%d)\n", is_0_reg ? "registre" : "word", r0);
		if (f0 & HAVE_INDIR)
			d = (uint16_t *)&state->mem[*d | (regs.ds << 4)];

		if (r1 > ES_T) {
			r1 -= AH_T;
			*d OPERATION try_indir(state, f1, regs.buf_8[r1] + c1);
		} else if (r1 >= AX_T) {
			r1 -= AX_T;
			*d OPERATION try_indir(state, f1, regs.buf_16[r1] + c1);
		} else {
			printf("op: on const: %x %x %x %x %x\n", f1,
			       try_indir(state, f1, c1), c1,
			       regs.ds << 4, regs.es);
			*d OPERATION try_indir(state, f1, c1);
		}
		SET_FLAG_RESULT()
#ifndef COPY
		scan_ptr_mem(state, d, 2);
#endif
	}
}

#undef CHECK_ADD
#undef CHECK_SUB
#undef DO_CHECK
#undef try_indir
#undef OPERATION

#ifdef OTHER_CHECK
#undef OTHER_CHECK
#endif

#ifdef COPY
#undef COPY
#endif
