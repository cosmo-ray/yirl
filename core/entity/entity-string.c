/*
**Copyright (C) 2013 Matthias Gatto
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
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<glib.h>
#include	<inttypes.h>
#include	<unistd.h>
#include	"entity.h"
#include	"utils.h"
#include	"stack.h"
#include	"script.h"

int yeStringAdd(Entity *ent, const char *str)
{
  int origLen;
  int totalLength;
  int strLen;

  if (unlikely(!ent || !str))
    return -1;
  strLen = strlen(str);
  origLen = yeLen(ent);
  totalLength = origLen + strLen;
  if (!YE_TO_STRING(ent)->origin) {
    YE_TO_STRING(ent)->value = realloc(YE_TO_STRING(ent)->value,
				       totalLength + 1);
    char *beg = YE_TO_STRING(ent)->value + origLen;
    strncpy(beg, str, strLen + 1);
  } else {
    YE_TO_STRING(ent)->value = g_strdup_printf("%s%s", YE_TO_STRING(ent)->value,
					       str);
    g_free(YE_TO_STRING(ent)->origin);
    YE_TO_STRING(ent)->origin = NULL;
  }
  YE_TO_STRING(ent)->len = totalLength;
  return 0;
}

int yeStringAddNl(Entity *ent, const char *str)
{
  yeStringAdd(ent, str);
  return yeStringAdd(ent, "\n");
}

int yeStringAddInt(Entity *ent, int i)
{
  char *tmp = YE_TO_STRING(ent)->value;

  if (unlikely(!tmp))
    return -1;
  YE_TO_STRING(ent)->value = g_strdup_printf("%s%d", tmp, i);
  YE_TO_STRING(ent)->len = strlen(YE_TO_STRING(ent)->value);
  YE_TO_STRING(ent)->origin = NULL;
  g_free(tmp);
  return 0;
}

int yeStringAddLong(Entity *ent, long i)
{
  char *tmp = YE_TO_STRING(ent)->value;

  if (unlikely(!tmp))
    return -1;
  YE_TO_STRING(ent)->value = g_strdup_printf("%s%ld", tmp, i);
  YE_TO_STRING(ent)->len = strlen(YE_TO_STRING(ent)->value);
  YE_TO_STRING(ent)->origin = NULL;
  g_free(tmp);
  return 0;
}

int yeAddStrFromFd(Entity *e, int fd, int len)
{
  int ret = -1;
  char *tmp = g_new(char, len + 1);

  if (!tmp || read(fd, tmp, len) < 0)
    goto exit;
  tmp[len] = 0;
  if (yeStringAdd(e, tmp) < 0)
    goto exit;
  ret = 0;
 exit:
  g_free(tmp);
  return ret;
}

int yeCountCharacters(Entity *str, char carac, int lineLimit)
{
  const char *cStr = yeGetString(str);
  int ret = 0;

  for (int i = 0; *cStr; ++i, ++cStr) {
    /*
     * if lineLimit is -1, the comparaison between i and lineLimit
     * will never be true
     */
    if (unlikely(*cStr == carac || i == lineLimit)) {
      i = 0;
      ++ret;
    }
  }
  return ret;
}

int yeStringShrink(Entity *str, uint32_t len)
{
  if (len >= yeLen(str))
    return -1;
  if (!YE_TO_STRING(str)->origin)
    YE_TO_STRING(str)->origin = YE_TO_STRING(str)->value;
  YE_TO_STRING(str)->value += len;
  YE_TO_STRING(str)->len -= len;
  return 0;
}

void yeStringShrinkBlank(Entity *s)
{
  const char *str = yeGetString(s);

  if (unlikely(!str))
    return;
  for (; (*str == ' ' || *str == '\t'); ++str);
  if (str == yeGetString(s))
    return;
  if (!YE_TO_STRING(s)->origin)
    YE_TO_STRING(s)->origin = YE_TO_STRING(s)->value;
  YE_TO_STRING(s)->len += str - YE_TO_STRING(s)->value;
  YE_TO_STRING(s)->value = (char *)str;
}

const char *yeStringNextWord(Entity *str, int *len, int shrinkBlank)
{
  int l = 0;
  static char ret[256];
  const char *cStr;

  if (shrinkBlank)
    yeStringShrinkBlank(str);
  cStr = yeGetString(str);

  if (unlikely(!cStr)) {
    DPRINT_ERR("bad str");
    goto error;
  }

  for (; *cStr != '\0' &&  *cStr != ' ' &&
	 *cStr != '\t' && l < 256; ++l, ++cStr) {
    ret[l] = *cStr;
  }

  if (l == 256) {
    DPRINT_ERR("word too long");
    goto error;
  }

  if (len)
    *len = l;

  if (!l) {
    return NULL;
  }
  ret[l] = 0;
  return ret;
 error:
  if (len)
    *len = -1;
  return NULL;
}

enum basic_tok {
  YTOK_STR_BASE
};

static int tryMatchStr(const char *str, Entity *tok, int len)
{
  const char *cstr = yeGetString(tok);
  int tokL = yeLen(tok);
  int i = 0;

  if (len < tokL)
    return 0;
  for (; i < tokL && str[i] == cstr[i]; ++i);
  return i == tokL;
}

static int isOneOf(char a, const char *str)
{
  for (; *str; ++str) {
    if (a == *str)
      return 1;
  }
  return 0;
}

static int tryMatchReapeater(const char *str, Entity *tok, int len)
{
  const char *cstr = yeGetStringAt(tok, 1);
  int i = 0;
  Entity *realStr = yeGet(tok, 3);

  yeSetString(realStr, "");
  for (; i < len && isOneOf(str[i], cstr); ++i) {
    char buf[2] = {0,0};
    buf[0] = str[i];
    yeStringAdd(realStr, buf);
  }
  if (i == 0)
    return 0;
  yeSetAt(tok, 2, i);
  return 1;
}

static int tryMatchSepReap(const char *str, Entity *tok, int len, int beg)
{
  if (!beg)
    return 0;
  return tryMatchReapeater(str, tok, len);
}

static int tryMatchSep(const char *str, Entity *tok, int len, int beg)
{
  if (!beg)
    return 0;
  return tryMatchStr(str, tok, len);
}

static int tryMatchToks(const char *str, Entity *tokInfo, int len, int beg)
{
  if (!str[0]) {
    return YTOK_END;
  }

  for (uint32_t i = 2; i < yeLen(tokInfo); ++i) {
    Entity *curTok = yeGet(tokInfo, i);
    if (yeType(curTok) == YSTRING) { // asume it's a word
      if (tryMatchStr(str, curTok, len))
	return i;
    } else if (yeType(curTok) == YARRAY) { // assume it's a reapeater
      switch (yeGetIntAt(curTok, 0)) {
      case YTOK_TYPE_REAPETED_STR:
	if (tryMatchReapeater(str, curTok, len))
	  return i;
	break;
      case YTOK_TYPE_SEP_REAP_STR:
	if (tryMatchSepReap(str, curTok, len, beg))
	  return i;
	break;
      case YTOK_TYPE_SEPARATE_STR:
	if (tryMatchSep(str, curTok, len, beg))
	  return i;
	break;
      case YTOK_TYPE_STR:
	if (tryMatchStr(str, curTok, len))
	  return i;
	break;
      }
    } else { // assume devloper is a d***head
      return YTOK_ERR;
    }
  }
  return YTOK_WORD;
}

static int tokLen(Entity *tok)
{
    if (yeType(tok) == YSTRING) { // asume it's a word
      return yeLen(tok);
    } else if (yeType(tok) == YARRAY) { // assume it's a reapeater
      switch (yeGetIntAt(tok, 0)) {
      case YTOK_TYPE_REAPETED_STR:
      case YTOK_TYPE_SEP_REAP_STR:
	return yeGetIntAt(tok, 2);
      case YTOK_TYPE_SEPARATE_STR:
      case YTOK_TYPE_STR:
	return yeLen(yeGet(tok, 1));
      }
    }
    return -1;
}

static const char *tokStr(Entity *tok)
{
    if (yeType(tok) == YSTRING) { // asume it's a word
      return yeGetString(tok);
    } else if (yeType(tok) == YARRAY) { // assume it's a reapeater
      switch (yeGetIntAt(tok, 0)) {
      case YTOK_TYPE_REAPETED_STR:
      case YTOK_TYPE_SEP_REAP_STR:
	return yeGetStringAt(tok, 3);
      case YTOK_TYPE_SEPARATE_STR:
      case YTOK_TYPE_STR:
	return yeGetStringAt(tok, 1);
      }
    }
    return NULL;
}

const char *yeTokString(Entity *tokInfo, int tokIdx)
{
  return tokStr(yeGet(tokInfo, tokIdx));
}

int yeTokLen(Entity *tokInfo, int tokIdx)
{
  return tokLen(yeGet(tokInfo, tokIdx));
}

int yeStringNextTok(Entity *str, Entity *tokInfo)
{
  int ret = YTOK_ERR;
  char *cStr;
  const char *begStr;
  int len;
  Entity *word;
  char tmp;

  if (yeGetIntAt(tokInfo, 0) < 0)
    return YTOK_END;
  if (yeType(str) != YSTRING || !tokInfo)
    return ret;
  len = yeLen(str) - yeGetIntAt(tokInfo, 0);
  begStr = yeGetString(str) + yeGetIntAt(tokInfo, 0);
  if ((ret = tryMatchToks(begStr, tokInfo, len, 1)) != YTOK_WORD) {
    if (ret == YTOK_END)
      yeSetIntAt(tokInfo, 0, -1);
    else
      yeAddInt(yeGet(tokInfo, 0), tokLen(yeGet(tokInfo, ret)));
    return ret;
  }
  for (cStr = (char *)begStr + 1, --len;
       tryMatchToks(cStr, tokInfo, len, 0) == YTOK_WORD;
       --len, ++cStr);

  yeSetIntAt(tokInfo, 0, yeLen(str) - len);
  word = yeGet(tokInfo, YTOK_WORD);
  tmp = *cStr;
  *cStr = 0;
  yeSetString(word, begStr);
  *cStr = tmp;
  return YTOK_WORD;
}
