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

#include <yirl/game.h>
#include <yirl/menu.h>

void *init(int nbArg, void **args)
{
  Entity *mod = args[0];
  Entity *init;
  Entity *map = yeCreateArray(mod, "game");

  yeCreateString("vapz", map, "<type>");
  yePushBack(map, yeGetByStr(mod, "resources.map"), "resources");

  init = yeCreateArray(NULL, NULL);
  yeCreateString("dialogue", init, "name");
  yeCreateFunction("dialogueInit", ygGetManager("tcc"), init, "callback");
  ywidAddSubType(init);
    /* registre functions */
  yeCreateFunction("dialogueChangeText", ygGetTccManager(), mod, "change-text");
  return NULL;
}

void *dialogueAction(int nbArgs, void **args)
{
  Entity *wid = args[0];
  void *ret = (void *)NOTHANDLE;
  YEvent *events = args[1];
  YEvent *eve = events;

  YEVE_FOREACH(eve, events) {
    if (ywidEveType(eve) == YKEY_DOWN) {
      switch (ywidEveKey(eve)) {
      case 'q':
	yFinishGame();
	ret = (void *)ACTION;
	break;
      default:
	break;

      }
    }
  }
  return ret;
}

static void printfTextAndAnswer(Entity *wid, Entity *textScreen,
				Entity *menu, Entity *curent)
{
  Entity *dialogue = yeGetByIdx(yeGetByStr(wid, "dialogue"), yeGetInt(curent));
  Entity *answers = yeGetByStr(dialogue, "answers");
  Entity *entries;

  yeReCreateString(yeGetString(yeGetByStr(dialogue, "text")),
		   textScreen, "text");
  entries = yeReCreateArray(menu, "entries", NULL);
  yeReplaceBack(menu, wid, "_main");
  YE_ARRAY_FOREACH(answers, answer) {
    yePushBack(entries, answer, NULL);
  }
  ywMenuReBind(menu);
}

void *dialogueChangeText(int nbArgs, void **args)
{
  Entity *cur = args[0];

  printf("dialogueChangeText: %d %p\n", nbArgs, args[0]);
  printf("main: %p\n", yeGetByStrFast(cur, "_main"));
}

void *dialogueInit(int nbArgs, void **args)
{
  Entity *main = args[0];
  Entity *entries = yeCreateArray(main, "entries");
  Entity *textScreen = yeCreateArray(entries, NULL);
  Entity *answers = yeCreateArray(entries, NULL);
  Entity *active_dialogue = yeTryCreateInt(0, main, "active_dialogue");
  void *ret;


  yeRemoveChildByStr(main, "action");
  ywidCreateFunction("dialogueAction", ygGetTccManager(), main, "action");
  yeCreateString("text-screen", textScreen, "<type>");
  yeTryCreateInt(70, textScreen, "size");
  yePushBack(textScreen, yeGetByStrFast(main, "speaker_background"),
	     "background");
  yeCreateString("menu", answers, "<type>");
  yePushBack(answers, yeGetByStrFast(main, "answer_background"),
	     "background");

  yeTryCreateInt(1, main, "current");
  ret = ywidNewWidget(main, "contener");
  printfTextAndAnswer(main, textScreen, answers, active_dialogue);
  return ret;
}
