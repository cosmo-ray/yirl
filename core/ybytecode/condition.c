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

#include <yirl/condition.h>
#include <yirl/ybytecode.h>
#include <yirl/ybytecode-script.h>
#include <yirl/entity-script.h>
#include <yirl/game.h>

static inline int pushComVal(Entity *val, uint64_t *instructions, int *idx)
{
	int i = *idx;

	if (yeType(val) == YINT) {
		*idx = i + 2;
		instructions[i] = YB_CREATE_INT;
		instructions[i + 1] = yeGetInt(val);
	} else if (yeType(val) == YSTRING) {
		*idx = i + 2;
		instructions[i] = YB_YG_GET_PUSH;
		instructions[i + 1] = (size_t)yeGetString(val);
	}
	return 0;
}

static inline int pushStrComVal(Entity *val, uint64_t *instructions, int *idx)
{
	int i = *idx;

	*idx = i + 2;
	if (ygGet(yeGetString(val))) {
		instructions[i] = YB_YG_GET_PUSH;
		instructions[i + 1] = (size_t)yeGetString(val);
	} else {
		instructions[i] = YB_CREATE_STRING;
		instructions[i + 1] = (size_t)yeGetString(val);
	}
	return 0;
}

int conditionCall(Entity *condition)
{
	Entity *f = ygGet(yeGetStringAt(condition, 1));
	int nb = yeLen(condition) - 2;
	union ycall_arg args[nb < 0 ? 0 : nb];
	int types[4] = {YS_ENTITY};

	for (int i = 0; i < nb; ++i)
		args[i].e = yeGet(condition, i + 2);

	return (intptr_t)yesCallInt(f, nb, args, types);
}

int yeCheckCondition(Entity *condition)
{
	if (yeType(condition) == YSTRING) {
		Entity *e = ygGet(yeGetString(condition));
		if (yeIsNum(e))
			return !!yeGetInt(e);
		return !!e;
	}

	Entity *actionEnt = yeGetByIdx(condition, 0);
	int i = 0;

	if (unlikely(!condition))
		return 0;
	if (unlikely(!actionEnt))
		return 0;
	/* compille stuff \0/ */
	/* if (yeType(actionEnt) == YSTRING || */
	/*     yeType(yeGet(actionEnt, 1)) != YFUNCTION) { */
	/* in theory I should keepthe compiled code, */
	/* problem is that it can be free in some unlucky maner */
	if (1) {
		Entity *func;
		Entity *estr = yeType(actionEnt) == YSTRING ? actionEnt :
			yeGet(actionEnt, 0);
		const char *action = yeGetString(estr);

		if (!yeStrCmp(estr, "call")) {
			return conditionCall(condition);
		} else if (!yeStrCmp(estr, "not")) {
			return !yeCheckCondition(yeGet(condition, 1));
		} else if (!yeStrCmp(estr, "contain_string")) {
			Entity *array = ygGet(yeGetStringAt(condition, 1));

			if (yeType(array) != YARRAY)
				return 0;
			YE_FOREACH(array, s) {
				if (!yeStrCmp(s, yeGetStringAt(condition, 2))) {
					return 1;
				}
			}
			return 0;
		} else if (!yeStrCmp(estr, "and")) {
			return yeCheckCondition(yeGet(condition, 1)) &&
				yeCheckCondition(yeGet(condition, 2));
		} else if (!yeStrCmp(estr, "or")) {
			return yeCheckCondition(yeGet(condition, 1)) ||
				yeCheckCondition(yeGet(condition, 2));
		} else if (!yeStrCmp(estr, "exist")) {
			return !!ygGet(yeGetStringAt(condition, 1));
		} else if (!yeStrCmp(estr, "!exist")) {
			return !ygGet(yeGetStringAt(condition, 1));
		}
		int len = yeLen(estr);
		Entity *data = yeCreateDataExt(NULL, NULL, NULL,
					       YE_DATA_USE_OWN_METADATA);
		uint64_t *instructions = yeGetData(data);
/*     int16_t instMaxLen = */
/*       yeMetadataSize(DataEntity) / sizeof(uint64_t); */

		instructions[0] = 0; // not compilled yet
		if (yeType(actionEnt) == YSTRING) {
			actionEnt = yeConvert(actionEnt, YARRAY);
			action = yeGetStringAt(actionEnt, 0);
		}

		if (!action)
			return 0;

		switch(len) {
		case 1:
		case 2:
			i = 1;
			if (len == 2 && action[0] == 'S' &&
			    (action[1] == '=' || action[1] == '!')) {
				pushStrComVal(yeGet(condition, 1),
					      instructions, &i);
				pushStrComVal(yeGet(condition, 2),
					      instructions, &i);
				instructions[i] = action[1] == '=' ?
					YB_STR_EQUAL : YB_STR_NEQUAL;
				goto comparaisons_instructions;
			}
			pushComVal(yeGet(condition, 1), instructions, &i);
			pushComVal(yeGet(condition, 2), instructions, &i);
			if (action[0] == '>') {
				instructions[i] = YB_SUP;
			} else if (action[0] == '<') {
				instructions[i] = YB_INF;
			} else if (action[0] == '=') {
				instructions[i] = YB_EQUAL;
			} else if (action[0] == '&') {
				instructions[i] = YB_CHECK_B_AND;
			} else if (len == 2 && action[0] == '!' &&
				   action[1] == '=') {
				instructions[i] = YB_NOT_EQUAL;
			} else {
				return 0;
			}
		comparaisons_instructions:
			instructions[i + 1] = 0;
			instructions[i + 2] = 1;
			instructions[i + 3] = i + 5;
			instructions[i + 4] = YB_RETURN0;
			instructions[i + 5] = YB_RETURN_IVAL;
			instructions[i + 6] = 1;
			instructions[i + 7] = YB_END_FUNC;
			break;
		default:
			return 0;
		}
		func = ysYbytecodeCreateFunc(data, NULL, NULL);
		yePushAt(actionEnt, func, 1);
		yeDestroy(data);
		yeDestroy(func);
	}
	/* add args */
	int_ptr_t ret = (int_ptr_t)yesCall(yeGet(actionEnt, 1));
	return ret;
}
