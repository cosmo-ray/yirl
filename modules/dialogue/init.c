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
#include <yirl/canvas.h>
#include <yirl/text-screen.h>
#include <yirl/entity-script.h>
#include <yirl/container.h>
#include <yirl/condition.h>

void *yFinishGame();

static int current_answer;

struct mainDrv {
	Entity *(*getTextWidget)(Entity *);
	Entity *(*getMenu)(Entity *);
};

struct menuDrv {
	Entity *(*getAnswer)(Entity *, int);
	Entity *(*getCurAnswer)(Entity *);
	void (*setCurAnswer)(Entity *, Entity *);
	Entity *(*getAnswers)(Entity *);
	Entity *(*getMain)(Entity *);
};

static Entity *getMain(Entity *mn);
static Entity *getAnswers(Entity *mn);
static Entity *getAnswer(Entity *mn, int idx);

static Entity *getMnCurAnswer(Entity *mn)
{
	return ywMenuGetCurrentEntry(mn);
}

static void setMnCurAnswer(Entity *mn, Entity *new_cur)
{
	return ywMenuSetCurrentEntry(mn, new_cur);
}

static Entity *getTextWidget(Entity *main);
static Entity *getMenu(Entity *main);

void *dialogueGotoNext(int nbArgs, void **args);


struct mainDrv cntDialogueMainDrv = {
	getTextWidget, getMenu
};

struct menuDrv cntDialogueMnDrv = {
	getAnswer, getMnCurAnswer, setMnCurAnswer, getAnswers, getMain
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

static Entity *boxGetCurAnswer(Entity *box)
{
	return yesCall(ygGet("DialogueBox.getCurAnswer"), box);
}

static Entity *boxGetAnswers(Entity *box)
{
	return yesCall(ygGet("DialogueBox.getAnswers"), box);
}

static void boxSetCurAnswer(Entity *box, Entity *nc)
{
	Entity *an;
	Entity *answers = boxGetAnswers(box);

	for (int i = 0; i < yeLen(answers); ++i) {
		if ((an = boxGetAnswer(box, i)) == NULL)
			continue;
		if (nc == an) {
			yesCall(ygGet("DialogueBox.moveAnswer"), box, i);
			return;
		}
	}
}

static Entity *boxGetMain(Entity *box)
{
	return yeGet(box, boxMainPos);
}

struct menuDrv boxDialogueMnDrv = {
	boxGetAnswer, boxGetCurAnswer, boxSetCurAnswer,
	boxGetAnswers, boxGetMain
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

static size_t dialogueLen(Entity *main)
{
  return yeLen(yeGet(main, "dialogue"));
}

static inline void findReplace__box(Entity *box, Entity *c)
{
	if (yeType(c) != YARRAY)
		return;
	YE_ARRAY_FOREACH_ENTRY(c, ae) {
		if (ae->name)
			return;
		Entity *cur = ae->entity;

		if (yeType(cur) == YSTRING && !yeStrCmp(cur, "__box")) {
			yeReplace(c, cur, box);
		} else if (yeType(cur) == YARRAY) {
			findReplace__box(box, cur);
		}
	}
}

static int dialogueCondition(Entity *box, Entity *c, Entity *answer)
{
	int ret;
	Entity *ca = NULL;

	if (answer) {
		ca = getMenuDrv(box)->getCurAnswer(box);
		getMenuDrv(box)->setCurAnswer(box, answer);
		findReplace__box(box, c);
	}
	yeReCreateInt(1, box, "is_dialogue_condition");
	ret = yeCheckCondition(c);
	if (ca) {
		getMenuDrv(box)->setCurAnswer(box, ca);
	}
	yeRemoveChild(box, "is_dialogue_condition");
	return ret;
}

static void refreshAnswer(Entity *wid, Entity *menu, Entity *curent)
{
	struct menuDrv *drv = getMenuDrv(menu);
	Entity *answers = drv->getAnswers(menu);

	YE_ARRAY_FOREACH(answers, answer) {
		Entity *condition = yeType(answer) == YARRAY ?
			yeGet(answer, "condition") : NULL;

		if (condition) {
			yeReCreateInt(!dialogueCondition(menu, condition, answer),
				      answer, "hiden");
		}
	}
	if (drv == &boxDialogueMnDrv)
		yesCall(ygGet("DialogueBox.reload"), wid, menu);
}

static Entity *getText(Entity *box, Entity *e)
{
	if (yeType(e) == YSTRING ||
	    (yeType(e) == YARRAY && !yeGetKeyAt(e, 0))) {
		return e;
	}
	Entity *txt = yeGet(e, "text");

	if (yeType(txt) == YSTRING || yeType(txt) == YARRAY)
		return txt;
	if (!txt) {
		Entity *condition;
		Entity *rtext = yeGet(e, "rand-texts");
		Entity *txts = yeGet(e, "texts");

		if (rtext) {
			Entity *cur_txt;

			cur_txt = yeGet(rtext, yuiRand() % yeLen(rtext));
			return cur_txt;
		}

		YE_ARRAY_FOREACH(txts, cur_txt) {
			condition =  yeType(cur_txt) == YARRAY ?
				yeGet(cur_txt, "condition") : NULL;

			if (condition && !dialogueCondition(box, condition, NULL))
				continue;
			ywidActions(box, cur_txt, NULL);
			return yeGet(cur_txt, "text");
		}
	}
	return NULL;
}

static void printfTextAndAnswer(Entity *wid, Entity *textScreen,
				Entity *menu, Entity *curent)
{
	struct menuDrv *drv = getMenuDrv(menu);
	Entity *dialogue;
	Entity *answers;
	Entity *txt;
	Entity *entries;
	int destroyable_answers = 0;

	if (yeGetIntAt(wid, "isBlock") == 1) {
		dialogue = yeGet(wid, "block");
	} else {
		if (yeIsNum(curent)) {
			dialogue = yeGet(yeGet(wid, "dialogue"),
					 yeGetInt(curent));
		} else {
			dialogue = yeGet(yeGet(wid, "dialogue"),
					 yeGetString(curent));
		}
	}
	txt = getText(menu, dialogue);
	answers = yeGet(dialogue, "answers");

	if (!answers) {
		Entity *answer = yeGet(dialogue, "answer");

		if (dialogue == txt) {
			answers = yeCreateArray(NULL, NULL);
			destroyable_answers = 1;
		} else {
			answers = yeCreateArray(dialogue, "answers");
		}
		if (answer) {
			yePushBack(answers, answer, NULL);
		} else {
			if (yeGet(dialogue, "action")) {
				Entity *answer = yeCreateArray(answers, NULL);

				yeCreateString("(continue)", answer, "text");
				yeGetPush(dialogue, answer, "action");
			} else if (yeGet(dialogue, "actions")) {
				Entity *answer = yeCreateArray(answers, NULL);

				yeCreateString("(continue)", answer, "text");
				yeGetPush(dialogue, answer, "actions");
			} else {
				yeCreateString("(continue)", answers, NULL);
			}
		}
	}
	if (drv == &cntDialogueMnDrv) {
		ywContainerUpdate(wid, textScreen);
		entries = yeReCreateArray(menu, "entries", NULL);
	} else {
		yesCall(ygGet("DialogueBox.pushAnswers"), menu, answers);
		yesCall(ygGet("DialogueBox.moveAnswer"), menu, 0);
	}
	if (yeType(txt) == YARRAY)
		yeReplaceBack(textScreen, txt, "text");
	else
		yeReCreateString(yeGetString(txt), textScreen, "text");
	yeReCreateInt(1, menu, "isDialogue");

	YE_ARRAY_FOREACH(answers, answer) {
		Entity *condition = NULL;
		int ar_t = yeType(answer);

		if (ar_t == YARRAY)
			condition = yeGet(answer, "condition");

		if (condition) {
			yeReCreateInt(!dialogueCondition(menu, condition, answer),
				      answer, "hiden");
		}

		if (drv == &cntDialogueMnDrv) {
			if (yeType(answer) == YSTRING) {
				Entity *strAnswer = answer;
				answer = yeCreateArray(entries, NULL);
				yePushBack(answer, strAnswer, "text");
				yeCreateString("Dialogue.gotoNext", answer,
					       "action");
			} else {
				yePushBack(entries, answer, NULL);
			}
		}
	}
	if (destroyable_answers)
		yeDestroy(answers);
}

void *dialogueGetCAnswer(int nbArgs, void **args)
{
	return getMenuDrv(args[0])->getCurAnswer(args[0]);
}

void *dialogueGetMain(int nbArgs, void **args)
{
	return getMenuDrv(args[0])->getMain(args[0]);
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
  yeCreateFunction("dialogueGotoNext", ygGetTccManager(), mod, "gotoNext");
  yeCreateFunction("dialogueBlock", ygGetTccManager(), mod, "block");

  yeCreateFunctionSimple("newDialogue", ygGetTccManager(), mod);

  ygRegistreFunc(2, "showDialogueImage", "yShowDialogueImage");
  ygRegistreFunc(5, "newDialogueEntity", "yNewDialogueEntity");
  ygRegistreFunc(1, "dialogueGetMain", "yDialogueGetMain");
  ygRegistreFunc(1, "dialogueGetCAnswer", "yDialogueCurAnswer");
  return NULL;
}

void *dialoguePostAction(int nbArgs, void **args)
{
	uint64_t ret_type = (long)args[0];
	Entity *e = args[1];
	struct mainDrv *drv = getMainDrv(e);

	refreshAnswer(e, drv->getMenu(e), yeGet(e, "active_dialogue"));
	return ((void *)ret_type == ACTION ? ACTION: NOTHANDLE);
}

void *dialogueAction(int nbArgs, void **args)
{
	struct mainDrv *drv = getMainDrv(args[0]);
	Entity *eve;

	if (drv != &boxDialogueMainDrv)
			return NOTHANDLE;

	Entity *box = boxGetbox(args[0]);
	int current = (int64_t)yesCall(ygGet("DialogueBox.pos"), box);
	int hasMove = 0;

	YEVE_FOREACH(eve, args[1]) {
		if (ywidEveType(eve) == YKEY_DOWN) {
			if (ywidEveKey(eve) == 'q') {
				yFinishGame();
				return (void *)ACTION;
			} else if(ywidEveKey(eve) == Y_DOWN_KEY) {
				current += 1;
				hasMove = 1;
			} else if(ywidEveKey(eve) == Y_UP_KEY) {
				current -= 1;
				hasMove = 1;
			} else if(ywidEveKey(eve) == '\n') {
				Entity *answer = boxGetAnswer(box,
							      current);

				if (!yeGet(answer, "action") &&
				    !yeGet(answer, "actions")) {
					/* so... that's a stupide way to call a tcc entity function */
					return dialogueGotoNext(2, (void *[])
								{box, answer});
				} else {
					return (void *)ywidActions(box, answer,
								   eve);
				}
			}
		}
	}
	if (hasMove) {
		yesCall(ygGet("DialogueBox.moveAnswer"), box, current);
		current = (int64_t)yesCall(ygGet("DialogueBox.pos"), box);
		return (void *)ACTION;
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
  if (drv == &cntDialogueMnDrv)
    ywMenuDown(args[0]);
  return 0;
}

void *dialogueGoto(int nbArgs, void **args)
{
  Entity *main = getMenuDrv(args[0])->getMain(args[0]);
  struct mainDrv *drv = getMainDrv(main);
  int idx;

  if (yeIsNum(args[2]))
	  idx = yeGetInt(args[2]);
  else
	  idx = yeArrayIdx(yeGet(main, "dialogue"), yeGetString(args[2]));

  yeReCreateInt(idx, main, "active_dialogue");
  printfTextAndAnswer(main, drv->getTextWidget(main), args[0], args[2]);
  return 0;
}

void *dialogueGotoNext(int nbArgs, void **args)
{
  Entity *box = args[0];
  Entity *main = getMenuDrv(box)->getMain(args[0]);
  struct mainDrv *drv = getMainDrv(main);
  Entity *active_dialogue = yeGet(main, "active_dialogue");
  Entity *condition;
  int pass;

  if (drv == &cntDialogueMainDrv) {
    ywtextScreenResetTimer(ywCntGetEntry(main, 0));
  }

  do {
    yeAddInt(active_dialogue, 1);
    Entity *cur_dialogue = yeGet(yeGet(main, "dialogue"),
				 yeGetInt(active_dialogue));
    condition = yeType(cur_dialogue) == YARRAY ?
	    yeGet(cur_dialogue, "condition") : NULL;
    if (condition)
	    pass = dialogueCondition(box, condition, NULL);
    else
	    pass = 1;
  } while (!pass);

  if (yeGetInt(active_dialogue) >= dialogueLen(main)) {
    return (void *)ywidAction(yeGet(main, "endAction"), box, NULL);
  }
  printfTextAndAnswer(main, drv->getTextWidget(main), box, active_dialogue);
  return 0;
}

void *dialogueBlock(int nbArgs, void **args)
{
  Entity *main = getMenuDrv(args[0])->getMain(args[0]);
  struct mainDrv *drv = getMainDrv(main);
  Entity *block = yeGet(main, "block");
  Entity *answers = yeGet(block, "answers");
  Entity *answer;
  char *block_dialogue = NULL;
  char *block_action = NULL;
  char *block_answer = NULL;

  if (nbArgs > 2)
    block_action = args[2];
  if (nbArgs > 3)
    block_dialogue = args[3];
  if (nbArgs > 4)
    block_answer = args[4];

  if (!block) {
    if (!block_action)
      return NULL;
    block = yeCreateArray(main, "block");
    answers = yeCreateArray(block, "answers");
    yeCreateArray(answers, NULL);
   }
  answer = yeGet(answers, 0);
  if (block_dialogue)
    yeReCreateString(yeGetString(block_dialogue), block, "text");
  else if (!yeGet(block, "text"))
    yeCreateString("this persone don't want to talk to you", block, "text");

  if (block_answer)
    yeReCreateString(yeGetString(block_answer), answer, "text");
  else if (!yeGet(answer, "text"))
    yeCreateString("end dialogue", answer, "text");

  if (block_action)
    yeReCreateString(yeGetString(block_action), answer, "action");

  yeReCreateInt(1, main, "isBlock");
  return 0;
}


void *dialogueChangeText(int nbArgs, void **args)
{
  Entity *main = getMenuDrv(args[0])->getMain(args[0]);
  struct mainDrv *drv = getMainDrv(main);

  ywContainerUpdate(main, getTextWidget(main));
  yeReplaceBack(drv->getTextWidget(main), getText(args[0], args[2]), "text");
  return 0;
}

Entity *defaultActiveDialogue(Entity *main)
{
  int i = 0;
  Entity *cur_dialogue;
  Entity *condition;

 again:
  cur_dialogue = yeGet(yeGet(main, "dialogue"), i);
  condition = yeType(cur_dialogue) == YARRAY ?
	  yeGet(cur_dialogue, "condition") :
	  NULL;
  if (condition && !yeCheckCondition(condition)) {
    ++i;
    goto again;
  }
  if (yeGetIntAt(main, "keep_dialogue") == 1)
    return yeTryCreateInt(i, main, "active_dialogue");
  return yeReCreateInt(i, main, "active_dialogue");
}

void *dialogueInit(int nbArgs, void **args)
{
  Entity *main = args[0];
  Entity *entries = yeCreateArray(main, "entries");
  Entity *textScreen = yeCreateArray(entries, NULL);
  Entity *answers = yeCreateArray(entries, NULL);
  Entity *active_dialogue = defaultActiveDialogue(main);
  void *ret;

  yeCreateData(&cntDialogueMainDrv, main, "drv");
  yeRemoveChildByStr(main, "action");
  yeCreateFunction("dialogueAction", ygGetTccManager(), main, "action");
  yeCreateFunction("dialoguePostAction", ygGetTccManager(),
		     main, "post-action");
  yeGetPush(main, textScreen, "text-speed");
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

void *dialogueDestroy(int nbArgs, void **args)
{
	yeRemoveChild(yeGet(args[0], "box"), args[0]);
	yeRemoveChild(yeGet(args[0], "name-box"), args[0]);
}

void *showDialogueImage(int n, void **args) {
	Entity *canvas = args[0], *img_cnt = args[1];
	Entity *image = yeGet(img_cnt, "image");

	if (image) {
		char *img;
		int tx = 0, ty = 0;
		int p = 0;

		if (yeType(image) == YSTRING) {
			img = yeGetString(image);
		} else {
			Entity *threshold = yeGet(image, "dst-threshold");

			img = yeGetStringAt(image, "src");
			tx = yeGetIntAt(threshold, 0);
			ty = yeGetIntAt(threshold, 1);
			p = yeGetIntAt(image, "reduce");
		}

		Entity *img_e = ywCanvasNewImg(canvas, 300 + tx, 300 + ty,
					       img, NULL);
		if (p)
			ywCanvasPercentReduce(img_e, p);
		int r = yeGetIntAt(img_cnt, "image_rotate");
		if (r) {
			ywCanvasRotate(img_e, r);
		}
	}
}

void *dialogueCanvasInit(int nbArgs, void **args)
{
  Entity *main = args[0];
  Entity *active_dialogue = defaultActiveDialogue(main);
  YWidgetState *ret;
  Entity *box;
  Entity *dialogue;
  Entity *data;
  Entity *name = yeGet(main, "name");
  int y = 10;

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
  if (name) {
    yesCall(ygGet("DialogueBox.new_text"), main, 10, y,
	    yeGetString(name), main, "name-box");
    y += 30;
  }
  box = yesCall(ygGet("DialogueBox.new_empty"), main, 10, y, main, "box");
  data = yeCreateData(&boxDialogueMnDrv, NULL, NULL);
  yeAttach(box, data, boxMainPos + 1, "drv", 0);
  yeDestroy(data);
  yePushAt(box, main, boxMainPos);
  yeCreateFunction("dialogueDestroy", ygGetTccManager(), main, "destroy");
  printfTextAndAnswer(main, boxGetTX(main), box, active_dialogue);
  showDialogueImage(2, (void *[]){main, main});

  yesCall(ygGet("DialogueBox.reload"), main, box);
  return ret;
}
