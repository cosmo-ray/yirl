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
DEF(DOUBLE_QUOTE, "\"", string)
DEF(BACKSLASH, "\\", string)
DEF(YB_YG_GET_PUSH_TOK, "yb_yg_get_push", string)
DEF(NUMBER, "0123456789", separated_repeater)
DEF(GLOBAL, "global", separated_string)
DEF(CALL_ENTITY, "call_entity", separated_string)
DEF(YB_INCR_TOK, "yb_incr", separated_string)
DEF(END, "leave", separated_string)
DEF(CREATE_INT, "int", separated_string)
DEF(SET_INT, "set_int", separated_string)
DEF(END_RET, "return", separated_string)
