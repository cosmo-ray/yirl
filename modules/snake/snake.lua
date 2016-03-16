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

   ywMapCreatePos(0, 1, head, "dir")
   yePushBack(head, tmp, "elem")
   yeCreateInt(headPos, head, "pos")

   local body = yeCreateArray(map, "body")
   yeCreateInt(headPos, head, "pos")

   return map
end

function addBody(map, oldPos)
   local body = yeGet(map, "body")

   yeCreateInt(oldPos, body, nil)
   yeCreateInt(3, yeGet(yeGet(map, "map"), oldPos), nil)
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

   yeCreateInt(oldPos, arg, "oldPos")
   yeCreateInt(newPos, arg, "newPos")
   yeCreateInt(dir, arg, "dir")
   ywidCallSignal(wid, nil, arg, yeGetInt(yeGet(map, "hitWallIdx")))
   ywidCallSignal(wid, nil, arg, yeGetInt(yeGet(map, "endTurnIdx")))
   yeDestroy(arg)
end

function moveHeadInternal(wid, map, oldPos, newPos)
   local mapElems = yeGet(map, "map")
   local destCase = yeGet(mapElems, newPos)
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

   yePushBack(yeGet(mapElems, newPos), headElem, nil)
   yeSetInt(yeGet(head, "pos"), newPos)
   yeRemoveChild(yeGet(yeGet(map, "map"), oldPos),
		 headElem)

   if bodyLen == 0 then
      ywidCallSignal(wid, nil, nil, yeGetInt(yeGet(map, "endTurnIdx")))
      return
   end

   local tailingBody = yeGet(body, 0)
   yePopBack(yeGet(mapElems, yeGetInt(tailingBody)))
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
   yeCreateInt(4, yeGet(mapElems, yeGetInt(headBody)))
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

   local oldPos = yeGetInt(yeGet(head, "pos"))
   local newPos = oldPos +
      yeGetInt(yeGet(dir, "y")) * yeGetInt(yeGet(map, "width")) +
      yeGetInt(yeGet(dir, "x"))

   -- check out of border
   if (oldPos % width) == 0 and (newPos % width) == (width - 1) then
      hitWall(wid, map, oldPos, newPos, 0)
      return
   elseif (newPos % width) == 0 and (oldPos % width) == (width - 1) then
      hitWall(wid, map, oldPos, newPos, 1)
      return
   elseif (newPos < 0) then
      hitWall(wid, map, oldPos, newPos, 2)
      return
   elseif (newPos > lenMap - 1) then
      hitWall(wid, map, oldPos, newPos, 3)
      return
   end
   moveHeadInternal(wid, map, oldPos, newPos)
end


function changeDir(map, eve)
   local head = yeGet(map, "head")
   local dir = yeGet(head, "dir")
   local x = yeGet(dir, "x")
   local y = yeGet(dir, "y")

   if ywidEveKey(eve) == Y_UP_KEY then
      if (yeGetInt(x) == 0 and yeGetInt(y) == 1) then
	 return
      end
      yeSetInt(x, 0)
      yeSetInt(y, -1)
   elseif ywidEveKey(eve) == Y_DOWN_KEY then
      if (yeGetInt(x) == 0 and yeGetInt(y) == -1) then
	 return
      end
      yeSetInt(x, 0)
      yeSetInt(y, 1)
   elseif ywidEveKey(eve) == Y_RIGHT_KEY then
      if (yeGetInt(x) == -1 and yeGetInt(y) == 0) then
	 return
      end
      yeSetInt(x, 1)
      yeSetInt(y, 0)
   elseif ywidEveKey(eve) == Y_LEFT_KEY then
      if (yeGetInt(x) == 1 and yeGetInt(y) == 0) then
	 return
      end
      yeSetInt(x, -1)
      yeSetInt(y, 0)
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
   local ent = ywidEntity(wid);
   local mapLen = yeLen(yeGet(ent, "map"))
   local mapW = yeGetInt(yeGet(ent, "width"))
   local newPos = yeGet(arg, "newPos")

   if (dir == 3) then
      yeSetInt(newPos, yeGetInt(newPos) - mapLen)
   elseif (dir == 2) then
      yeSetInt(newPos, yeGetInt(newPos) + mapLen)
   elseif (dir == 1) then
      yeSetInt(newPos, yeGetInt(newPos) - mapW)
   elseif (dir == 0) then
      yeSetInt(newPos, yeGetInt(newPos) + mapW)
   end
   moveHeadInternal(wid, ent, yeGetInt(yeGet(arg, "oldPos")), yeGetInt(newPos))
end

function initSnake(entity)
   ywidAddCallback(ywidCreateCallback("snakeAction", action))
   ywidAddCallback(ywidCreateCallback("snakeDie", die))
   ywidAddCallback(ywidCreateCallback("snakeWarp", warp))
   local init = yeCreateArray(nil, nil)
   yeCreateString("snake", init, "name")
   yeCreateFunction("createSnake", 1, init, "callback")
   ywidAddSubType(init)
   yeCreateInt(0, entity, "score");
end
