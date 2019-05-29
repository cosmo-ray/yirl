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

#include "entity.h"
#include "tests.h"

void stringsTests(void)
{
  yeInitMem();

  Entity *str = yeCreateString(NULL, NULL, NULL);
  int l;
  Entity *tokInfo = yeTokInfoCreate(NULL, NULL);
  enum toks {
    YTOK_STR_BASE,
    EXCLAMATION_MARK,
    RETURN,
    SPACES,
  };
  yeCreateString("!", tokInfo, NULL);
  yeCreateString("\n", tokInfo, NULL);
  yeTokInfoAddRepeated(" \t", tokInfo);

  g_assert(str);
  g_assert(yeLen(str) == 0);
  g_assert(yeStringAddNl(str, NULL));
  g_assert(!yeStrCmp(str, "\n"));
  g_assert(yeCountCharacters(str, '\n', -1) == 1);
  g_assert(yeStringAdd(str, "ma lite !\n"));
  g_assert(!yeStrCmp(str, "\nma lite !\n"));
  g_assert(yeStringAddNl(str, "ma lite !"));
  g_assert(!yeStrCmp(str, "\nma lite !\nma lite !\n"));
  g_assert(yeCountCharacters(str, '\n', -1) == 3);
  g_assert(yeCountCharacters(str, '\n', 9) == 5);
  g_assert(yeCountCharacters(str, '\n', 10) == 3);
  g_assert(!yeStringShrink(str, 1));
  g_assert(!yeStrCmp(str, "ma lite !\nma lite !\n"));
  g_assert(!yeStringShrink(str, 3));
  g_assert(!yeStrCmp(str, "lite !\nma lite !\n"));
  g_assert(yeStringAddNl(str, "    2 le retour"));
  g_assert(!yeStrCmp(str, "lite !\nma lite !\n    2 le retour\n"));
  g_assert(yuiStrEqual(yeStringNextWord(str, &l, 0), "lite"));
  g_assert(l == 4);
  g_assert(!yeStrCmp(str, "lite !\nma lite !\n    2 le retour\n"));
  g_assert(!yeStringShrink(str, l));
  g_assert(yeStringNextWord(str, &l, 0) == NULL);
  g_assert(l == 0);
  g_assert(!yeStrCmp(str, " !\nma lite !\n    2 le retour\n"));
  g_assert(yuiStrEqual(yeStringNextWord(str, &l, 1), "!\nma"));
  g_assert(l == 4);
  g_assert(!yeStrCmp(str, "!\nma lite !\n    2 le retour\n"));
  int tok = yeStringNextTok(str, tokInfo);
  g_assert(tok == EXCLAMATION_MARK);
  tok = yeStringNextTok(str, tokInfo);
  g_assert(tok == RETURN);
  tok = yeStringNextTok(str, tokInfo);
  g_assert(tok == YTOK_WORD);
  tok = yeStringNextTok(str, tokInfo);
  g_assert(tok == SPACES);
  tok = yeStringNextTok(str, tokInfo);
  g_assert(tok == YTOK_WORD);
  g_assert(yeTokLen(tokInfo, tok) == 4);
  tok = yeStringNextTok(str, tokInfo);
  g_assert(tok == SPACES);
  g_assert(yeTokLen(tokInfo, tok) == 1);
  g_assert(yeStringNextTok(str, tokInfo) == EXCLAMATION_MARK);
  g_assert(yeStringNextTok(str, tokInfo) == RETURN);
  tok = yeStringNextTok(str, tokInfo);
  g_assert(tok == SPACES);
  g_assert(yeTokLen(tokInfo, tok) == 4);
  yeDestroy(tokInfo);
  g_assert(yeStringReplace(str, "lite", "track") == 1);
  g_assert(!yeStrCmp(str, "!\nma track !\n    2 le retour\n"));
  g_assert(yeStringReplace(str, "\n", "---") == 3);
  g_assert(!yeStrCmp(str, "!---ma track !---    2 le retour---"));
  yeDestroy(str);
  yeEnd();
}
