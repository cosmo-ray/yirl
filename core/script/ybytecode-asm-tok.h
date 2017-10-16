/*
 * yirl bytecode asm tokens
 */

DEF(CPP_COMMENT, "//", string)
#include "ybytecode-tok.h"
DEF(OPEN_BRACE, "{", string)
DEF(SPACES, " \t", repeater)
DEF(RETURN, "\n", string)
DEF(DOUBLE_QUOTE, "\"", string)
DEF(BACKSLASH, "\\", string)
DEF(COLON, ":", string)
DEF(NIL, "nil", separated_string)
DEF(NUMBER, "0123456789", separated_repeater)
DEF(GLOBAL, "global", separated_string)
