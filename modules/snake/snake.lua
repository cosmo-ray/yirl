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


local action = yeCreateFunction("snakeAction", 3, nil, nil)
local die = yeCreateFunction("snakeDie", 3, nil, nil)
local warp = yeCreateFunction("snakeWarp", 3, nil, nil)

local Q_KEY = 113

function getLooseScreen(entity)
  return yeGet(yeGet(entity, "menus"), "LooseScreen")
end

function snakeMap(entity)
   local map = entity
   local i = 0;

   yeCreateInt(200000, map, "turn-length")
   yeCreateInt(20, map, "width")
   yeCreateInt(0, map, "nbPeanut")
   --yeCreateString( "rgba: 180 210 20 50", map, "background")

   local cases = yeCreateArray(map, "map")

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
   yeCreateInt(headPos, head, "pos")

   local body = yeCreateArray(map, "body")
   yeCreateInt(headPos, head, "pos")

   return map
end

function addBody(map, pos)
   local body = yeGet(map, "body")

   yeCreateInt(pos, body, nil)
   yeCreateInt(3, yeGet(yeGet(map, "map"), pos), "bd")
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

function hitWall(wid, map, oldPos, newPos, dir)
   local arg = yeCreateArray(nil, nil);

   yePushBack(arg, oldPos, "oldPos")
   yePushBack(arg, newPos, "newPos")
   yeCreateInt(dir, arg, "dir")
   ywidCallSignal(wid, nil, arg, yeGetInt(yeGet(map, "hitWallIdx")))
   ywidCallSignal(wid, nil, arg, yeGetInt(yeGet(map, "endTurnIdx")))
   yeDestroy(arg)
end

function moveHeadInternal(wid, map, opos, npos)
   local oldPos = ywMapIntFromPos(wid, opos);
   local newPos = ywMapIntFromPos(wid, npos);
   local mapElems = yeGet(map, "map")
   local destCase = ywMapGetCase(wid, npos)
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
      --add body :)
      yeSetInt(score, yeGetInt(score) + 1)
      addBody(map, oldPos)
      bodyLen = 0
   end

   ywMapMove(wid, opos, npos, headElem)
   yeSetInt(yeGet(head, "pos"), newPos)

   if bodyLen == 0 then
      ywidCallSignal(wid, nil, nil, yeGetInt(yeGet(map, "endTurnIdx")))
      return
   end

   ywMapRemove(wid, ywMapPosFromInt(wid, yeGetInt(yeGet(body, 0))), "bd");
   -- pop back tail body

   local i = 0
   while i < bodyLen do
      local curBody = yeGet(body, bodyLen - 1 - i)
      local oldPos2 = yeGetInt(curBody)

      yeSetInt(curBody, oldPos)
      oldPos = oldPos2
      i = i + 1
   end

   local headBody = yeGet(body, bodyLen - 1)
   yeCreateInt(4, yeGet(mapElems, yeGetInt(headBody)), "bd")
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
   local gc = yeCreateArray()

   local oldPos = yeGetInt(yeGet(head, "pos"))
   --local newPos = oldPos + yeGetInt(yeGet(dir, "y")) * yeGetInt(yeGet(map, "width")) + yeGetInt(yeGet(dir, "x"))
   local opos = ywMapPosFromInt(wid, oldPos, gc)
   local npos = ywMapPosFromInt(wid, oldPos, gc)
   
   ywPosAdd(npos, dir)
   -- check out of border
   if ywPosIsSameX(npos, -1) then
      hitWall(wid, map, opos, npos, 0)
   elseif ywPosIsSameX(npos, ywMapW(wid)) then
      hitWall(wid, map, opos, npos, 1)
   elseif (ywPosIsSameY(npos, -1)) then
      hitWall(wid, map, opos, npos, 2)
   elseif (ywPosIsSameY(npos, ywMapH(wid))) then
      hitWall(wid, map, opos, npos, 3)
   else
      moveHeadInternal(wid, map, opos, npos)
   end
   yeDestroy(gc)
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
	    ywidCallCallbackByStr("FinishGame", wid, eve, false)
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
   yeCreateInt(ywidAddSignal(entity, "endTurn"),
	       entity, "endTurnIdx")
   local map = ywidNewWidget(entity, "map")

   ywidBind(map, "action", "snakeAction")
   yeSetAt(ygGetMod("snake"), "score", 0)
   --ywidAddSignal(map, "loose");
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
      yeSetAt(npos, "y", ywMapH(wid) - 1)
   elseif (dir == 1) then
      yeSetAt(npos, "x", 0)
      yeSetAt(npos, "y", yeGetInt(yeGet(opos, "y")))
   elseif (dir == 0) then
      yeSetAt(npos, "x", ywMapW(wid) - 1)
      yeSetAt(npos, "y", yeGetInt(yeGet(opos, "y")))
   end
   moveHeadInternal(wid, ent, opos, npos)
end

function initSnake(entity)
   ywidAddCallback(ywidCreateCallback("snakeAction", action))
   ywidAddCallback(ywidCreateCallback("snakeDie", die))
   ywidAddCallback(ywidCreateCallback("snakeWarp", warp))
   local init = yeCreateArray(nil, nil)
   yeCreateString("snake", init, "name")
   yeCreateFunction("createSnake", init, "callback")
   ywidAddSubType(init)
   yeCreateInt(0, entity, "score");
end
