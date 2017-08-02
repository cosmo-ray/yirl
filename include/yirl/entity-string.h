/*
**Copyright (C) 2017 Matthias Gatto
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

/* !!!!!! ATTENTION !!!!!!
 * DO NOT INCLUDE THIS FILE, include entity.h instead
 */

/**
 * @brief Add @str to the string entity @Ent
 */
int yeStringAdd(Entity *ent, const char *str);

/**
 * @brief same as yeStringAdd, but add a new line
 */
int yeStringAddNl(Entity *ent, const char *str);

/**
 * @brief Count the number of @carac in @ent
 *
 * @param ent a string entity
 * @param lineLimit hard to explain, use -1, or go read the code...
 * @returnreturn the number a @carac in @ent
 * @examples yeCountCharacters(str, '\n', -1), will return the number of
 *	     lines in str
 */
int yeCountCharacters(Entity *ent, char carac, int lineLimit);

int yeAddStrFromFd(Entity *e, int fd, int len);
int yeStringAddInt(Entity *ent, int i);
int yeStringAddLong(Entity *ent, long i);


/**
 * @brief store next word in a temporary
 * @param len if not NULL store len of next word
 * @param shrinkBlank if 1 call yeStringShrinkBlank
 * @return return next word or NULL
 */
const char *yeStringNextWord(Entity *str, int *len, int shrinkBlank);

#define YTOK_STR_BASE					\
  YTOK_ERR = -1, YTOK_END = 0, YTOK_WORD = 1

typedef enum {
  YTOK_TYPE_STR,
  YTOK_TYPE_REAPETED_STR
} YTokType;

/**
 * Add a token that is not a word (like '{'), but some repeted caracter
 * examples, a list of spaces, so a repeted tok with the string " " in param
 * would match ' ', but '       ' too, as the same token
 */
static inline Entity *yeTokInfoAddRepeated(const char *str, Entity *father)
{
  Entity *ret = yeCreateArray(father, NULL);

  yeCreateInt(YTOK_TYPE_REAPETED_STR, ret, NULL);
  yeCreateString(str, ret, NULL);
  yeCreateInt(-1, ret, NULL);
  return ret;
}

static inline Entity *yeTokInfoCreate(Entity *father, const char *name)
{
  Entity *ret = yeCreateArray(father, name);

  yeCreateInt(0, ret, NULL);
  yeCreateString(NULL, ret, NULL);
  return ret;
}

int yeTokLen(Entity *tokInfo, int tokIdx);

int yeStringNextTok(Entity *str, Entity *tokInfo);

/**
 * @brief remove @len caracters at the begin of @str
 *
 * @param str the string to shrink
 * @param len the number of carac to remove
 */
int yeStringShrink(Entity *str, uint32_t len);

/**
 * @brief remove all blank and tab up front
 */
void yeStringShrinkBlank(Entity *str);
