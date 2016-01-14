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

print("loading snake.lua");

function snakeMap(entity)
   local map = yeCreateArray(entity, "start");
   local i = 0;

   yeCreateString( "map", map, "<type>")
   yePushBack(map, yeGet(entity, "SnakeResources"), "resources")
   print("res ", yeGet(entity, "SnakeResources"))
   print("res ", yeGet(map, "resources"))
   yeCreateInt(10000, map, "turn-length")
   yeCreateInt(20, map, "width")
   local case = yeCreateArray(map, "map");

   while i < 20 * 20 do
      local tmp = yeCreateArray(case, NULL)
      yeCreateInt(0, tmp, NULL)
      i = i + 1
   end
   return map
end

function initSnake(entity)
   -- TODO: this functions: C/lua
   local mapEntity = snakeMap(entity)

   print("init ", mapEntity)
   local map = ywidNewWidget(mapEntity)
   print("init ", map)
   ywidBind(map, "action", "FinishGame");
   ywidSetMainWid(map, 0)
end