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

function scoreInit(wid, eve, args)
   -- Get the score from the snake module
   local scoreEnt = yeGet(ygGetMod("snake"), "score")
   local score = yeGetInt(scoreEnt)
   local hs = ygFileToEnt(YJSON, "snake-hightscore.json")
   local scoreStr = "you have a score of " .. score .. " points"

   if (score < 30) then
      scoreStr = scoreStr .. " noob !"
   end
   if (yeGetInt(hs) > score) then
      scoreStr = scoreStr .. "\nhightscore:" .. yeGetInt(hs)
   else
      scoreStr = scoreStr .. "\nnew Hightscore !"
      ygEntToFile(YJSON, "snake-hightscore.json", scoreEnt)
   end
   yeSetString(yeGet(wid, "text"), scoreStr)
   yeSetInt(scoreEnt, 0);
   yeDestroy(hs)
end

function eat(map, eve, args)
   local tl = yeGet(map, "turn-length")
   local tlInt = yeGetInt(tl) - 2000

   if (tlInt < 50000) then
      return
   end
   yeSetInt(tl, tlInt)
end

yeCreateFunction("scoreInit", ygGetMod("testSnake"))
yeCreateFunction("eat", ygGetMod("testSnake"))
