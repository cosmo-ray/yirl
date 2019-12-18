
DEF(ADD, "add", separated_string)
DEF(SUB, "sub", separated_string)
DEF(MOV, "mov", separated_string)
DEF(ORG, "org", separated_string)
DEF(EQU, "equ", separated_string)
DEF(INT, "int", separated_string)
DEF(CLD, "cld", separated_string)
DEF(XOR, "xor", separated_string)
DEF(STOSW, "stosw", separated_string)
DEF(STOSB, "stosb", separated_string)
DEF(JMP, "jmp", separated_string)
DEF(INC, "inc", separated_string)
DEF(DEC, "dec", separated_string)
DEF(CMP, "cmp", separated_string)
DEF(JZ, "jz", separated_string)
DEF(JE, "je", separated_string)
DEF(JC, "jc", separated_string)
DEF(JNE, "jne", separated_string)
DEF(JNZ, "jnz", separated_string)
DEF(JNC, "jnc", separated_string)
DEF(IN, "in", separated_string)
DEF(PUSH, "push", separated_string)
DEF(POP, "pop", separated_string)
DEF(JCXZ, "jcxz", separated_string)
DEF(CALL, "call", separated_string)
DEF(RET, "ret", separated_string)
DEF(TEST, "test", separated_string)
DEF(LOOP, "loop", separated_string)
DEF(SHR, "shr", separated_string)
DEF(OR, "or", separated_string)
DEF(AND, "and", separated_string)
DEF(TIMES, "times", separated_string)
DEF(DB, "db", separated_string)

DEF(YIRL_DEBUG, "YIRL_DEBUG", separated_string)

DEF(WORD, "word", separated_string)
DEF(BYTE, "byte", separated_string)


/*
 * register tok need to be in the same order
 * than what they are in struct regs
 */
DEF(AX, "ax", separated_string)
DEF(BX, "bx", separated_string)
DEF(CX, "cx", separated_string)
DEF(DX, "dx", separated_string)
DEF(DI, "di", separated_string)
DEF(SI, "si", separated_string)

DEF(DS, "ds", separated_string)
DEF(ES, "es", separated_string)

DEF(AL, "al", separated_string)
DEF(AH, "ah", separated_string)
DEF(BL, "bl", separated_string)
DEF(BH, "bh", separated_string)
DEF(CL, "cl", separated_string)
DEF(CH, "ch", separated_string)
DEF(DL, "dl", separated_string)
DEF(DH, "dh", separated_string)


DEF(OPEN_BRACKET, "[", string)
DEF(CLOSE_BRACKET, "]", string)
DEF(COLON, ":", string)
DEF(SEMI_COLON, ";", string)
DEF(COMMA, ",", string)
DEF(SPACES, " \t", repeater)
DEF(RETURN, "\n", string)
DEF(HEX0, "0x", string)
DEF(HEX1, "0X", string)
DEF(NUMBER, "0123456789", separated_repeater)
DEF(SIMPLE_QUOTE, "'", string)

DEF(PLUS, "+", string)
DEF(MINUS, "-", string)
DEF(STAR, "*", string)

#ifdef DEF_string
#undef DEF_string
#undef DEF_repeater
#undef DEF_separated_string
#undef DEF_separated_repeated
#endif

#undef DEF
