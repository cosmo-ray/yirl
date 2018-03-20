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
#include <yirl/entity-script.h>
#include <yirl/container.h>
#include <yirl/condition.h>

void *yFinishGame();

struct mainDrv {
  Entity *(*getTextWidget)(Entity *);
  Entity *(*getMenu)(Entity *);
};

struct menuDrv {
  Entity *(*getAnswer)(Entity *, int);
  Entity *(*getAnswers)(Entity *);
  Entity *(*getMain)(Entity *);
};

static Entity *getMain(Entity *mn);
static Entity *getAnswers(Entity *mn);
static Entity *getAnswer(Entity *mn, int idx);

static Entity *getTextWidget(Entity *main);
static Entity *getMenu(Entity *main);

struct mainDrv cntDialogueMainDrv = {
  getTextWidget, getMenu
};

struct menuDrv cntDialogueMnDrv = {
  getAnswer, getAnswers, getMain
};

int boxMainPos;

static Entity *boxGetbox(Entity *main)
{
  return yeGet(main, "box");
}

static Entity *boxGetTX(Entity *main)
{
  return yesCall(ygGet("DialogueBox.getDialogue"), boxGetbox(main));
}

struct mainDrv boxDialogueMainDrv = {
  boxGetTX, boxGetbox
};


static Entity *boxGetAnswer(Entity *box, int idx)
{
  return yesCall(ygGet("DialogueBox.getAnswer"), box, idx);
}

static Entity *boxGetAnswers(Entity *box)
{
  return yesCall(ygGet("DialogueBox.getAnswers"), box);
}

static Entity *boxGetMain(Entity *box)
{
  return yeGet(box, boxMainPos);
}

struct menuDrv boxDialogueMnDrv = {
  boxGetAnswer, boxGetAnswers, boxGetMain
};

static struct menuDrv *getMenuDrv(Entity *menu)
{
  return yeGetDataAt(menu, "drv");
}

static struct mainDrv *getMainDrv(Entity *main)
{
  return yeGetDataAt(main, "drv");
}

static Entity *getTextWidget(Entity *main)
{
  return ywCntGetEntry(main, 0);
}

static Entity *getMenu(Entity *main)
{
  return ywCntGetEntry(main, 1);
}

static Entity *getAnswers(Entity *mn)
{
  return yeGet(mn, "entries");
}

static void refreshAnswer(Entity *wid, Entity *menu, Entity *curent)
{
  struct menuDrv *drv = getMenuDrv(menu);
  Entity *answers = drv->getAnswers(menu);

  YE_ARRAY_FOREACH(answers, answer) {
    Entity *condition = yeGet(answer, "condition");

    if (condition) {
      yeReCreateInt(!yeCheckCondition(condition), answer, "hiden");
    }
  }
  if (drv == &boxDialogueMnDrv)
    yesCall(ygGet("DialogueBox.reload"), wid, menu);
}

static Entity *getText(Entity *e)
{
  if (yeType(e) == YSTRING)
    return e;
  Entity *txt = yeGet(e, "text");

  if (yeType(txt) == YSTRING)
    return txt;
  if (!txt) {
    Entity *condition;
    Entity *txts = yeGet(e, "texts");

    YE_ARRAY_FOREACH(txts, cur_txt) {
      condition = yeGet(cur_txt, "condition");

      if (condition && !yeCheckCondition(condition))
	continue;
      return yeGet(cur_txt, "text");
    }
  }
  return NULL;
}

static void printfTextAndAnswer(Entity *wid, Entity *textScreen,
				Entity *menu, Entity *curent)
{
  struct menuDrv *drv = getMenuDrv(menu);
  Entity *dialogue = yeGet(yeGet(wid, "dialogue"), yeGetInt(curent));
  Entity *answers = yeGet(dialogue, "answers");
  Entity *txt = getText(dialogue);
  Entity *entries;

  if (drv == &cntDialogueMnDrv) {
    ywContainerUpdate(wid, textScreen);
    entries = yeReCreateArray(menu, "entries", NULL);
  } else {
    yesCall(ygGet("DialogueBox.pushAnswers"), menu, answers);
  }
  yeReCreateString(yeGetString(txt), textScreen, "text");

  YE_ARRAY_FOREACH(answers, answer) {
    Entity *condition = yeGet(answer, "condition");

    if (condition)
      yeReCreateInt(!yeCheckCondition(condition), answer, "hiden");
    if (drv == &cntDialogueMnDrv)
      yePushBack(entries, answer, NULL);
  }
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

Entity *getMain(Entity *w)
{
  return ywCntWidgetFather(w);
}

void *init(int nbArg, void **args)
{
  Entity *mod = args[0];
  Entity *init;
  Entity *map = yeCreateArray(mod, "game");

  /* menu dialogue */
  init = yeCreateArray(NULL, NULL);
  yeCreateString("dialogue", init, "name");
  yeCreateFunction("dialogueInit", ygGetManager("tcc"), init, "callback");
  ywidAddSubType(init);
  /* canvas dialogue */
  init = yeCreateArray(NULL, NULL);
  yeCreateString("dialogue-canvas", init, "name");
  yeCreateFunction("dialogueCanvasInit", ygGetManager("tcc"), init, "callback");
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
  struct mainDrv *drv = getMainDrv(e);

  if (ret_type == NOTHANDLE)
    return;
  refreshAnswer(e, drv->getMenu(e), yeGet(e, "active_dialogue"));
}

void *dialogueAction(int nbArgs, void **args)
{
  struct mainDrv *drv = getMainDrv(args[0]);
  Entity *eve;

  if (drv == &boxDialogueMainDrv) {
    Entity *box = boxGetbox(args[0]);
    int current = yesCall(ygGet("DialogueBox.pos"), box);
    int hasMove = 0;

    YEVE_FOREACH(eve, args[1]) {
      if (ywidEveType(eve) == YKEY_DOWN) {
	if (ywidEveKey(eve) == 'q') {
	  yFinishGame();
	  return ACTION;
	} else if(ywidEveKey(eve) == Y_DOWN_KEY) {
	  current += 1;
	  hasMove = 1;
	} else if(ywidEveKey(eve) == Y_UP_KEY) {
	  current -= 1;
	  hasMove = 1;
	}
      }
    }
    if (hasMove) {
      yesCall(ygGet("DialogueBox.moveAnswer"), box, current);
      current = yesCall(ygGet("DialogueBox.pos"), box);
      return ACTION;
    }
  }
  // canvas movement need to be handle here :p
  return NOTHANDLE;
}

static Entity *getAnswer(Entity *mn, int idx)
{
  Entity *answers = yeGet(mn, "entries");
  Entity *answer = yeGetByIdx(answers, idx);

  return answer;
}

void *dialogueHide(int nbArgs, void **args)
{
  struct menuDrv *drv = getMenuDrv(args[0]);
  Entity *answer = drv->getAnswer(args[0], yeGetIntAt(drv->getMain(args[0]),
						      "current"));

  yeReCreateInt(1, answer, "hiden");
  ywMenuDown(args[0]);
  return (void *)NOACTION;
}

void *dialogueGoto(int nbArgs, void **args)
{
  Entity *main = getMenuDrv(args[0])->getMain(args[0]);
  struct mainDrv *drv = getMainDrv(main);

  yeReplaceBack(main, args[3], "active_dialogue");
  printfTextAndAnswer(main, drv->getTextWidget(main), args[0], args[3]);
  return (void *)NOACTION;
}

void *dialogueChangeText(int nbArgs, void **args)
{
  Entity *main = getMenuDrv(args[0])->getMain(args[0]);
  struct mainDrv *drv = getMainDrv(main);

  ywContainerUpdate(main, getTextWidget(main));
  yeReplaceBack(drv->getTextWidget(main), getText(args[3]), "text");
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

  yeCreateData(&cntDialogueMainDrv, main, "drv");
  yeRemoveChildByStr(main, "action");
  yeCreateFunction("dialogueAction", ygGetTccManager(), main, "action");
  yeCreateFunction("dialoguePostAction", ygGetTccManager(),
		     main, "post-action");
  yeCreateString("text-screen", textScreen, "<type>");
  yeTryCreateInt(70, textScreen, "size");
  yePushBack(textScreen, yeGet(main, "speaker_background"),
	     "background");
  yeCreateString("menu", answers, "<type>");
  yeCreateData(&cntDialogueMnDrv, answers, "drv");
  yePushBack(answers, yeGet(main, "answer_background"),
	     "background");

  yeTryCreateInt(1, main, "current");
  ret = ywidNewWidget(main, "container");
  printfTextAndAnswer(main, textScreen, answers, active_dialogue);
  return ret;
}

void *dialogueCanvasInit(int nbArgs, void **args)
{
  Entity *main = args[0];
  Entity *active_dialogue = yeTryCreateInt(0, main, "active_dialogue");
  YWidgetState *ret;
  Entity *box;
  Entity *dialogue;
  Entity *data;

  if (!ygGetMod("DialogueBox")) {
    DPRINT_ERR("DialogueBox module need to be load");
    return NULL;
  }
  boxMainPos = yeGetInt(ygGet("DialogueBox.privateDataSize"));

  yeCreateData(&boxDialogueMainDrv, main, "drv");
  yeRemoveChildByStr(main, "action");
  yeRemoveChildByStr(main, "post-action");
  yeCreateFunction("dialogueAction", ygGetTccManager(), main, "action");
  yeCreateFunction("dialoguePostAction", ygGetTccManager(),
		     main, "post-action");
  yeTryCreateInt(1, main, "current");
  yeCreateArray(main, "objs");
  ret = ywidNewWidget(main, "canvas");
  dialogue = yeGet(main, "dialogue");
  box = yesCall(ygGet("DialogueBox.new_empty"), main, 10, 10, main, "box");
  data = yeCreateData(&boxDialogueMnDrv, NULL, NULL);
  yeAttach(box, data, boxMainPos + 1, "drv", 0);
  yeDestroy(data);
  yePushAt(box, main, boxMainPos);
  printfTextAndAnswer(main, boxGetTX(main), box, active_dialogue);
  yesCall(ygGet("DialogueBox.reload"), main, box);
  return ret;
}
