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
#include <yirl/contener.h>
#include <yirl/condition.h>

static void refreshAnswer(Entity *wid, Entity *textScreen,
			  Entity *menu, Entity *curent)
{
  Entity *answers = yeGetByStrFast(menu, "entries");

  YE_ARRAY_FOREACH(answers, answer) {
    Entity *condition = yeGetByStrFast(answer, "condition");

    if (condition) {
      yeReCreateInt(!yeCheckCondition(condition), answer, "hiden");
    }
  }
}

static Entity *getText(Entity *e)
{
  if (yeType(e) == YSTRING)
    return e;
  Entity *txt = yeGetByStrFast(e, "text");

  if (!txt) {
    Entity *condition;
    Entity *txts = yeGetByStr(e, "texts");

    YE_ARRAY_FOREACH(txts, cur_txt) {
      condition = yeGetByStrFast(cur_txt, "condition");

      if (condition && !yeCheckCondition(condition))
	continue;
      return yeGetByStrFast(cur_txt, "text");
    }
  }
}

static void printfTextAndAnswer(Entity *wid, Entity *textScreen,
				Entity *menu, Entity *curent)
{
  Entity *dialogue = yeGetByIdx(yeGetByStr(wid, "dialogue"), yeGetInt(curent));
  Entity *answers = yeGetByStr(dialogue, "answers");
  Entity *txt = getText(dialogue);
  Entity *entries;

  ywContenerUpdate(wid, textScreen);
  yeReCreateString(yeGetString(txt), textScreen, "text");
  entries = yeReCreateArray(menu, "entries", NULL);
  yeReplaceBack(menu, wid, "_main");
  YE_ARRAY_FOREACH(answers, answer) {
    Entity *condition = yeGetByStrFast(answer, "condition");

    if (condition)
      yeReCreateInt(!yeCheckCondition(condition), answer, "hiden");
    yePushBack(entries, answer, NULL);
  }
  ywMenuReBind(menu);
}

void *newDialogueEntity(int nbArgs, void **args)
{
  Entity *dialogue = args[0];
  Entity *father = args[1];
  Entity *name = args[2];
  Entity *speaker_background = args[3];
  Entity *answer_background = args[4];
  Entity *ret = yeCreateArray(father, name);

  yePushBack(ret, dialogue, "dialogue");
  yeCreateString("dialogue", ret, "<type>");

  yePushBack(ret, speaker_background, "speaker_background");
  yePushBack(ret, answer_background, "answer_background");
  return ret;
}

void *dialogueGetMain(int nbArg, void **args)
{
  return yeGet(args[0], "_main");
}

void *init(int nbArg, void **args)
{
  Entity *mod = args[0];
  Entity *init;
  Entity *map = yeCreateArray(mod, "game");

  init = yeCreateArray(NULL, NULL);
  yeCreateString("dialogue", init, "name");
  yeCreateFunction("dialogueInit", ygGetManager("tcc"), init, "callback");
  ywidAddSubType(init);
  /* registre functions */
  yeCreateFunction("dialogueChangeText", ygGetTccManager(), mod, "change-text");
  yeCreateFunction("dialogueHide", ygGetTccManager(), mod, "hide");
  yeCreateFunction("dialogueGoto", ygGetTccManager(), mod, "goto");

  yeCreateFunctionSimple("newDialogue", ygGetTccManager(), mod);

  ygRegistreFunc(5, "newDialogueEntity", "yNewDialogueEntity");
  ygRegistreFunc(1, "dialogueGetMain", "yDialogueGetMain");
  return NULL;
}

void *dialoguePostAction(int nbArgs, void **args)
{
  uint64_t ret_type = (long)args[0];
  Entity *e = args[1];

  if (ret_type == NOTHANDLE)
    return;
  refreshAnswer(e, ywCntGetEntry(e, 0), ywCntGetEntry(e, 1),
		yeGetByStrFast(e, "active_dialogue"));
}

void *dialogueAction(int nbArgs, void **args)
{
  Entity *wid = args[0];
  void *ret = (void *)NOTHANDLE;
  Entity *events = args[1];
  Entity *eve = events;

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

void *dialogueHide(int nbArgs, void **args)
{
  Entity *answers = yeGetByStrFast(args[0], "entries");
  Entity *answer = yeGetByIdx(answers,
			      yeGetInt(yeGetByStr(args[0], "_main.current")));

  yeReCreateInt(1, answer, "hiden");
  ywMenuDown(args[0]);
  return (void *)NOACTION;
}

void *dialogueGoto(int nbArgs, void **args)
{
  Entity *main = yeGetByStr(args[0], "_main");

  yeReplaceBack(main, args[3], "active_dialogue");
  printfTextAndAnswer(main, ywCntGetEntry(main, 0), args[0], args[3]);
  return (void *)NOACTION;
}

void *dialogueChangeText(int nbArgs, void **args)
{
  Entity *main = yeGetByStr(args[0], "_main");
  ywContenerUpdate(main, ywCntGetEntry(main, 0));
  yeReplaceBack(ywCntGetEntry(main, 0), getText(args[3]), "text");
  return (void *)NOACTION;
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
  yeCreateFunction("dialoguePostAction", ygGetTccManager(),
		     main, "post-action");
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
