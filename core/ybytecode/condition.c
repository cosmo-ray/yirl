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
		instructions[i] = YB_YG_GET_PUSH_INT;
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

	args[0].e = nb > 0 ? yeGet(condition, 2) : NULL;
	args[1].e = nb > 1 ? yeGet(condition, 3) : NULL;
	args[2].e = nb > 2 ? yeGet(condition, 4) : NULL;
	args[3].e = nb > 3 ? yeGet(condition, 5) : NULL;

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
	const char *action = yeGetString(actionEnt);
	int i = 0;

	if (unlikely(!condition))
		return 0;
	if (unlikely(!actionEnt))
		return 0;
	/* compille stuff \0/ */
	if (yeType(actionEnt) == YSTRING) {
		if (!yeStrCmp(actionEnt, "call")) {
			return conditionCall(condition);
		} else if (!yeStrCmp(actionEnt, "not")) {
			return !yeCheckCondition(yeGet(condition, 1));
		} else if (!yeStrCmp(actionEnt, "and")) {
			return yeCheckCondition(yeGet(condition, 1)) &&
				yeCheckCondition(yeGet(condition, 2));
		} else if (!yeStrCmp(actionEnt, "or")) {
			return yeCheckCondition(yeGet(condition, 1)) ||
				yeCheckCondition(yeGet(condition, 2));
		} else if (!yeStrCmp(actionEnt, "exist")) {
			return !!ygGet(yeGetStringAt(condition, 1));
		}
		int len = yeLen(actionEnt);
		Entity *data = yeCreateDataExt(NULL, NULL, NULL,
					       YE_DATA_USE_OWN_METADATA);
		uint64_t *instructions = yeGetData(data);
/*     int16_t instMaxLen = */
/*       yeMetadataSize(DataEntity) / sizeof(uint64_t); */

		instructions[0] = 0; // not compilled yet
		actionEnt = yeConvert(actionEnt, YARRAY);

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
			} else if (len == 2 && action[0] == '!' &&
				   action[1] == '=') {
				instructions[i] = YB_NOT_EQUAL;
			}
			else {
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
		ysYbytecodeCreateFunc(data, actionEnt, NULL);
		yeDestroy(data);
	}
	/* add args */
	return (int_ptr_t)yesCall(yeGet(actionEnt, 1));
}
