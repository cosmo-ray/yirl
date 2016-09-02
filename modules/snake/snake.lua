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


local Q_KEY = 113

function getLooseScreen(entity)
  return yeGet(yeGet(entity, "menus"), "LooseScreen")
end

function snakeMap(map)
   yeCreateInt(200000, map, "turn-length")
   yeCreateInt(20, map, "width")
   yeCreateInt(0, map, "nbPeanut")
   --yeCreateString( "rgba: 180 210 20 50", map, "background")

   local cases = yeCreateArray(map, "map")

   local i = 0
   while i < 20 * 20 do
     local tmp = yeCreateArray(cases, NULL)
     yeCreateInt(0, tmp, NULL)
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

function addBody(wid, map, pos)
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

function hitWall(wid, map, opos, npos, dir)
   local arg = yeCreateArray()

   yePushBack(arg, opos, "oldPos")
   yePushBack(arg, npos, "newPos")
   yeCreateInt(dir, arg, "dir")
   ywidCallSignal(wid, nil, arg, yeGetInt(yeGet(map, "hitWallIdx")))
   ywidCallSignal(wid, nil, arg, yeGetInt(yeGet(map, "endTurnIdx")))
   yeDestroy(arg)
end

function moveHeadInternal(wid, map, opos, npos)
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
      addBody(wid, map, opos)
      bodyLen = 0
      ywidCallSignal(wid, score, nil, yeGetInt(yeGet(map, "eatIdx")))
   end

   ywMapMove(map, opos, npos, headElem)

   if bodyLen == 0 then
      yeReplaceBack(head, npos, "pos")
      ywidCallSignal(wid, nil, nil, yeGetInt(yeGet(map, "endTurnIdx")))
      return
   end

   ywMapRemove(map, yeGet(body, 0), "bd");
   -- pop back tail body

   local i = 0
   while i < bodyLen do
      local curBody = yeGet(body, bodyLen - 1 - i)
      local tmpPos = ywPosCreate(curBody);
      ywPosSet(curBody, opos)
      ywPosSet(opos, tmpPos)
      yeDestroy(tmpPos)
      i = i + 1
   end

   local headBody = yeGet(body, bodyLen - 1)
   ywMapPushNbr(map, 4, headBody, "bd")
   yeReplaceBack(head, npos, "pos")
   ywidCallSignal(wid, nil, nil, yeGetInt(yeGet(map, "endTurnIdx")))
   -- push first body
end

function moveHead(wid, map)
   local mapElems = yeGet(map, "map")
   local lenMap = yeLen(mapElems)
   local width = yeGetInt(yeGet(map, "width"))
   local head = yeGet(map, "head")
   local headElem = yeGet(head, "elem")
   local dir = yeGet(head, "dir")

   local opos = yeGet(head, "pos")
   local npos = ywPosCreate(opos)
   
   ywPosAdd(npos, dir)
   -- check out of border
   if ywPosIsSameX(npos, -1) then
      hitWall(wid, map, opos, npos, 0)
   elseif ywPosIsSameX(npos, ywMapW(map)) then
      hitWall(wid, map, opos, npos, 1)
   elseif (ywPosIsSameY(npos, -1)) then
      hitWall(wid, map, opos, npos, 2)
   elseif (ywPosIsSameY(npos, ywMapH(map))) then
      hitWall(wid, map, opos, npos, 3)
   else
      moveHeadInternal(wid, map, opos, npos)
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


function snakeAction(wid, eve, arg)
   local map = ywidEntity(wid)

   addPeanut(map)
   moveHead(wid, map)
   while ywidEveIsEnd(eve) == false do
      if ywidEveType(eve) == YKEY_DOWN then
	 if ywidEveKey(eve) == Q_KEY then
	    ygCall(nil, "FinishGame")
	 elseif ywidEveKey(eve) == Y_UP_KEY
	    or ywidEveKey(eve) == Y_DOWN_KEY
	    or ywidEveKey(eve) == Y_RIGHT_KEY
	    or ywidEveKey(eve) == Y_LEFT_KEY
	 then
	    changeDir(map, eve)
	 end
      end
      eve = ywidNextEve(eve)
   end
   return YEVE_ACTION
end

function createSnake(entity)
   -- TODO: this functions: C/lua
   snakeMap(entity)
   yuiRandInit()
   yeCreateInt(ywidAddSignal(entity, "hitWall"),
	       entity, "hitWallIdx")
   yeCreateInt(ywidAddSignal(entity, "eat"),
	       entity, "eatIdx")
   yeCreateInt(ywidAddSignal(entity, "endTurn"),
	       entity, "endTurnIdx")
   local map = ywidNewWidget(entity, "map")

   ywidBind(map, "action", "snake:snakeAction")
   yeSetAt(ygGetMod("snake"), "score", 0)
   yeCreateInt(1, entity, "recreate-logic")
   yeCreateFunction("reset", entity)

   local bak = yeCreateArray()
   yeCopy(entity, bak)
   yePushBack(entity, bak, "initial-state");
   print("bak: ", yeToLuaString(bak))
   return map
end

function snakeDie(wid, useless1, useless2)
   ywidNext(yeGet(ywidEntity(wid), "next"))
end

function snakeWarp(wid, useless1, arg)
   local dir = yeGetInt(yeGet(arg, "dir"))
   local ent = ywidEntity(wid)
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
   moveHeadInternal(wid, ent, opos, npos)
end

function initSnake(entity)
   yeCreateFunction("snakeAction", entity, nil)
   yeCreateFunction("snakeDie", entity, nil)
   yeCreateFunction("snakeWarp", entity)

   local init = yeCreateArray(nil, nil)

   yeCreateString("snake", init, "name")
   yeCreateFunction("createSnake", init, "callback")
   ywidAddSubType(init)
   yeCreateInt(0, entity, "score");
end
