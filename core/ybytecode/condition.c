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
    /* return yeGetInt(val); */
  } else if (yeType(val) == YSTRING) {
    *idx = i + 2;
    instructions[i] = YB_YG_GET_PUSH_INT;
    instructions[i + 1] = (size_t)yeGetString(val);
  }
  return 0;
}

int yeCheckCondition(Entity *condition)
{
  Entity *actionEnt = yeGetByIdx(condition, 0);
  const char *action = yeGetString(actionEnt);
  int i = 0;

  if (unlikely(!condition))
    return 0;
  if (unlikely(!actionEnt))
    return 0;
  /* compille stuff \0/ */
  if (yeType(actionEnt) == YSTRING) {
    int len = yeLen(actionEnt);
    Entity *data = yeCreateDataExt(NULL, NULL, NULL,
				   YE_DATA_USE_OWN_METADATA);
    uint64_t *instructions = yeGetData(data);
    /* int16_t instMaxLen = */
    /*   yeMetadataSize(DataEntity) / sizeof(uint64_t); */

    instructions[0] = 0; // not compilled yet
    actionEnt = yeConvert(actionEnt, YARRAY);

    if (!action)
      return 0;

    instructions[1] = YB_CREATE_INT;
    instructions[2] = 1;
    i = 3;
    pushComVal(yeGet(condition, 1), instructions, &i);
    pushComVal(yeGet(condition, 2), instructions, &i);
    instructions[i] = YB_JMP;
    instructions[i + 1] = i + 2;
    switch(len) {
    case 1:
      if (action[0] == '>') {
	instructions[i + 2] = YB_SUP;
      } else if (action[0] == '<') {
	instructions[i + 2] = YB_INF;
      } else if (action[0] == '=') {
	instructions[i + 2] = YB_EQUAL;
      } else {
	return 0;
      }
      instructions[i + 3] = 1;
      instructions[i + 4] = 2;
      instructions[i + 5] = i + 9;
      instructions[i + 6] = YB_SET_INT;
      instructions[i + 7] = 0;
      instructions[i + 8] = 0;
      instructions[i + 9] = YB_RETURN;
      instructions[i + 10] = 0;
      instructions[i + 11] = YB_END_FUNC;
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
