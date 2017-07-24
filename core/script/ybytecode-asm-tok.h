/*
 * yirl bytecode asm tokens
 */

DEF(OPEN_BRACE, "{", string)
DEF(CLOSE_BRACE, "}", string)
DEF(SPACES, " \t", repeater)
DEF(RETURN, "\n", string)
DEF(CPP_COMMENT, "//", string)
DEF(ADD, "+", string)
DEF(SUB, "-", string)
DEF(DIV, "/", string)
DEF(MULT, "*", string)
DEF(END, "e", string)
DEF(END_RET, "E", string)
DEF(YB_INCR_TOK, "YB_INCR", string)
DEF(NUMBER, "0123456789", separated_repeater)
