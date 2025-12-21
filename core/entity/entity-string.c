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
#include	<inttypes.h>
#include	<unistd.h>
#include	<ctype.h>
#include	"entity.h"
#include	"utils.h"
#include	"stack.h"
#include	"script.h"
#include	"game.h"

static int tryMatchToks(const char *str, Entity *tokInfo, int len, int beg);

int yeStrLastCh(const Entity str[static 1])
{
	const char *val = YE_TO_C_STRING(str)->value;

	if (!val)
		return -1;
	int l = YE_TO_C_STRING(str)->len;
	return val[l - 1];
}

int yeStrChrIdx(Entity *str_ent, char c)
{
	const char *str = yeGetString(str_ent);
	const char *p;

	if (unlikely(!str))
		return -1;
	p = strchr(str, c);
	if (unlikely(!p))
		return -1;
	return p - str;
}

int yeStrIsRangeChr(Entity *str_ent, int beg, int end, char c)
{
	const char *str = yeGetString(str_ent);
	int l = yeLen(str_ent);

	if (unlikely(!str))
		return 0;
	if (beg > end || end > l)
		return 0;
	for (; beg < end; ++beg) {
		if (str[beg] != c)
			return 0;
	}
	return 1;
}

int yeStrIsRangeStr(Entity *str_ent, int beg, int end, const char *str)
{
	int32_t m = 0;

	if (!str)
		return 0;
	for (;*str; ++str) {
		m |= yeStrIsRangeChr(str_ent, beg, end, *str);
	}
	return !!m;
}

int yeStrDoesRangeContainChr(Entity *str_ent, int beg, int end, char c)
{
	const char *str = yeGetString(str_ent);
	int l = yeLen(str_ent);

	if (unlikely(!str))
		return 0;
	if (beg > end || end > l)
		return 0;
	for (; beg < end; ++beg) {
		if (str[beg] == c)
			return 1;
	}
	return 0;
}

int yeStrDoesRangeContainStr(Entity *str_ent, int beg, int end, const char *str)
{
	if (!str)
		return 0;
	for (;*str; ++str) {
		if (yeStrIsRangeChr(str_ent, beg, end, *str))
			return 1;
	}
	return 0;
}


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
	if (unlikely(!e))
		return NULL;
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

Entity *yeStringReplaceStrAt(Entity *haystack, const char *needle, size_t at)
{
	int l = strlen(needle);

	for (int i = 0; i < l; ++i) {
		char c = needle[i];
		if (unlikely(yeStringReplaceCharAt(haystack, c, at + i) < 0))
			return NULL;
	}
	return haystack;
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
	char *toFree = NULL;
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

	if (yeStringIsValueAllocated(ent))
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

Entity *yeStringAddChByEntity(Entity *ent, Entity *ch)
{
	return yeStringAddCh(ent, yeGetInt(ch));
}

Entity *yeStringAdd(Entity *ent, const char *str)
{
	int origLen;
	int totalLength;
	int strLen;

	if (unlikely(!ent || !str))
		return NULL;
	strLen = strlen(str);
	origLen = yeLen(ent);
	totalLength = origLen + strLen;
	if ((!YE_TO_STRING(ent)->origin) && yeStringIsValueAllocated(ent)) {
		YE_TO_STRING(ent)->value = realloc(YE_TO_STRING(ent)->value,
						   totalLength + 1);
		char *beg = YE_TO_STRING(ent)->value + origLen;
		/* as strlen is use, strcpy is as safe as strlcpy */
		strcpy(beg, str);
	} else {
		char *to_free = yeStringFreeable(ent);
		YE_TO_STRING(ent)->value =
			y_strdup_printf("%s%s", YE_TO_STRING(ent)->value,
					str);
		YE_TO_STRING(ent)->origin = NULL;
		free(to_free);
	}
	ygAssert(YE_TO_STRING(ent)->value);
	YE_TO_STRING(ent)->len = totalLength;
	return ent;
}

Entity *yeStringAddCh(Entity *ent, char c)
{
	char buf[2] = {c, 0};

	return yeStringAdd(ent, buf);
}

Entity *yeStringAddNl(Entity *ent, const char *str)
{
	yeStringAdd(ent, str);
	return yeStringAdd(ent, "\n");
}

Entity *yeStringAddInt(Entity *ent, int i)
{
	char *tmp = YE_TO_STRING(ent)->value;
	char *to_free = yeStringFreeable(ent);

	if (unlikely(!tmp))
		return NULL;
	YE_TO_STRING(ent)->value = y_strdup_printf("%s%d", tmp, i);
	YE_TO_STRING(ent)->len = strlen(YE_TO_STRING(ent)->value);
	YE_TO_STRING(ent)->origin = NULL;
	ygAssert(YE_TO_STRING(ent)->value);
	free(to_free);
	return ent;
}

Entity *yeStringAddDouble(Entity *ent, double d)
{
	char *tmp = YE_TO_STRING(ent)->value;
	char *to_free = yeStringFreeable(ent);

	if (unlikely(!tmp))
		return NULL;
	YE_TO_STRING(ent)->value = y_strdup_printf("%s%f", tmp, d);
	YE_TO_STRING(ent)->len = strlen(YE_TO_STRING(ent)->value);
	YE_TO_STRING(ent)->origin = NULL;
	ygAssert(YE_TO_STRING(ent)->value);
	free(to_free);
	return ent;
}

Entity *yeStringAddI64(Entity *ent, int64_t i)
{
	char *tmp = YE_TO_STRING(ent)->value;
	char *to_free = yeStringFreeable(ent);

	if (unlikely(!tmp))
		return NULL;
	YE_TO_STRING(ent)->value = y_strdup_printf("%s" PRIint64, tmp, i);
	YE_TO_STRING(ent)->len = strlen(YE_TO_STRING(ent)->value);
	YE_TO_STRING(ent)->origin = NULL;
	ygAssert(YE_TO_STRING(ent)->value);
	free(to_free);
	return ent;
}

Entity *yeStringAddLong(Entity *ent, long i)
{
	char *tmp = YE_TO_STRING(ent)->value;
	char *to_free = yeStringFreeable(ent);

	if (unlikely(!tmp))
		return NULL;
	YE_TO_STRING(ent)->value = y_strdup_printf("%s%ld", tmp, i);
	YE_TO_STRING(ent)->len = strlen(YE_TO_STRING(ent)->value);
	YE_TO_STRING(ent)->origin = NULL;
	ygAssert(YE_TO_STRING(ent)->value);
	free(to_free);
	return ent;
}

Entity *yeAddStrFromFd(Entity *e, int fd, int len)
{
	char *tmp = malloc(len + 1);

	if (!tmp || read(fd, tmp, len) < 0)
		goto exit;
	tmp[len] = 0;
	if (!yeStringAdd(e, tmp))
		goto exit;
exit:
	free(tmp);
	return e;
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

Entity *yeStringTruncate(Entity *str, uint32_t len)
{
	uint32_t l = yeLen(str);
	if (len > l)
		return NULL;
	l -= len;
	YE_TO_STRING(str)->value[l] = '\0';
	YE_TO_STRING(str)->len = l;
	return str;
}

Entity *yeStringShrink(Entity *str, uint32_t len)
{
	if (len >= yeLen(str))
		return NULL;
	if (!YE_TO_STRING(str)->origin)
		YE_TO_STRING(str)->origin = YE_TO_STRING(str)->value;
	YE_TO_STRING(str)->value += len;
	YE_TO_STRING(str)->len -= len;
	return str;
}

Entity *yeStringShrinkBlank(Entity *s)
{
	const char *str = yeGetString(s);
	int l = 0;

	if (unlikely(!str))
		return NULL;
	for (; (*str == ' ' || *str == '\t'); ++str)
		++l;
	if (!l)
		return NULL;
	if (!YE_TO_STRING(s)->origin)
		YE_TO_STRING(s)->origin = YE_TO_STRING(s)->value;
	YE_TO_STRING(s)->len -= l;
	YE_TO_STRING(s)->value = (char *)str;
	return s;
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

static int tryMatchSepReap(Entity *tokInfo, const char *str,
			   Entity *tok, int len, int beg)
{
	if (!beg)
		return 0;
	if (tryMatchReapeater(str, tok, len)) {
		const char *cstr = tokStr(tok);
		int tl = strlen(cstr);
		int m;
		YE_NEW(string, stre, cstr);

		m = tryMatchToks(str + tl, tokInfo, len - tl, 1);
		if (m == YTOK_WORD)
			return 0;
		yeSetStringAt(tok, 3, yeGetString(stre));
		/* if (m > YTOK_WORD) { */
		/* 	int tt = yeGetIntAt(yeGet(tokInfo, m), 0); */
		/* 	if (tt == YTOK_TYPE_SEPARATE_STR || */
		/* 	    tt == YTOK_TYPE_SEP_REAP_STR) */
		/* 		return 0; */
		/* } */
		return 1;
	}
	return 0;
}

static int tryMatchSep(Entity *tokInfo, const char *str, Entity *tok,
		       int len, int beg)
{
	if (!beg)
		return 0;
	if (tryMatchStr(str, tok, len)) {
		const char *cstr = tokStr(tok);
		int tl = strlen(cstr);
		int m;

		m = tryMatchToks(str + tl, tokInfo, len - tl, 1);
		if (m == YTOK_WORD)
			return 0;
		/* if (m > YTOK_WORD) { */
		/* 	int tt = yeGetIntAt(yeGet(tokInfo, m), 0); */
		/* 	if (tt == YTOK_TYPE_SEPARATE_STR || */
		/* 	    tt == YTOK_TYPE_SEP_REAP_STR) */
		/* 		return 0; */
		/* } */
		return 1;
	}
	return 0;
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
				if (tryMatchSepReap(tokInfo, str, curTok,
						    len, beg))
					return i;
				break;
			case YTOK_TYPE_SEPARATE_STR:
				if (tryMatchSep(tokInfo, str, curTok, len, beg))
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
			free(entity_str);
			writable_txt[j] = tmp;
			i = j;
		} else {
			yeStringAddCh(ret, txt[i]);
		}
	}
	return ret;
}

int yeSplitInto(Entity *into, const char *src, const char *separator)
{
	if (!src || !into)
		return 0;
	if (!separator) {
		yeCreateString(src, into, NULL);
		return 1;
	}

	const char *tmp;
	int i = 0;

	while ((tmp = strstr(src, separator)) != NULL) {
		yeCreateNString(src, tmp - src, into, NULL);
		src = tmp + 1;
		++i;
	}

	yeCreateString(src, into, NULL);
	return ++i;
}
