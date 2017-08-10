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
DEF(GLOBAL, "global", string)
DEF(CALL_ENTITY, "CALL_ENTITY", string)
DEF(YB_INCR_TOK, "YB_INCR", string)
DEF(DOUBLE_QUOTE, "\"", string)
DEF(BACKSLASH, "\\", string)
DEF(YB_YG_GET_PUSH_TOK, "YB_YG_GET_PUSH", string)
DEF(NUMBER, "0123456789", separated_repeater)
DEF(END, "e", string)
DEF(END_RET, "E", string)
