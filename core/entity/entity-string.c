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
    goto error;
  }

  for (;*cStr != ' ' && *cStr != '\t' && l < 256; ++l, ++cStr) {
    ret[l] = *cStr;
  }

  if (l == 256)
    goto error;

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
