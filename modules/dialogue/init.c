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

/*
 * As of the 11/02/2023 I have 5 years of documentation of this module to do...
 * let's do it :)
 * in order to see dialogue callback, go to: void *init(int nbArg, void **args)
 * there is 2 way to use this modules:
 *	as dialogue-canvas: which will use dialogue-box module, and add stuff as canvas widget
 *	as dialogue: which will create a text-screen, with a menu widget
 *
 * the former is if you want a dialogue more like what you add in snes FF
 * the dialogue, is if you want a more VN-like dialogue
 * in order to choose betweeb then, you need to create either a
 * 'dialogue' or a 'dialogue-canvas' widget
 * (using <type> elemen of the widget description)
 *
 * widget description:
 *	txt-size - int - DIALOGUE ONLY: the size of the text-screen in comparaison to the menu, default, 70.
 *	is_looner_dialogue - 0/1 - BOTH: set to one, if dialogue elem, is the dialogue and an an array of dialogue
 */

#include <yirl/game.h>
#include <yirl/menu.h>
#include <yirl/canvas.h>
#include <yirl/text-screen.h>
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
	Entity *c_answer = drv->getCurAnswer(menu);

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
	else if (yeGetIntAt(c_answer, "hiden")) {
		ywMenuDown(menu);
	}
}

static Entity *getText_(Entity *box, Entity *e)
{
	Entity *condition;
	Entity *stext = yeGet(e, "switch-texts");
	Entity *rtext = yeGet(e, "rand-texts");
	Entity *txts = yeGet(e, "texts");

	if (stext) {
		Entity *cur_txt;
		int i = yeType(yeGet(stext, 0)) == YINT ? yeGetIntAt(stext, 0) :
			yeGetInt(ygGet(yeGetStringAt(stext, 0)));
	again_s:

		cur_txt = yeGet(stext, (i % (yeLen(stext) -1)) + 1);
		condition = yeGet(cur_txt, "condition");
		if (condition) {
			if (!dialogueCondition(box, condition, NULL)) {
				++i;
				goto again_s;
			}
		}

		ywidActions(box, cur_txt, NULL);
		if (yeGet(cur_txt, "text"))
			cur_txt = yeGet(cur_txt, "text");
		return cur_txt;
	}
	if (rtext) {
		Entity *cur_txt;

	again_r:
		cur_txt = yeGet(rtext, yuiRand() % yeLen(rtext));
		condition = yeGet(cur_txt, "condition");
		if (condition) {
			if (!dialogueCondition(box, condition, NULL))
				goto again_r;
			else
				return yeGet(cur_txt, "text");
		}

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
	return NULL;
}

static Entity *getText(Entity *box, Entity *e)
{
	if (yeType(e) == YSTRING ||
	    (yeType(e) == YARRAY && !yeGetKeyAt(e, 0))) {
		return e;
	}
	Entity *txt = yeGet(e, "text");

	if (yeType(txt) != YSTRING && yeType(txt) != YARRAY) {
		txt = getText_(box, e);
	}
	Entity *cnd_txt_append = yeGet(e, "conditional-texts-append");
	if (cnd_txt_append) {
		Entity *r;
		if (yeType(txt) == YARRAY) {
			if (yeGet(e, "_tmp_txt"))
				yeRemoveChildByStr(e, "_tmp_txt");
			r = yeCreateCopy(txt, e, "_tmp_txt");
		} else {
			r = yeReCreateArray(e, "_tmp_txt", NULL);
			yePushBack(r, txt, NULL);
		}
		YE_FOREACH(cnd_txt_append, cnd_txt) {
			Entity *condition = yeGet(cnd_txt, 0);
			if (!condition)
				continue;
			if (dialogueCondition(box, condition, NULL)) {
				yePushBack(r, yeGet(cnd_txt, 1), NULL);
			}
		}
		txt = r;
	}
	return txt;
}

static int printfTextAndAnswer_earlyret;

static void printfTextAndAnswer(Entity *wid, Entity *textScreen,
				Entity *menu, Entity *curent)
{
	struct menuDrv *drv = getMenuDrv(menu);
	Entity *dialogue;
	Entity *answers;
	Entity *txt;
	Entity *entries;
	int destroyable_answers = 0;
	Entity *gen_callback;

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

	if ((gen_callback = yeGet(dialogue, "gen-callback"))) {
		Entity *c = ygGet(yeGetString(gen_callback));

		yesCall(c, wid, dialogue);
	}

	if (yeGet(dialogue, "pre-action")) {
		printfTextAndAnswer_earlyret = 0;
		ywidAction(yeGet(dialogue, "pre-action"), wid, NULL);
		if (printfTextAndAnswer_earlyret)
			return;
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
		const char *bg = yeGetStringAt(dialogue, "speaker_background");
		const char *tbg = yeGetStringAt(dialogue, "text_background");

		if (bg) {
			yeReCreateString(bg, textScreen, "background");
		}

		if (tbg) {
			yeReCreateString(tbg, textScreen, "text_background");
		}

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

		if (yeType(answer) == YARRAY && !yeGet(answer, "text")) {
			yePushBack(answer, getText_(wid, answer), "text");
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
	yeCreateFunction("dialogueCanvasInit", ygGetManager("tcc"),
			 init, "callback");
	ywidAddSubType(init);
	/* registre functions */
	yeCreateFunction("dialogueChangeText", ygGetTccManager(),
			 mod, "change-text");
	yeCreateFunction("dialogueHide", ygGetTccManager(), mod, "hide");
	yeCreateFunction("dialogueGoto", ygGetTccManager(), mod, "goto");
	yeCreateFunction("dialogueGetMenu", ygGetTccManager(), mod, "get_menu");
	yeCreateFunction("dialogueSwap", ygGetTccManager(), mod, "swap");
	yeCreateFunction("dialogueConditionGoto", ygGetTccManager(),
			 mod, "condition_goto");
	yeCreateFunction("dialogueSwitch", ygGetTccManager(),
			 mod, "condition_switch");
	yeCreateFunction("dialogueGotoNext", ygGetTccManager(), mod, "gotoNext");
	yeCreateFunction("dialogueBlock", ygGetTccManager(), mod, "block");

	yeCreateFunctionSimple("newDialogue", ygGetTccManager(), mod);

	ygRegistreFunc(2, "showDialogueImage", "yShowDialogueImage");
	ygRegistreFunc(5, "newDialogueEntity", "yNewDialogueEntity");
	ygRegistreFunc(1, "dialogueGetMain", "yDialogueGetMain");
	ygRegistreFunc(1, "dialogueGetCAnswer", "yDialogueCurAnswer");
	/* return current actie dialogue in the dialogue wid, take full dialogue as param */
	ygRegistreFunc(1, "dialogueCurDialogue", "yDialogueCur");
	return NULL;
}

void *dialogueGetMenu(int nbArgs, void **args)
{
	Entity *e = args[0];
	struct mainDrv *drv = getMainDrv(e);

	return drv->getMenu(e);
}

void *dialogueCurDialogue(int nbArgs, void **args)
{
	Entity *e = args[0];
	int c = yeGetIntAt(e, "active_dialogue");

	return yeGet(yeGet(e, "dialogue"), c);
}

void *dialoguePostAction(int nbArgs, void **args)
{
	uint64_t ret_type = (long)args[0];
	Entity *e = args[1];

	if (!ywidInTree(e))
		return NOTHANDLE;
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
			} else if(ywidEveKey(eve) == '\n' ||
				  ywidEveKey(eve) == ' ') {
				Entity *answer = boxGetAnswer(box, current);

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
	Entity *answer = drv->getCurAnswer(args[0]);
	Entity *mwid = drv->getMain(args[0]);

	yeReCreateInt(1, answer, "hiden");
	refreshAnswer(drv->getMain(args[0]), args[0],
		      yeGet(mwid, "active_dialogue"));
	return 0;
}

void *dialogueConditionGoto(int nbArgs, void **args)
{
	Entity *main = getMenuDrv(args[0])->getMain(args[0]);
	struct mainDrv *drv = getMainDrv(main);

	for (int i = 2; i < nbArgs; i += 2) {
		if (i + 1 < nbArgs && yeCheckCondition(args[i])) {
			return dialogueGoto(3, (void *[]){args[0],
					NULL, args[i + 1]});
		} else if (i + 1 == nbArgs) {
			return dialogueGoto(3, (void *[]){args[0], NULL,
					args[i]});
		}
	}
	return 0;
}

void *dialogueSwitch(int nbArgs, void **args)
{
	Entity *main = getMenuDrv(args[0])->getMain(args[0]);
	struct mainDrv *drv = getMainDrv(main);

	int c_ret = ywidAction(args[2], main, NULL);

	if (c_ret + 3 >= nbArgs) {
		return 0;
	}
	return dialogueGoto(3, (void *[]){args[0], NULL, args[c_ret + 3]});
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
		yeCreateString("this persone don't want to talk to you",
			       block, "text");

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
	yeReplaceBack(drv->getTextWidget(main),
		      getText(args[0], args[2]), "text");
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

static void handler_looner_dialogue(Entity *wid)
{
	if (yeGetIntAt(wid, "is_looner_dialogue") == 1) {
		YE_NEW(array, dialogue);

		yePushBack(dialogue, yeGet(wid, "dialogue"), NULL);
		yeReplaceBack(wid, dialogue, "dialogue");
	}
}

void *dialogueInit(int nbArgs, void **args)
{
	Entity *main = args[0];
	Entity *entries = yeCreateArray(main, "entries");
	Entity *textScreen = yeCreateArray(entries, NULL);
	Entity *answers = yeCreateArray(entries, NULL);
	Entity *active_dialogue = defaultActiveDialogue(main);
	Entity *txt_size = yeGet(main, "txt-size");
	void *ret;

	handler_looner_dialogue(main);
	yeCreateData(&cntDialogueMainDrv, main, "drv");
	yeRemoveChildByStr(main, "action");
	yeCreateFunction("dialogueAction", ygGetTccManager(), main, "action");
	yeCreateFunction("dialoguePostAction", ygGetTccManager(),
			 main, "post-action");
	yeGetPush(main, textScreen, "text-speed");
	yeCreateString("text-screen", textScreen, "<type>");
	yeCreateInt(1, textScreen, "fmt");
	if (txt_size)
		yeReplaceBack(textScreen, txt_size, "size");
	else
		yeTryCreateInt(70, textScreen, "size");
	yePushBack(textScreen, yeGet(main, "speaker_background"),
		   "background");
	yeCreateString("menu", answers, "<type>");
	yeGetPush(main, answers, "next");
	yeCreateData(&cntDialogueMnDrv, answers, "drv");
	yePushBack(answers, yeGet(main, "answer_background"),
		   "background");

	yeTryCreateInt(1, main, "current");
	ret = ywidNewWidget(main, "container");
	yePrint(main);
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
		Entity *layers = NULL;
		char *img;
		int tx = 0, ty = 0;
		int p = 0;
		int layer_p = 0;

		if (yeType(image) == YSTRING) {
			img = yeGetString(image);
		} else {
			Entity *threshold = yeGet(image, "dst-threshold");

			img = yeGetStringAt(image, "src");
			tx = yeGetIntAt(threshold, 0);
			ty = yeGetIntAt(threshold, 1);
			p = yeGetIntAt(image, "reduce");
			layers = yeGet(image, "layers");
		}

		do {
			if (img)
				printf("%s\n", img);
			Entity *img_e = ywCanvasNewImg(
				canvas, 300 + tx, 300 + ty, img, NULL);
			if (!img_e)
				continue;
			if (p)
				ywCanvasPercentReduce(img_e, p);
			int r = yeGetIntAt(img_cnt, "image_rotate");
			if (r) {
				ywCanvasRotate(img_e, r);
			}
		} while ((img = yeGetStringAt(layers, layer_p++)) != NULL);
	}
}

void *dialogueSwap(int nbArgs, void **args)
{
	Entity *main = args[0];
	struct mainDrv *drv = getMainDrv(main);
	const char *dialogue = yeGetString(args[2]);
	Entity *active_dialogue = NULL;
	Entity *target_dialogue = ygGet(dialogue);
	Entity *new_d = yeGet(target_dialogue, "dialogue");

	if (target_dialogue && !new_d) {
		new_d = target_dialogue;
	} else if (!new_d) {
		DPRINT_ERR("FAILT TO SWAP TO DIALOGUE %s\n", dialogue);
	}

	yeRemoveChild(main, "dialogue");
	yePushBack(main, new_d, "dialogue");
	active_dialogue = defaultActiveDialogue(main);
	printfTextAndAnswer(main, drv->getTextWidget(main),
			    drv->getMenu(main), active_dialogue);
	printfTextAndAnswer_earlyret = 1;
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

	handler_looner_dialogue(main);
	yeCreateData(&boxDialogueMainDrv, main, "drv");
	yeRemoveChildByStr(main, "action");
	yeRemoveChildByStr(main, "post-action");
	yeCreateFunction("dialogueAction", ygGetTccManager(), main, "action");
	yeCreateFunction("dialoguePostAction", ygGetTccManager(),
			 main, "post-action");
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
