--
--Copyright (C) 2016 Matthias Gatto
--
--This program is free software: you can redistribute it and/or modify
--it under the terms of the GNU Lesser General Public License as published by
--the Free Software Foundation, either version 3 of the License, or
--(at your option) any later version.
--
--This program is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--GNU General Public License for more details.
--
--You should have received a copy of the GNU Lesser General Public License
--along with this program.  If not, see <http://www.gnu.org/licenses/>.
--

function getLooseScreen(entity)
  return yeGet(yeGet(entity, "menus"), "LooseScreen")
end

function snakeMap(map)
   ywMapSetSmootMovement(map, 1);
   yeCreateInt(200000, map, "turn-length")
   yeCreateInt(20, map, "width")
   yeCreateInt(0, map, "nbPeanut")
   local cases = yeCreateArray(map, "map")

   local i = 0
   while i < 20 * 20 do
     local tmp = yeCreateArray(cases)
      yeCreateInt(0, tmp)
      i = i + 1
   end

   -- pos head :)
   local headPos = 20 * 10 + 10
   local tmp = yeCreateInt(1, yeGet(cases, headPos), NULL)
   local head = yeCreateArray(map, "head")

   ywPosCreate(0, 1, head, "dir")
   yePushBack(head, tmp, "elem")
   ywPosCreate(10, 10, head, "pos")

   local body = yeCreateArray(map, "body")

   return map
end

function addBody(map, pos)
   local body = yeGet(map, "body")

   local tmp = ywPosCreate(pos, body)
   ywMapPushNbr(map, 3, pos, "bd");
end

function addPeanut(map)
   local nbPeanut = yeGet(map, "nbPeanut")
   local lenMap = yeLen(yeGet(map, "map"))

   if yeGetInt(nbPeanut) < 1 then
      local lenMap = yeLen(yeGet(map, "map"))
      local dest = yuiRand() % lenMap
      local case = yeGet(yeGet(map, "map"), dest)

      if yeLen(case) > 1 then
	 -- we try another pos
	 return addPeanut(map)
      end
      yeCreateInt(2, case, NULL)
      yeSetInt(nbPeanut, yeGetInt(nbPeanut) + 1)
   end
end

function rmPeanut(map, case)
   local nbPeanut = yeGet(map, "nbPeanut")

   yePopBack(case)
   yeSetInt(nbPeanut, yeGetInt(nbPeanut) - 1)
end

function hitWall(map, opos, npos, dir)
   local arg = yeCreateArray()

   yePushBack(arg, opos, "oldPos")
   yePushBack(arg, npos, "newPos")
   yeCreateInt(dir, arg, "dir")
   yesCall(ygGet(yeGetString(yeGet(map, "hitWall"))), map, arg)
   yesCall(ygGet(yeGetString(yeGet(map, "endTurn"))), map, arg)
   yeDestroy(arg)
end

function moveHeadInternal(map, opos, npos)
   local mapElems = yeGet(map, "map")
   local destCase = ywMapGetCase(map, npos)
   local body = yeGet(map, "body")
   local bodyLen = yeLen(body)
   local head = yeGet(map, "head")
   local headElem = yeGet(head, "elem")
   local dir = yeGet(head, "dir")

   if yeLen(destCase) > 1 then
      if (yeGetInt(yeGet(destCase, 1)) ~= 2) then
	 ywidNext(yeGet(map, "next"))
      end
      local score = yeGet(ygGetMod("snake"), "score")
      rmPeanut(map, destCase)
      --use yeOps instead :)
      yeSetInt(score, yeGetInt(score) + 1)
      addBody(map, opos)
      bodyLen = 0
      yesCall(yeGet(map, "eat"), score, nil)
   end

   if (yuiAbs(ywPosX(npos) - ywPosX(opos)) > 1) then
      ywMapMove(map, opos, npos, headElem);
   elseif (yuiAbs(ywPosY(npos) - ywPosY(opos)) > 1) then
      ywMapMove(map, opos, npos, headElem);
   else
      ywMapSmootMove(map, opos, npos, headElem);
   end

   if bodyLen == 0 then
      yeReplaceBack(head, npos, "pos")
      yesCall(yeGet(map, "endTurn"))
      return
   end

   local i = 0
   while i < bodyLen do
      local curBodyPos = yeGet(body, bodyLen - 1 - i)
      local elem = ywMapGetEntityById(map, curBodyPos, 4);
      local tmpPos = ywPosCreate(curBodyPos);

      if yLovePtrToNumber(elem) == 0 then
	 elem = ywMapGetEntityById(map, curBodyPos, 3)
	 yeSetInt(elem, 4)
      end
      if (yuiAbs(ywPosX(curBodyPos) - ywPosX(opos)) > 1) then
	 ywMapMove(map, curBodyPos, opos, elem);
      elseif (yuiAbs(ywPosY(curBodyPos) - ywPosY(opos)) > 1) then
	 ywMapMove(map, curBodyPos, opos, elem);
      else
	 ywMapSmootMove(map, curBodyPos, opos, elem);
      end
      ywPosSet(curBodyPos, opos)
      ywPosSet(opos, tmpPos)
      yeDestroy(tmpPos)
      i = i + 1
   end

   yeReplaceBack(head, npos, "pos")
   yesCall(yeGet(map, "endTurn"), nil, nil)
end

function moveHead(map)
   local mapElems = yeGet(map, "map")
   local lenMap = yeLen(mapElems)
   local width = yeGetInt(yeGet(map, "width"))
   local head = yeGet(map, "head")
   local headElem = yeGet(head, "elem")
   local dir = yeGet(head, "dir")

   local opos = yeGet(head, "pos")
   local npos = ywPosCreate(opos)

   local headPos = 20 * 10 + 10

   ywPosAdd(npos, dir)
   -- check out of border
   if ywPosIsSameX(npos, -1) then
      hitWall(map, opos, npos, 0)
   elseif ywPosIsSameX(npos, ywMapW(map)) then
      hitWall(map, opos, npos, 1)
   elseif (ywPosIsSameY(npos, -1)) then
      hitWall(map, opos, npos, 2)
   elseif (ywPosIsSameY(npos, ywMapH(map))) then
      hitWall(map, opos, npos, 3)
   else
      moveHeadInternal(map, opos, npos)
   end
   yeDestroy(npos);
end

function changeDir(map, eve)
   local dir = yeGet(yeGet(map, "head"), "dir")

   if ywidEveKey(eve) == Y_UP_KEY then
      if ywPosIsSame(dir, 0, 1) then return end
      ywPosSet(dir, 0, -1)
   elseif ywidEveKey(eve) == Y_DOWN_KEY then
      if ywPosIsSame(dir, 0, -1) then return end
      ywPosSet(dir, 0, 1)
   elseif ywidEveKey(eve) == Y_RIGHT_KEY then
      if ywPosIsSame(dir, -1, 0) then return end
      ywPosSet(dir, 1, 0)
   elseif ywidEveKey(eve) == Y_LEFT_KEY then
      if ywPosIsSame(dir, 1, 0) then return end
      ywPosSet(dir, -1, 0)
   end
end

function snakeAction(map, eve, arg)
   local hasChange = false

   while ywidEveIsEnd(eve) == false do
      if ywidEveType(eve) == YKEY_DOWN then
	 if ywidEveKey(eve) == Y_Q_KEY then
	    ygCall(nil, "FinishGame")
	 elseif (ywidEveKey(eve) == Y_UP_KEY
		    or ywidEveKey(eve) == Y_DOWN_KEY
		    or ywidEveKey(eve) == Y_RIGHT_KEY
		 or ywidEveKey(eve) == Y_LEFT_KEY) and hasChange == false then
	    changeDir(map, eve)
	    hasChange = true
	 elseif ywidEveKey(eve) == Y_L_KEY then
	    ywidNext(yeGet(map, "next"))
	 end
      end
      eve = ywidNextEve(eve)
   end
   addPeanut(map)
   moveHead(map)
   return YEVE_ACTION
end

function createSnake(entity)
   -- TODO: this functions: C/lua
   snakeMap(entity)
   yuiRandInit()
   yeSetAt(ygGetMod("snake"), "score", 0)
   yeCreateFunction("reset", entity)
   yeCreateString("snake:snakeAction", entity, "action")

   local map = ywidNewWidget(entity, "map")

   local bak = yeCreateArray()
   yeCopy(entity, bak)
   yePushBack(entity, bak, "initial-state");
   yeDestroy(bak);
   yeCreateInt(1, entity, "recreate-logic")
   return map
end

function snakeDie(wid, useless1, useless2)
   ywidNext(yeGet(wid, "next"))
end

function snakeWarp(ent, arg)
   local dir = yeGetInt(yeGet(arg, "dir"))
   local mapLen = yeLen(yeGet(ent, "map"))
   local mapW = yeGetInt(yeGet(ent, "width"))
   local npos = yeGet(arg, "newPos")
   local opos = yeGet(arg, "oldPos")

   if (dir == 3) then
      yeSetAt(npos, "y", 0)
   elseif (dir == 2) then
      yeSetAt(npos, "y", ywMapH(ent) - 1)
   elseif (dir == 1) then
      yeSetAt(npos, "x", 0)
      yeSetAt(npos, "y", yeGetInt(yeGet(opos, "y")))
   elseif (dir == 0) then
      yeSetAt(npos, "x", ywMapW(ent) - 1)
      yeSetAt(npos, "y", yeGetInt(yeGet(opos, "y")))
   end
   moveHeadInternal(ent, opos, npos)
end

function reset(entity)
   local bak = yeGet(entity, "initial-state")
   yeIncrRef(bak)
   yeCopy(bak, entity)
   yePushBack(entity, bak, "initial-state")
   local map = ywidNewWidget(entity, "map")
   yeCreateInt(1, entity, "recreate-logic")

   local headPos = 20 * 10 + 10
   local tmp = yeGet(yeGet(entity, "map"), headPos)
   yeDestroy(bak)
   return map
end

function initSnake(entity)
   yeCreateFunction("snakeAction", entity)
   yeCreateFunction("snakeDie", entity)
   yeCreateFunction("snakeWarp", entity)

   local init = yeCreateArray(nil, nil)

   yeCreateString("snake", init, "name")
   yeCreateFunction("createSnake", init, "callback")
   ywidAddSubType(init)
   yeCreateInt(0, entity, "score");
end
