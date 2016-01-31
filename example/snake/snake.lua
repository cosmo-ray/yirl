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

function snakeMap(entity)
   local map = yeCreateArray(entity, "start")
   local i = 0;

   yeCreateString( "map", map, "<type>")
   yePushBack(map, yeGet(entity, "SnakeResources"), "resources")
   yeCreateInt(200000, map, "turn-length")
   yeCreateInt(20, map, "width")
   yeCreateInt(0, map, "nbPeanut")

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
   local lenMap = yeLen(yeGet(map, "map"))
   local width = yeGetInt(yeGet(map, "width"))
   local head = yeGet(map, "head")
   local headElem = yeGet(head, "elem")
   local dir = yeGet(head, "dir")

   local oldPos = yeGetInt(yeGet(head, "pos"))
   local newPos = oldPos +
      yeGetInt(yeGet(dir, "y")) * yeGetInt(yeGet(map, "width")) +
      yeGetInt(yeGet(dir, "x"))

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
      rmPeanut(map, destCase)
   end

   yePushBack(yeGet(yeGet(map, "map"), newPos), headElem, nil)
   yeSetInt(yeGet(head, "pos"), newPos)
   yeRemoveChild(yeGet(yeGet(map, "map"), oldPos),
		 headElem)
end


function changeDir(map, eve)
   local head = yeGet(map, "head")
   local dir = yeGet(head, "dir")
   local x = yeGet(dir, "x")
   local y = yeGet(dir, "y")

   if ywidEveKey(eve) == Y_UP_KEY then
      yeSetInt(x, 0)
      yeSetInt(y, -1)
   elseif ywidEveKey(eve) == Y_DOWN_KEY then
      yeSetInt(x, 0)
      yeSetInt(y, 1)
   elseif ywidEveKey(eve) == Y_RIGHT_KEY then
      yeSetInt(x, 1)
      yeSetInt(y, 0)
   elseif ywidEveKey(eve) == Y_LEFT_KEY then
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
end


function initSnake(entity)
   -- TODO: this functions: C/lua
   local mapEntity = snakeMap(entity)

   local map = ywidNewWidget(mapEntity)

   local action = yeCreateFunction("snakeAction", entity, "snakeAction", 3)
   yuiRandInit()
   ywidAddCallback(ywidCreateCallback("snakeAction", action))

   ywidBind(map, "action", "snakeAction")
   ywidSetMainWid(map, 0)
end
