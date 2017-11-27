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

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "ybytecode.h"
#include "ybytecode-script.h"
#include "entity.h"
#include "rawfile-desc.h"
#include "description.h"
#include "game.h"

static int t = -1;
static void *manager;
static int global;

struct YBytecodeScript {
  YScriptOps ops;
  Entity *map;
};

int YBytecodeScriptDirectReturn;

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

static void *ybytecodeFastCall(void *opacFunc, va_list ap)
{
  Entity *stack  = yeCreateArrayExt(NULL, NULL,
				    YBLOCK_ARRAY_NOINIT |
				    YBLOCK_ARRAY_NOMIDFREE);
  Entity *ret;
  int iret;
  double fret;
  void *dret;

  for (void *tmp = va_arg(ap, void *);
       tmp != Y_END_VA_LIST; tmp = va_arg(ap, void *)) {
    if (yeIsPtrAnEntity(tmp))
      yePushBack(stack, tmp, NULL);
    else
      yeCreateData(tmp, stack, NULL);
  }
  ret = ybytecode_exec(stack, opacFunc);
  yeDestroy(stack);
  if (!ret) {
    if (ybytecode_error) {
      DPRINT_ERR("%s", ybytecode_error);
      g_free(ybytecode_error);
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

static void *ybytecodeCall(void *sm, const char *name, va_list ap)
{
  void *data = ybytecodeGetFastPath(sm, name);
  if (unlikely(!data))
    return NULL;
  return ybytecodeFastCall(data, ap);
}

static int ybytecodeDestroy(void *sm)
{
  if (!sm)
    return -1;
  yeDestroy(GET_MAP(sm));
  g_free(sm);
  return 0;
}

static int isTokSeparato(int tok)
{
  return tok == SPACES_TOK || tok == RETURN_TOK;
}


static int nextNonSeparatorTok(Entity *str, Entity *tokInfo)
{
  int tok;

  while (isTokSeparato((tok = yeStringNextTok(str, tokInfo))));
  return tok;
}

static int tryStoreNumber(int64_t *dest, Entity *str, Entity *tokInfo)
{
  int tok = nextNonSeparatorTok(str, tokInfo);
  int neg_modifier = 1;

  if (tok == YB_SUB_TOK) {
    neg_modifier = -1;
    tok = yeStringNextTok(str, tokInfo);
  }
  if (tok != NUMBER_TOK) {
    DPRINT_ERR("expected number, got '%s'\n", yeTokString(tokInfo, tok));
    return -1;
  }
  *dest = (atoi(yeTokString(tokInfo, tok)) * neg_modifier);
  return 0;
}

static Entity *tryStoreStringCurTok(Entity *funcData, Entity *str,
				    Entity *tokInfo, int tok)
{
  Entity *ret;

  if (tok != DOUBLE_QUOTE_TOK)
    DPRINT_ERR("literal string expected(should begin with '\"', not '%s')",
	       yeTokString(tokInfo, tok));
  ret = yeCreateString(NULL, funcData, NULL);
  while ((tok = yeStringNextTok(str, tokInfo)) != DOUBLE_QUOTE_TOK) {
    if (tok == YTOK_END) {
      yeDestroy(ret);
      return NULL;
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

static int tryStoreLabels(int script_pos, Entity *str,
			  Entity *tokInfo, struct labelHead *labels_head,
			  int tok)
{
  Entity *strTmp = yeCreateString(NULL, NULL, NULL);
  for (tok = tok > 0 ? tok : nextNonSeparatorTok(str, tokInfo);
       tok != COLON_TOK;
       tok = yeStringNextTok(str, tokInfo)) {
    const char *cstr = yeTokString(tokInfo, tok);
    for (int i = 0; cstr[i]; ++i) {
      if (!(yuiIsCharAlphaNum(cstr[i]) || cstr[i] == '_')) {
	goto error;
      }
    }
    yeStringAdd(strTmp, cstr);
  }
  if (!yeGetString(strTmp)) {
    goto error;
  }
  struct labels *label = malloc(sizeof(struct labels));
  label->str = strTmp;
  label->pos = script_pos;
  LIST_INSERT_HEAD(labels_head, label, entries);
  return 0;
 error:
  DPRINT_ERR("label must containe only alphanumeric or '_' caracters");
  yeDestroy(strTmp);
  return -1;
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
  int ret = -1;
  struct labelHead labels = LIST_HEAD_INITIALIZER(labels);
  struct labelHead labels_needed = LIST_HEAD_INITIALIZER(labels_needed);

  script[0] = 0;

  tryStoreNumber(&nb_func_args, str, tokInfo);
  tok = nextNonSeparatorTok(str, tokInfo);

  if (tok != OPEN_BRACE_TOK) {
    DPRINT_ERR("unexpected '%s', expect '{' in function '%s'",
	       yeTokString(tokInfo, tok), yeGetString(funcName));
    goto exit;
  }
 still_in_func:
  tok = yeStringNextTok(str, tokInfo);
 still_in_func_no_next:
  switch (tok) {
  case YB_GET_AT_STR_TOK:
    {
      script[script_len] = tok;
      if (tryStoreNumber(&script[script_len + 1], str, tokInfo) < 0)
	goto exit;
      Entity *tmpStr = tryStoreString(funcData, str, tokInfo);
      if (!tmpStr)
	goto exit;
      script[script_len + 2] = (uintptr_t)yeGetString(tmpStr);
      script_len += 3;
    }
    goto still_in_func;
  case YB_NEW_WIDGET_TOK:
    {
      script[script_len] = tok;
      if (tryStoreNumber(&script[script_len + 1], str, tokInfo) < 0)
	goto exit;
      tok = nextNonSeparatorTok(str, tokInfo);
      if (tok == NIL_TOK) {
	script[script_len + 2] = 0;
      } else {
	Entity *tmpStr = tryStoreStringCurTok(funcData, str, tokInfo, tok);
	if (!tmpStr)
	  goto exit;
	script[script_len + 2] = (uintptr_t)yeGetString(tmpStr);
      }
      script_len += 3;
    }
    goto still_in_func;
  case YB_PUSH_BACK_TOK:
    {
      script[script_len] = tok;
      if (tryStoreNumber(&script[script_len + 1], str, tokInfo) < 0)
	goto exit;
      if (tryStoreNumber(&script[script_len + 2], str, tokInfo) < 0)
	goto exit;
      tok = nextNonSeparatorTok(str, tokInfo);
      if (tok == NIL_TOK) {
	script[script_len + 3] = 0;
      } else {
	Entity *tmpStr = tryStoreStringCurTok(funcData, str, tokInfo, tok);
	if (!tmpStr)
	  goto exit;
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

      if (!tmpStr)
	goto exit;
      script[script_len + 1] = (uintptr_t)yeGetString(tmpStr);
      script_len += 2;
    }
    goto still_in_func;
  case YB_CALL_TOK: /* variadic arguments */
    script[script_len] = tok;
    if (tryStoreNumber(&script[script_len + 2], str, tokInfo) < 0)
      goto exit;
    while ((tok = nextNonSeparatorTok(str, tokInfo)) == NUMBER_TOK) {
      ++nb_args;
      script[script_len + nb_args + 2] = atoi(yeTokString(tokInfo, tok));
    }
    script[script_len + 1] = nb_args;
    script_len += nb_args + 3;
    nb_args = 0;
    goto still_in_func_no_next;
  case YB_ADD_TOK:
  case YB_SUB_TOK:
  case YB_DIV_TOK:
  case YB_MULT_TOK:
    script[script_len] = tok;
    if (tryStoreNumber(&script[script_len + 1], str, tokInfo) < 0)
      goto exit;
    if (tryStoreNumber(&script[script_len + 2], str, tokInfo) < 0)
      goto exit;
    if (tryStoreNumber(&script[script_len + 3], str, tokInfo) < 0)
      goto exit;
    script_len += 4;
    goto still_in_func;
  case YB_SET_INT_TOK:
  case YB_GET_AT_IDX_TOK:
    script[script_len] = tok;
    if (tryStoreNumber(&script[script_len + 1], str, tokInfo) < 0)
      goto exit;
    if (tryStoreNumber(&script[script_len + 2], str, tokInfo) < 0)
      goto exit;
    script_len += 3;
    goto still_in_func;
  case YB_INCR_TOK:
  case YB_RETURN_TOK:
  case YB_CREATE_INT_TOK:
  case YB_PRINT_ENTITY_TOK:
  case YB_REGISTRE_WIDGET_SUBTYPE_TOK:
    script[script_len] = tok;
    if (tryStoreNumber(&script[script_len + 1], str, tokInfo) < 0)
      goto exit;
    script_len += 2;
    goto still_in_func;
  case YB_LEAVE_TOK:
  case YB_CREATE_ARRAY_TOK:
  case YB_PRINT_IRET_TOK:
  case YB_PRINT_POS_TOK:
    script[script_len] = tok;
    script_len += 1;
    goto still_in_func;
  case SPACES_TOK:
  case RETURN_TOK:
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
    if (tryStoreNumber(&script[script_len + 1], str, tokInfo) < 0)
      goto exit;
    if (tryStoreNumber(&script[script_len + 2], str, tokInfo) < 0)
      goto exit;
    if (tryStoreLabels(script_len + 3, str,
		       tokInfo, &labels_needed, -1) < 0)
      goto exit;
    script_len += 4;
    goto still_in_func;
  case YB_JMP_TOK:
    script[script_len] = tok;
    if (tryStoreLabels(script_len + 1, str,
		       tokInfo, &labels_needed, -1) < 0)
      goto exit;
    script_len += 2;
    goto still_in_func;
  default:
    if (!tryStoreLabels(script_len, str,
			tokInfo, &labels, tok))
      goto still_in_func;
    DPRINT_ERR("unexpected '%s' in function '%s'",
	       yeTokString(tokInfo, tok), yeGetString(funcName));
    goto exit;
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
    case SPACES_TOK:
    case RETURN_TOK:
      break;
    case GLOBAL_TOK:
      global = 1;
      tok = nextNonSeparatorTok(str, tokInfo);
      if (tok != YTOK_WORD) {
	DPRINT_ERR("global keyword expect a function");
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
      if (tok != YTOK_END && tok != RETURN_TOK)
	goto still_in_comment;
      break;
    default:
      DPRINT_ERR("error unexpected token: '%s'\n", yeTokString(tokInfo, tok));
      goto exit;
    }
  }
  ret = 0;
 exit:
  return ret;
}

static void *ybytecodeAllocator(void)
{
  struct YBytecodeScript *ybRet = g_new0(struct YBytecodeScript, 1);
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
