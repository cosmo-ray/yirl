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


function snakeMap(entity)
   local map = yeCreateArray(entity, "start");
   local i = 0;

   yeCreateString( "map", map, "<type>")
   yePushBack(map, yeGet(entity, "SnakeResources"), "resources")
   yeCreateInt(100000, map, "turn-length")
   yeCreateInt(20, map, "width")
   local case = yeCreateArray(map, "map");

   while i < 20 * 20 do
      local tmp = yeCreateArray(case, NULL)
      yeCreateInt(0, tmp, NULL)
      i = i + 1
   end
   return map
end

Q_KEY = 113

function snakeAction(wid, eve, arg)
  while ywidEveIsEnd(eve) == false do
    if ywidEveType(eve) == YKEY_DOWN then
       if ywidEveKey(eve) == Q_KEY then
          ywidCallCallbackByStr("FinishGame", wid, eve, false)
       else
       	  print("hello: ", ywidEveKey(eve))
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
   ywidAddCallback(ywidCreateCallback("snakeAction", action))

   ywidBind(map, "action", "snakeAction")
   ywidSetMainWid(map, 0)
end