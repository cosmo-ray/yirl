/*
**Copyright (C) 2015 Matthias Gatto
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
#include <glib.h>

#include "tcc-script.h"
#include "debug.h"

static int t = -1;

const char *ysTccPath;
static Entity *includeStrs;
static int tccLoadString(void *sm, const char *str);

#ifdef TCC_FILETYPE_C
#define tcc_add_c_file(s, filename) tcc_add_file(s, filename, TCC_FILETYPE_C)
#else
#define tcc_add_c_file(s, filename) tcc_add_file(s, filename)
#endif

#define GET_OPS(sm) (((YTccScript *)(sm))->ops)
#define GET_TCC_S(sm) (((YTccScript *)(sm))->l)
#define SET_REALLOC_NEEDED(sm) (((YTccScript *)(sm))->needRealloc = 1)
#define UNSET_REALLOC_NEEDED(sm) (((YTccScript *)(sm))->needRealloc = 0)
#define NEED_REALLOC(sm) (((YTccScript *)(sm))->needRealloc)

#define eBError(ua, fmt, args...) do {					\
    asprintf(&ua->error, fmt, args);					\
    ua->should_free = 1;						\
    return -1;								\
  } while (0);

static int gCreateEnt(TCCUserAction *ua, Entity *str,
		      Entity *real_getter, Entity *tmp_name,
		      int neested, int pos)
{
  int tok = tcc_tok();

  if (tok == '{' || tok == '[') {
    int arrayEnd = 1;
    int arrayStart = 0;
    char close = tok == '{' ? '}' : ']';
    YE_NEW(string, father_name,"_Entity_father");

    yeStringAddInt(father_name, neested);
    yeStringAdd(str, "\n{\n");
    yeStringAdd(str, "Entity *");
    yeAddEnt(str, father_name);
    yeStringAdd(str, " = yeReCreateArray(");
    yeAddEnt(str, real_getter);
    yeStringAddCh(str, ',');
    if (!tmp_name) {
      yeStringAdd(str, "NULL");
    } else {
      yeStringAdd(str, "\"");
      yeAddEnt(str, tmp_name);
      yeStringAddCh(str, '"');
    }
    yeStringAdd(str, ", NULL);\n");

    do {
      if (tok == '{') {
	tcc_next();
	if (tcc_tok_is_decimal(tcc_tok())) {
	  arrayStart = atoi(tcc_tok_str());
	  if (arrayStart < 0)
	    eBError(ua, "YEntityBlock, invalide number %d", arrayStart);
	  arrayEnd = arrayStart + 1;
	  tcc_next();
	  if (tcc_tok() == '-') {
	    tcc_next();
	    arrayEnd = atoi(tcc_tok_str());
	    if (arrayEnd <= arrayStart)
	      eBError(ua, "YEntityBlock, invalide number %d", arrayEnd);
	    tcc_next();
	  }
	}
	if (tcc_tok() != ':')
	  eBError(ua, "YEntityBlock, unexpected token: '%s'", tcc_tok_str());
      }
      tcc_next();

      // use a C reserved standart work to avoid colision
      for (int i = arrayStart, j = 0; i < arrayEnd; ++i, ++j)
	gCreateEnt(ua, str, father_name, NULL, neested + 1, arrayStart + j);

      tcc_next();
    } while (tcc_tok() == ',');
    yeStringAdd(str, "};\n");
    if (tcc_tok() != close)
      eBError(ua, "YEntityBlock, unexpected token: '%s'", tcc_tok_str());
  } else {
    const char *curTok = tcc_tok_str();
    TCCSym *s;

    if (tcc_tok_is_decimal(tok))
      yeStringAdd(str, "yeReCreateInt(");
    else if (tcc_tok_is_str(tok))
      yeStringAdd(str, "yeReCreateString(");
    else if (tcc_tok_is_ident(tok)) {
      s = tcc_sym();
      if (tcc_sym_is_function(s)) {
	yeStringAdd(str, "yeReCreateFunction(\"");
	yeStringAdd(str, curTok);
	yeStringAdd(str, "\",ygGetManager(\"tcc\"),");
	goto getter;
      } else if (tcc_sym_is_decimal(s)) {
	yeStringAdd(str, "yeReCreateInt(");
	printf("%s\n", curTok);
      }
    }
    yeStringAdd(str, curTok);
    yeStringAddCh(str, ',');
  getter:
    yeAddEnt(str, real_getter);
    yeStringAddCh(str, ',');
    if (tmp_name) {
      yeStringAddCh(str, '"');
      yeAddEnt(str, tmp_name);
      yeStringAddCh(str, '"');
    } else {
      yeStringAdd(str, "NULL");
    }
    yeStringAdd(str, ");\n");
  }
  return 0;
}

static int tccYEntityBlockCallback(TCCUserAction *ua)
{
  const char *curTok;
  YE_NEW(string, str, "");
  printf("Entity block !!!!!\n");

  tcc_next();
  curTok = tcc_tok_str();
  if (curTok[0] != '{') {
    ua->error = "YEntityBlock, missing '{' at block begining";
    return -1;
  }
  while (1) {
    int isDot;
    YE_NEW(string, tmp_getter, "");
    YE_NEW(string, tmp_sgetter, "");
    YE_NEW(string, tmp_name, "");
    Entity *real_getter = tmp_sgetter;

    tcc_next();
    curTok = tcc_tok_str();
    printf("%s\n", curTok);
    if (!strcmp(curTok, "}"))
      break;
    if (!strcmp(curTok, "<eof>")) {
      return -1;
    }

    yeStringAdd(tmp_sgetter, curTok);
    yeStringAdd(tmp_getter, "yeGetByStr(");
    yeStringAdd(tmp_getter, curTok);
    yeStringAdd(tmp_getter, ", \"");

    tcc_next();
    isDot = tcc_tok() == '.' || tcc_tok() == '[';

    int nbDots = 1;
    int isArray = tcc_tok() == '[';
    int isKeyStr = 0;
    for (; isDot; ++nbDots) {
      tcc_next();
      curTok = tcc_tok_str();
      isKeyStr = tcc_tok_is_str(tcc_tok());
      if (nbDots >= 2) {
	yeAddEnt(tmp_getter, tmp_name);
      }
      yeSetString(tmp_name, curTok);
      tcc_next();
      if (isArray) {
	if (isKeyStr) { /* remove the double quotes in string */
	  yeStringShrink(tmp_name, 1);
	  yeStringTruncate(tmp_name, 1);
	}
	tcc_next(); // ']'
      }
      isDot = tcc_tok() == '.' || tcc_tok() == '[';
      isArray = tcc_tok() == '[';
      if (isDot && nbDots >= 2)
	yeStringAdd(tmp_getter, ".");
    }

    if (nbDots > 2)
      real_getter = tmp_getter;
    yeStringAdd(tmp_getter, "\")");

    if (tcc_tok() != '=')
      eBError(ua, "YEntityBlock, unexpected token: '%s'", tcc_tok_str());
    tcc_next();
    if (gCreateEnt(ua, str, real_getter, tmp_name, 0) < 0)
      return -1;
    tcc_next(); // get ';'
  }

  ua->should_free = 1;
  ua->include_string = strdup(yeGetString(str));
  return TCCAddBlock;
}

#undef eBError

static TCCState *createTCCState(YTccScript *state)
{
  TCCState *l;

  if (state->nbStates > TCC_MAX_SATES)
    return NULL;
  l = tcc_new();
  tcc_set_options(l, "-nostdlib");
  if (l == NULL)
    return NULL;
  tccAddSyms(l);
  if (!ysTccPath) {
    tcc_add_sysinclude_path(l, YIRL_INCLUDE_PATH);
    tcc_set_lib_path(l, TCC_LIB_PATH);
  } else {
    char *includePath = g_strconcat(ysTccPath, "/include/", NULL);
    char *includePath2 = g_strconcat(ysTccPath, "/include/tinycc/", NULL);
    tcc_add_sysinclude_path(l, includePath);
    tcc_add_sysinclude_path(l, includePath2);
    g_free(includePath);
    g_free(includePath2);
    tcc_set_lib_path(l, ysTccPath);
  }
  tcc_add_sysinclude_path(l, YIRL_MODULES_PATH);
  tcc_define_symbol(l, "Y_INSIDE_TCC", NULL);
  tcc_set_output_type(l, TCC_OUTPUT_MEMORY);
  tcc_add_user_token(l, "YEntityBlock", tccYEntityBlockCallback);
  return l;
}

static int tccInit(void *sm, void *args)
{
  YTccScript *state = sm;

  (void)args;
  state->l = createTCCState(state);
  if (!state->l)
    return -1;
  state->states[0] = state->l;
  state->nbStates = 1;

  return 0;
}

static int tccLoadFile(void *sm, const char *filename)
{
  int ret = tcc_add_c_file(GET_TCC_S(sm), filename);
  SET_REALLOC_NEEDED(sm);
  return ret;
}

static int tccLoadString(void *sm, const char *str)
{
  if (tcc_compile_string(GET_TCC_S(sm), str) < 0) {
    DPRINT_ERR("fail to compille:\n%s\n", str);
    return -1;
  }

  SET_REALLOC_NEEDED(sm);
  return 0;
}

static int tccRegistreFunc(void *sm, const char *name, void *arg)
{
  tcc_add_symbol(GET_TCC_S(sm), name, arg);
  SET_REALLOC_NEEDED(sm);
  return 0;
}

static void addFuncSymbole(void *sm, const char *name, int nbArgs, Entity *func)
{
  (void)sm;
  if (!includeStrs) {
    includeStrs = yeCreateString("#include <yirl/entity-script.h>\n" , NULL, NULL);
  }
  if (!name)
    name = yeGetString(func);
  yeAddStr(includeStrs, "void *");
  yeAddStr(includeStrs, name);
  yeAddStr(includeStrs, "(");

  for (int i = 0; i < nbArgs; ++i) {
    if (i)
      yeAddStr(includeStrs, ", ");
    yeAddStr(includeStrs, "void *var");
    yeAddInt(includeStrs, i);
  }

  if (nbArgs) {
    yeAddStr(includeStrs, "){return yesCall(");
  } else {
    yeAddStr(includeStrs, "){return yesCall0(");
  }
  yeAddStr(includeStrs, "(Entity *)");
  yeAddLong(includeStrs, (int_ptr_t)func);

  for (int i = 0; i < nbArgs; ++i) {
    yeAddStr(includeStrs, ",var");
    yeAddInt(includeStrs, i);
  }
  yeStringAdd(includeStrs, ");}");
  SET_REALLOC_NEEDED(sm);
}

static int addDefine(void *sm, const char *name, const char *val)
{
  tcc_define_symbol(GET_TCC_S(sm), name, val);
  return 0;
}

static void *tccGetFastCall(void *scriptManager, const char *name)
{
  YTccScript *state = scriptManager;
  void *ret;

  if (!name) {
    DPRINT_ERR("can not call anonymous function...");
    return NULL;
  }

  if (NEED_REALLOC(scriptManager)) {
    if (includeStrs) {
      tccLoadString(state, yeGetString(includeStrs));
    }
    if (tcc_relocate(state->l, TCC_RELOCATE_AUTO) < 0) {
      DPRINT_ERR("reallocation fail");
      return NULL;
    }
    ret = tcc_get_symbol(state->l, name);
    state->l = createTCCState(state);
    state->states[state->nbStates] = state->l;
    state->needRealloc = 0;
    state->nbStates+= 1;
    if (ret)
      return ret;

    for (int i = 0; i < state->nbStates - 1; ++i) {
      ret = tcc_get_symbol(state->states[i], name);
      if (ret)
	return ret;
    }
  } else {
    for (int i = 0; i < state->nbStates; ++i) {
      ret = tcc_get_symbol(state->states[i], name);
      if (ret)
	return ret;
    }
  }
  return NULL;
}

static void *tccFCall(void *sym, va_list ap)
{
  /* should be declared as thread local */
  static void *args[16];
  int nbArg = 0;

  for (void *tmp = va_arg(ap, void *); tmp != Y_END_VA_LIST;
       tmp = va_arg(ap, void *)) {
    args[nbArg] = tmp;
    ++nbArg;
  }
  return ((void *(*)(int, void **args))sym)(nbArg, args);
}

static void *tccCall(void *sm, const char *name, va_list ap)
{
  void *sym = tccGetFastCall(sm, name);

  if (sym == NULL) {
    DPRINT_ERR("unable to find '%s' symbol", name);
    return NULL;
  }
  return tccFCall(sym, ap);
}

static int tccDestroy(void *sm)
{
  YTccScript *state = sm;

  for (int i = 0; i < state->nbStates; ++i) {
    tcc_delete(state->states[i]);
  }
  g_free(sm);
  return 0;
}

static void *tccAllocator(void)
{
  YTccScript *ret;

  ret = g_new0(YTccScript, 1);
  if (ret == NULL)
    return NULL;
  ret->l = NULL;
  ret->nbStates = 0;
  ret->ops.init = tccInit;
  ret->ops.destroy = tccDestroy;
  ret->ops.loadFile = tccLoadFile;
  ret->ops.loadString = tccLoadString;
  ret->ops.call = tccCall;
  ret->ops.fastCall = tccFCall;
  ret->ops.getFastPath = tccGetFastCall;
  ret->ops.addDefine = addDefine;
  ret->ops.registreFunc = tccRegistreFunc;
  ret->ops.addFuncSymbole = addFuncSymbole;
  return (void *)ret;
}

int ysTccGetType(void)
{
  return t;
}

int ysTccInit(void)
{
  t = ysRegister(tccAllocator);
  return t;
}

int ysTccEnd(void)
{
  yeDestroy(includeStrs);
  includeStrs = NULL;
  return ysUnregiste(t);
}

#undef GET_OPS
#undef GET_TCC_S
