 /*
**Copyright (C) 2016 Matthias Gatto
**
**This program is free software: you can redistribute it and/or modify
**it under the terms of the GNU Lesser General Public License as published by
**the Free Software Foundation, either version 3 of the License, or
**(at your option) any later version.
**
**This program is distributed in the hope that it will be useful,
**but WITHOUT ANY WARRANTY; without even the implied warranty of
**MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**GNU General Public License for more details.
**
**You should have received a copy of the GNU Lesser General Public License
**along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "ybytecode.h"
#include "ybytecode-script.h"
#include "entity.h"
#include "rawfile-desc.h"
#include "description.h"
#include "game.h"

static int t = -1;
static int line;
static const char *curFileName;
static void *manager;
static int global;

static jmp_buf error_env;

struct YBytecodeScript {
  YScriptOps ops;
  Entity *map;
};

int YBytecodeScriptDirectReturn;

#define	YBYTECODE_ERROR(format, args...)	do {			\
    printf("[ %s : %d]\n" format "\n",					\
	   curFileName, line, ## args);					\
  } while (0)


#define DEF(a, b, c) YUI_CAT(a, _TOK),

enum asm_toks {
  YTOK_STR_BASE,
#include "ybytecode-asm-tok.h"
};

#undef DEF

#define GET_MAP(sm) (((struct YBytecodeScript *)sm)->map)

static int ybytecodeRegistreFunc(void *sm, const char *name, void *arg)
{
  yeCreateData(arg, GET_MAP(sm), name);
  return 0;
}

Entity *ysYbytecodeCreateFunc(Entity *data, Entity *father, const char *name)
{
  Entity *ret;

  yePushBack(GET_MAP(ysYBytecodeManager()), data, name);
  ret = yeCreateFunctionExt(name, manager, father, name,
			    YE_FUNC_NO_FASTPATH_INIT);
  YE_TO_FUNC(ret)->fastPath = yeGetData(data);
  return ret;
}

static void *ybytecodeGetFastPath(void *sm, const char *name)
{
  return yeGetData(yeGet(GET_MAP(sm), name));
}

static void *ybytecodeFastCall(void *sm, void *opacFunc, int nb,
			       union ycall_arg *args,
			       int *types)
{
	Entity *stack  = yeCreateArrayExt(NULL, NULL,
					  YBLOCK_ARRAY_NOINIT |
					  YBLOCK_ARRAY_NOMIDFREE);
	Entity *ret;
	int iret;
	double fret;
	void *dret;

	for (int i = 0; i < nb; ++i) {
		void *tmp = args[i].vptr;
		if (yeIsPtrAnEntity(tmp))
			yePushBack(stack, tmp, NULL);
		else
			yeCreateData(tmp, stack, NULL);
	}
	ret = ybytecode_exec(stack, opacFunc);
	yeDestroy(stack);
	if (!ret) {
		if (ybytecode_error) {
			YBYTECODE_ERROR("%s", ybytecode_error);
			free(ybytecode_error);
			ybytecode_error = NULL;
		}
		return NULL;
	}
	if (YBytecodeScriptDirectReturn)
		return ret;

	switch (yeType(ret)) {
	case YINT:
		iret = yeGetInt(ret);
		yeDestroy(ret);
		return (void *)(size_t)iret;
	case YFLOAT:
		fret = yeGetFloat(ret);
		yeDestroy(ret);
		return (void *)(size_t)fret;
	case YDATA:
		dret = yeGetData(ret);
		yeDestroy(ret);
		return dret;
	default:
		break;
	}
	return ret;
}

static void *ybytecodeCall(void *sm, const char *name, int nb,
			   union ycall_arg *args, int *types)
{
  void *data = ybytecodeGetFastPath(sm, name);
  if (unlikely(!data))
    return NULL;
  return ybytecodeFastCall(sm, data, nb, args, types);
}

static int ybytecodeDestroy(void *sm)
{
  if (!sm)
    return -1;
  yeDestroy(GET_MAP(sm));
  free(sm);
  return 0;
}

static int isTokSeparato(int tok)
{
  return tok == SPACES_TOK || tok == RETURN_TOK;
}


static int nextNonSeparatorTok(Entity *str, Entity *tokInfo)
{
  int tok;

  while (isTokSeparato((tok = yeStringNextTok(str, tokInfo)))) {
    if (tok == RETURN_TOK)
      ++line;
  }
  return tok;
}

static void tryStoreNumber(int64_t *dest, Entity *str, Entity *tokInfo)
{
  int tok = nextNonSeparatorTok(str, tokInfo);
  int neg_modifier = 1;

  if (tok == YB_SUB_TOK) {
    neg_modifier = -1;
    tok = yeStringNextTok(str, tokInfo);
  }
  if (tok != NUMBER_TOK) {
    YBYTECODE_ERROR("expected number, got '%s'\n", yeTokString(tokInfo, tok));
    longjmp(error_env, 1);
  }
  *dest = (atoi(yeTokString(tokInfo, tok)) * neg_modifier);
}

struct identifiers {
  const char *str;
  int num;
  LIST_ENTRY(identifiers) entries;
};

LIST_HEAD(identifiersHead, identifiers);

static int getIdent(struct identifiersHead *identHead, const char *name)
{
  struct identifiers *iden;

  LIST_FOREACH(iden, identHead, entries) {
    if (name && !strcmp(iden->str, name)) {
      return iden->num;
    }
  }
  return -1;
}

static void storeIdent(Entity *str, Entity *tokInfo,
		      struct identifiersHead *identHead, int indent, int tok)
{
  const char *cstr = yeTokCIdentifier(tokInfo, tok);

  if (!cstr) {
    YBYTECODE_ERROR("expected identifier, got '%s'\n", yeTokString(tokInfo, tok));
    longjmp(error_env, 1);
  }

  if (getIdent(identHead, cstr) >= 0) {
      YBYTECODE_ERROR("redefinition of '%s'\n", cstr);
      longjmp(error_env, 1);
  }
  struct identifiers *iden = malloc(sizeof(struct identifiers));
  iden->str = strdup(cstr);
  iden->num = indent;
  LIST_INSERT_HEAD(identHead, iden, entries);
}

static int isIdentifier(Entity *str, Entity *tokInfo, int tok,
			struct identifiersHead *identHead)
{
  const char *cstr;

  if (tok == NUMBER_TOK)
    return 1;
  cstr = yeTokCIdentifier(tokInfo, tok);
  return getIdent(identHead, cstr) >= 0;
}

static void tryGetIdentifier(int64_t *dest, Entity *str, Entity *tokInfo,
			    struct identifiersHead *identHead)
{
  int tok = nextNonSeparatorTok(str, tokInfo);
  const char *cstr;

  if (tok == NUMBER_TOK) {
    *dest = atoi(yeTokString(tokInfo, tok));
    return;
  }

  cstr = yeTokCIdentifier(tokInfo, tok);
  if (!cstr) {
    YBYTECODE_ERROR("expected identifier, got '%s'\n", yeTokString(tokInfo, tok));
    longjmp(error_env, 1);
  }
  *dest = getIdent(identHead, cstr);
  if (*dest < 0) {
    YBYTECODE_ERROR("undeclared indentifier: '%s'\n", cstr);
    longjmp(error_env, 1);
  }
}

static Entity *tryStoreStringCurTok(Entity *funcData, Entity *str,
				    Entity *tokInfo, int tok)
{
  Entity *ret;

  if (tok != DOUBLE_QUOTE_TOK) {
    YBYTECODE_ERROR("literal string expected(should begin with '\"', not '%s')",
		    yeTokString(tokInfo, tok));
    longjmp(error_env, 1);
  }
  ret = yeCreateString("", funcData, NULL);
  while ((tok = yeStringNextTok(str, tokInfo)) != DOUBLE_QUOTE_TOK) {
    if (tok == YTOK_END) {
      YBYTECODE_ERROR("\" is missing to close the string");
      yeDestroy(ret);
      longjmp(error_env, 1);
    }
    yeStringAdd(ret, yeTokString(tokInfo, tok));
  }
  return ret;
}

static Entity *tryStoreString(Entity *funcData, Entity *str, Entity *tokInfo)
{
  int tok = nextNonSeparatorTok(str, tokInfo);

  return tryStoreStringCurTok(funcData, str, tokInfo, tok);
}

struct labels {
  Entity *str;
  int pos;
  LIST_ENTRY(labels) entries;
};

LIST_HEAD(labelHead, labels);

static void tryStoreLabels(int script_pos, Entity *str,
			   Entity *tokInfo, struct labelHead *labels_head,
			   int tok)
{
  Entity *strTmp = yeCreateString(NULL, NULL, NULL);
  for (tok = tok > 0 ? tok : nextNonSeparatorTok(str, tokInfo);
       tok != COLON_TOK;
       tok = yeStringNextTok(str, tokInfo)) {
    const char *cstr = yeTokCIdentifier(tokInfo, tok);
    if (!cstr)
      goto error;
    yeStringAdd(strTmp, cstr);
  }
  if (!yeGetString(strTmp)) {
    goto error;
  }
  struct labels *label = malloc(sizeof(struct labels));
  label->str = strTmp;
  label->pos = script_pos;
  LIST_INSERT_HEAD(labels_head, label, entries);
  return;
 error:
  YBYTECODE_ERROR("label must containe only alphanumeric or '_' caracters");
  yeDestroy(strTmp);
  longjmp(error_env, 1);
}

static int linkLabels(struct labelHead *labels,
		      struct labelHead *labels_needed,
		      int64_t *script, uint32_t script_len)
{
  struct labels *lab = NULL;
  struct labels *lab2 = NULL;

  if (!labels_needed)
    return 0;
  LIST_FOREACH(lab, labels_needed, entries) {
    LIST_FOREACH(lab2, labels, entries) {
      if (!yeStrCmp(lab->str, yeGetString(lab2->str))) {
	script[lab->pos] = lab2->pos;
	goto next;
      }
    }
    return -1;
  next:
    continue;
  }
  return 0;
}

static int parseFunction(Entity *map, Entity *str, Entity *tokInfo)
{
  int tok = YTOK_WORD;
  Entity *funcName = yeCreateString(yeTokString(tokInfo, tok), NULL, NULL);
  Entity *funcData = yeCreateArray(NULL, NULL);
  int64_t script[1024];
  uint32_t script_len = 1;
  int nb_args = 0;
  int64_t nb_func_args = 0;
  int have_return = 0;
  int current_identifier;
  int ret = -1;
  struct labelHead labels = LIST_HEAD_INITIALIZER(labels);
  struct labelHead labels_needed = LIST_HEAD_INITIALIZER(labels_needed);
  struct identifiersHead idents = LIST_HEAD_INITIALIZER(idents);

#define INCREMENT_IDENTIFIER() do {		\
    ++current_identifier;			\
    have_return = 1;				\
  } while (0)

  /*
   * This is buggy, we should remove all identifiers
   * that containe current_identifier as number
   */
#define DECREMENT_IDENTIFIER() do {		\
    --current_identifier;			\
  } while (0)

  script[0] = 0;

  if (setjmp(error_env) == 1)
    goto exit;

  tryStoreNumber(&nb_func_args, str, tokInfo);
  tok = nextNonSeparatorTok(str, tokInfo);

  if (tok != OPEN_BRACE_TOK) {
    YBYTECODE_ERROR("unexpected '%s', expect '{' in function '%s'",
	       yeTokString(tokInfo, tok), yeGetString(funcName));
    goto exit;
  }
  current_identifier = nb_func_args - 1;
 still_in_func:
  tok = yeStringNextTok(str, tokInfo);
 still_in_func_no_next:
  // handle return here, check have_return, and check for a '=>'
  if (have_return && tok == ASSIGNATION_TOK) {
    tok = nextNonSeparatorTok(str, tokInfo);
    storeIdent(str, tokInfo, &idents, current_identifier, tok);
    have_return = 0;
    goto still_in_func;
  }
  switch (tok) {
  case YB_GET_AT_STR_TOK:
  case YB_TRY_GET_AT_STR_TOK:
    {
      script[script_len] = tok;
      tryStoreNumber(&script[script_len + 1], str, tokInfo);
      Entity *tmpStr = tryStoreString(funcData, str, tokInfo);
      script[script_len + 2] = (uintptr_t)yeGetString(tmpStr);
      script_len += 3;
    }
    INCREMENT_IDENTIFIER();
    goto still_in_func;
  case YB_NEW_WIDGET_TOK:
    {
      script[script_len] = tok;
      tryGetIdentifier(&script[script_len + 1], str, tokInfo, &idents);
      tok = nextNonSeparatorTok(str, tokInfo);
      if (tok == NIL_TOK) {
	script[script_len + 2] = 0;
      } else {
	Entity *tmpStr = tryStoreStringCurTok(funcData, str, tokInfo, tok);
	script[script_len + 2] = (uintptr_t)yeGetString(tmpStr);
      }
      script_len += 3;
    }
    INCREMENT_IDENTIFIER();
    goto still_in_func;
  case YB_PUSH_BACK_TOK:
    {
      script[script_len] = tok;
      tryGetIdentifier(&script[script_len + 1], str, tokInfo, &idents);
      tryGetIdentifier(&script[script_len + 2], str, tokInfo, &idents);
      tok = nextNonSeparatorTok(str, tokInfo);
      if (tok == NIL_TOK) {
	script[script_len + 3] = 0;
      } else {
	Entity *tmpStr = tryStoreStringCurTok(funcData, str, tokInfo, tok);
	script[script_len + 3] = (uintptr_t)yeGetString(tmpStr);
      }
      script_len += 4;
    }
    goto still_in_func;
  case YB_YG_GET_PUSH_TOK: /* literal string argument */
  case YB_CREATE_STRING_TOK:
    {
      script[script_len] = tok;

      Entity *tmpStr = tryStoreString(funcData, str, tokInfo);

      script[script_len + 1] = (uintptr_t)yeGetString(tmpStr);
      script_len += 2;
    }
    INCREMENT_IDENTIFIER();
    goto still_in_func;

  case YB_CALL_TOK: /* variadic arguments */
    script[script_len] = tok;
    tryGetIdentifier(&script[script_len + 2], str, tokInfo, &idents);
    while (isIdentifier(str, tokInfo, (tok = nextNonSeparatorTok(str, tokInfo)),
			&idents)) {
      ++nb_args;
      if (tok == NUMBER_TOK)
	script[script_len + nb_args + 2] = atoi(yeTokString(tokInfo, tok));
      else
	script[script_len + nb_args + 2] = getIdent(&idents, yeTokCIdentifier(tokInfo, tok));
    }
    script[script_len + 1] = nb_args;
    script_len += nb_args + 3;
    nb_args = 0;
    INCREMENT_IDENTIFIER();
    goto still_in_func_no_next;

  case YB_ADD_TOK:
  case YB_SUB_TOK:
  case YB_DIV_TOK:
  case YB_MULT_TOK:
    script[script_len] = tok;
    tryStoreNumber(&script[script_len + 1], str, tokInfo);
    tryStoreNumber(&script[script_len + 2], str, tokInfo);
    tryStoreNumber(&script[script_len + 3], str, tokInfo);
    script_len += 4;
    goto still_in_func;

  case YB_SET_INT_TOK:
  case YB_GET_AT_IDX_TOK:
  case YB_STRING_ADD_CH_TOK:
  case YB_TRUNCATE_TOK:
    script[script_len] = tok;
    tryGetIdentifier(&script[script_len + 1], str, tokInfo, &idents);
    tryStoreNumber(&script[script_len + 2], str, tokInfo);
    script_len += 3;
    if (tok == YB_GET_AT_IDX_TOK)
      INCREMENT_IDENTIFIER();
    goto still_in_func;
  case YB_STRING_ADD_CH_ENT_TOK:
    script[script_len] = tok;
    tryGetIdentifier(&script[script_len + 1], str, tokInfo, &idents);
    tryGetIdentifier(&script[script_len + 2], str, tokInfo, &idents);
    script_len += 3;
    goto still_in_func;
  case YB_INCR_TOK:
  case YB_DECR_TOK:
  case YB_RETURN_TOK:
  case YB_PRINT_NBR:
  case YB_CREATE_INT_TOK:
  case YB_PRINT_ENTITY_TOK:
  case YB_LEN_TOK:
  case YB_REGISTRE_WIDGET_SUBTYPE_TOK:
    script[script_len] = tok;
    tryGetIdentifier(&script[script_len + 1], str, tokInfo, &idents);
    script_len += 2;
    if (tok == YB_CREATE_INT_TOK || tok == YB_LEN_TOK)
      INCREMENT_IDENTIFIER();
    goto still_in_func;
  case YB_LEAVE_TOK:
  case YB_STACK_POP_TOK:
  case YB_CREATE_ARRAY_TOK:
  case YB_PRINT_IRET_TOK:
  case YB_PRINT_STACK_TOK:
  case YB_PRINT_POS_TOK:
  case YB_NEXT_TOK:
    script[script_len] = tok;
    script_len += 1;
    if (tok == YB_STACK_POP_TOK)
      DECREMENT_IDENTIFIER();
    goto still_in_func;
  case RETURN_TOK:
    ++line;
    /* FALLTHROUGH */
  case SPACES_TOK:
    goto still_in_func;
  case YB_END_FUNC_TOK:
    script[script_len] = tok;
    script_len += 1;
    ret = 0;
    break;
  case YB_EQUAL_TOK:
  case YB_NOT_EQUAL_NBR_TOK:
  case YB_EQUAL_NBR_TOK:
  case YB_INF_COMP_NBR_TOK:
  case YB_SUP_COMP_NBR_TOK:
    script[script_len] = tok;
    tryGetIdentifier(&script[script_len + 1], str, tokInfo, &idents);
    tryStoreNumber(&script[script_len + 2], str, tokInfo);
    tryStoreLabels(script_len + 3, str, tokInfo, &labels_needed, -1);
    script_len += 4;
    goto still_in_func;
  case YB_JMP_TOK:
  case YB_JMP_IF_0_TOK:
    script[script_len] = tok;
    tryStoreLabels(script_len + 1, str,
		   tokInfo, &labels_needed, -1);
    script_len += 2;
    goto still_in_func;
  default:
    tryStoreLabels(script_len, str, tokInfo, &labels, tok);
    goto still_in_func;
  }
  Entity *data;

  if (linkLabels(&labels, &labels_needed, script, script_len) < 0)
    goto exit;
  if (script_len < (yeMetadataSize(DataEntity) / sizeof(uint64_t))) {
    data = yeCreateDataExt(NULL, NULL, NULL, YE_DATA_USE_OWN_METADATA);
  } else {
    void *realData = malloc(sizeof(uint64_t) * script_len);

    data = yeCreateData(realData, NULL, NULL);
    yeSetDestroy(data, free);
  }
  memcpy(yeGetData(data), script, sizeof(uint64_t) * script_len);
  yePushBack(map, data, yeGetString(funcName));
  if (GLOBAL_TOK) {
    ygRegistreFunc(ysYBytecodeManager(), nb_func_args,
		   yeGetString(funcName), yeGetString(funcName));
  }
  yeStringAdd(funcName, ".data");
  yePushBack(map, funcData, yeGetString(funcName));
  yeDestroy(data);
 exit:
  while (!LIST_EMPTY(&idents)) {
    struct identifiers *n1 = LIST_FIRST(&idents);
    free((char *)n1->str);
    LIST_REMOVE(n1, entries);
    free(n1);
  }

  while (!LIST_EMPTY(&labels_needed)) {           /* List Deletion. */
    struct labels *n1 = LIST_FIRST(&labels_needed);
    LIST_REMOVE(n1, entries);
    yeDestroy(n1->str);
    free(n1);
  }

  while (!LIST_EMPTY(&labels)) {            /* List Deletion. */
    struct labels *n1 = LIST_FIRST(&labels);
    LIST_REMOVE(n1, entries);
    yeDestroy(n1->str);
    free(n1);
  }

  yeDestroy(funcName);
  yeDestroy(funcData);
  return ret;
}

static int loadFile(void *opac, const char *fileName)
{
  Entity *map = GET_MAP(opac);
  Entity *str = ygFileToEnt(YRAW_FILE, fileName, NULL);
  int tok;
  int ret = -1;
  Entity *tokInfo = yeTokInfoCreate(NULL, NULL);
  curFileName = fileName;

  line = 1;

  /* populate tokInfo, withstuff declare inside "ybytecode-asm-tok.h" */
#define DEF(a, b, c) YUI_CAT(DEF_, c)(a, b)
#define DEF_string(a, b) yeCreateString(b, tokInfo, NULL);
#define DEF_repeater(a, b) yeTokInfoAddRepeated(b, tokInfo);
#define DEF_separated_string(a, b) yeTokInfoAddSepStr(b, tokInfo);
#define DEF_separated_repeater(a, b) yeTokInfoAddSepRepStr(b, tokInfo);

#include "ybytecode-asm-tok.h"

#undef DEF_string
#undef DEF_repeater
#undef DEF_separated_string
#undef DEF_separated_repeated
#undef DEF

  while ((tok = yeStringNextTok(str, tokInfo)) != YTOK_END) {
    switch (tok) {
    case RETURN_TOK:
      ++line;
      /* FALLTHROUGH */
    case SPACES_TOK:
      break;
    case GLOBAL_TOK:
      global = 1;
      tok = nextNonSeparatorTok(str, tokInfo);
      if (tok != YTOK_WORD) {
	YBYTECODE_ERROR("global keyword expect a function");
	goto exit;
      }
      /* fall through */
    case YTOK_WORD:
      if (parseFunction(map, str, tokInfo) < 0)
	goto exit;
      global = 0;
      break;
    case CPP_COMMENT_TOK:
    still_in_comment:
      tok = yeStringNextTok(str, tokInfo);
      if (tok != YTOK_END && tok != RETURN_TOK) {
	++line;
	goto still_in_comment;
      }
      break;
    default:
      YBYTECODE_ERROR("error unexpected token: '%s'\n", yeTokString(tokInfo, tok));
      goto exit;
    }
  }
  ret = 0;
 exit:
  yeMultDestroy(tokInfo, str);
  return ret;
}

static void *ybytecodeAllocator(void)
{
  struct YBytecodeScript *ybRet = y_new0(struct YBytecodeScript, 1);
  YScriptOps *ret = &ybRet->ops;

  if (ret == NULL)
    return NULL;
  ybRet->map = yeCreateArray(NULL, NULL);
  ret->call = ybytecodeCall;
  ret->getFastPath = ybytecodeGetFastPath;
  ret->loadFile = loadFile;
  ret->fastCall = ybytecodeFastCall;
  ret->registreFunc = ybytecodeRegistreFunc;
  ret->destroy = ybytecodeDestroy;
  return (void *)ret;
}

static int ysYBytecodeInit(void)
{
  t = ysRegister(ybytecodeAllocator);
  return t;
}

void * ysYBytecodeManager(void)
{
  if (manager)
    return manager;
  ysYBytecodeInit();
  manager = ysNewManager(NULL, t);
  return manager;
}

int ysYBytecodeEnd(void)
{
  ysDestroyManager(manager);
  manager = NULL;
  return ysUnregiste(t);
}
