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


Q_KEY = 113

function getLooseScreen(entity)
  return yeGet(yeGet(entity, "menus"), "LooseScreen")
end

function snakeMap(entity)
   local map = yeCreateArray(entity, "start")
   local i = 0;

   yePushBack(map, getLooseScreen(entity), "next")
   yeCreateString( "map", map, "<type>")
   yePushBack(map, yeGet(entity, "SnakeResources"), "resources")
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

function moveHead(map)
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
   local body = yeGet(map, "body")
   local bodyLen = yeLen(body)

   -- check out of border
   if (oldPos % width) == 0 and (newPos % width) == (width - 1) then
      return
   elseif (newPos % width) == 0 and (oldPos % width) == (width - 1) then
      return
   elseif (newPos < 0) then
      return
   elseif (newPos > lenMap) then
      return
   end

   local destCase = yeGet(yeGet(map, "map"), newPos)

   if yeLen(destCase) > 1 then
      if (yeGetInt(yeGet(destCase, 1)) ~= 2) then
	 ywidNext(yeGet(map, "next"))
      end
      rmPeanut(map, destCase)
      --add body :)
      addBody(map, oldPos)
      bodyLen = 0
   end

   yePushBack(yeGet(mapElems, newPos), headElem, nil)
   yeSetInt(yeGet(head, "pos"), newPos)
   yeRemoveChild(yeGet(yeGet(map, "map"), oldPos),
		 headElem)

   if bodyLen == 0 then
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
   -- push first body
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
   moveHead(map)
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

function scoreInit(wid, eve, args)
   local scoreStr = "you have a score of " .. 17 .. "points"
   yeSetString(yeGet(ywidEntity(wid), "text"), scoreStr);
end

function initSnake(entity)
   -- TODO: this functions: C/lua
   local mapEntity = snakeMap(entity)
   local map = ywidNewWidget(mapEntity)
   local action = yeCreateFunction("snakeAction", 3, entity, "snakeAction")
   local menuInit = yeCreateFunction("scoreInit", 3, entity, "scoreInit")

   yuiRandInit()
   ywidAddCallback(ywidCreateCallback("snakeAction", action))
   ywidAddCallback(ywidCreateCallback("scoreInit", menuInit))
   ywidBind(map, "action", "snakeAction")
   ywidSetMainWid(map, 0)
end
