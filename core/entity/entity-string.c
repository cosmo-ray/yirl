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
#include	<ctype.h>
#include	"entity.h"
#include	"utils.h"
#include	"stack.h"
#include	"script.h"
#include	"game.h"

Entity *yeToLower(Entity *e)
{
  char *c = (char *)yeGetString(e);

  if (unlikely(!c))
    return NULL;

  for (; *c; ++c) {
    *c = tolower(*c);
  }
  return e;
}

Entity *yeStringAddTmpEnd(Entity *e, size_t newEndPos,
			  Entity *father, const char *name)
{
  Entity *ret;
  size_t l = yeLen(e);
  char *str = YE_TO_STRING(e)->value;

  if (newEndPos >= l)
    return NULL;
  ret = yeCreateArray(father, name);
  yeCreateInt(l, ret, NULL);
  yeCreateNString(&str[newEndPos], 1, ret, NULL);
  str[newEndPos] = '\0';
  YE_TO_STRING(e)->len = strlen(str);
  return ret;
}

void yeStringRestoreEnd(Entity *e, Entity *oldEnd, int shouldFree)
{
  int oldLen = YE_TO_STRING(e)->len;
  YE_TO_STRING(e)->len = yeGetIntAt(oldEnd, 0);
  YE_TO_STRING(e)->value[oldLen] = yeGetStringAt(oldEnd, 1)[0];
  if (shouldFree)
    yeDestroy(oldEnd);
}

signed char yeStringReplaceCharAt(Entity *ent, char c, size_t at)
{
  int ret;
  char *str = YE_TO_STRING(ent)->value;

  if (at >= yeLen(ent))
    return -1;
  ret = str[at];
  str[at] = c;
  return ret;
}

int yeStringReplace(Entity *ent, const char *substr, const char *replacement)
{
  int substrlen = strlen(substr);
  int replacementlen = strlen(replacement);
  const char *entChar = yeGetString(ent);
  int entLen;
  int16_t begs[256];
  int nbFound = 0;
  char *dest;
  char *toFree;
  int destLen;

  if (!substrlen || !ent)
    return 0;
  entLen = yeLen(ent);
  dest = (char *)entChar;
  while ((dest = strstr(dest, substr)) != NULL) {
    if (nbFound == 256)
      return -1;
    begs[nbFound] = dest - entChar;
    ++dest;
    ++nbFound;
  }

  if (!nbFound)
    return 0;
  toFree = (char *)entChar;
  destLen = entLen - (substrlen * nbFound) + (replacementlen * nbFound);
  dest = malloc(destLen + 1);
  YE_TO_STRING(ent)->value = dest;
  for (int i = 0; i < nbFound; ++i) {
    int len = begs[i];
    if (i > 0) {
      len -= (begs[i - 1] + substrlen);
    }
    memcpy(dest, entChar, len);
    dest += len;
    memcpy(dest, replacement, replacementlen);
    dest += replacementlen;
    entChar += len + substrlen;
  }
  memcpy(dest, entChar, entLen - (begs[nbFound -1] + substrlen) + 1);
  if (YE_TO_STRING(ent)->origin) {
    free(YE_TO_STRING(ent)->origin);
    YE_TO_STRING(ent)->origin = NULL;
  } else {
    free(toFree);
  }
  YE_TO_STRING(ent)->len = destLen;
  return nbFound;
}

int yeStringAddChByEntity(Entity *ent, Entity *ch)
{
  return yeStringAddCh(ent, yeGetInt(ch));
}

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
    /* as strlen is use, strcpy is as safe as strlcpy */
    strcpy(beg, str);
  } else {
    YE_TO_STRING(ent)->value = g_strdup_printf("%s%s", YE_TO_STRING(ent)->value,
					       str);
    g_free(YE_TO_STRING(ent)->origin);
    YE_TO_STRING(ent)->origin = NULL;
  }
  YE_TO_STRING(ent)->len = totalLength;
  return 0;
}

int yeStringAddCh(Entity *ent, char c)
{
  char buf[2] = {c, 0};

  return yeStringAdd(ent, buf);
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

int yeStringTruncate(Entity *str, uint32_t len)
{
  uint32_t l = yeLen(str);
  if (len >= l)
    return -1;
  l -= len;
  YE_TO_STRING(str)->value[l] = '\0';
  YE_TO_STRING(str)->len = l;
  return 0;
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
  int l = 0;

  if (unlikely(!str))
    return;
  for (; (*str == ' ' || *str == '\t'); ++str)
    ++l;
  if (!l)
    return;
  if (!YE_TO_STRING(s)->origin)
    YE_TO_STRING(s)->origin = YE_TO_STRING(s)->value;
  YE_TO_STRING(s)->len -= l;
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

static int tryMatchStr(const char *str, Entity *tok, int len)
{
  const char *cstr = tokStr(tok);
  int tokL = tokLen(tok);
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

Entity *yeCreateYirlFmtString(Entity *fmt, Entity *father, const char *name)
{
  Entity *ret = yeReCreateString("", father, name);
  const char *txt = yeGetString(fmt);

  if (unlikely(!txt))
	  return NULL;
  for (int i = 0; txt[i]; ++i) {
    if (txt[i] == '{') {
      char tmp;
      char *writable_txt = (char *)txt;
      char *entity_str;
      int j = i;

      for (; txt[j] && txt[j] != '}'; ++j);
      if (!txt[j])
	return NULL;
      tmp = txt[j];
      writable_txt[j] = 0;
      entity_str = yeToCStr(ygGet(&txt[i + 1]), 1, 0);
      yeStringAdd(ret, entity_str);
      g_free(entity_str);
      writable_txt[j] = tmp;
      i = j;
    } else {
      yeStringAddCh(ret, txt[i]);
    }
  }
  return ret;
}
