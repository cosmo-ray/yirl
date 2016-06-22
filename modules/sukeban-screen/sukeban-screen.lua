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


function sksAction(wid, eve, arg)
   while ywidEveIsEnd(eve) == false do
      if ywidEveType(eve) == YKEY_DOWN then
	 if ywidEveKey(eve) == Q_KEY then
	    ywidCallCallbackByStr("FinishGame", wid, eve, false)
	 end
      end
      eve = ywidNextEve(eve)
   end
   return YEVE_NOTHANDLE
end

function sukeNewMap(entity)
   local layers = yeCreateArray()
   local mapPath = yeGet(entity, "map")
   local resources = yeGet(entity, "resources")

   ygCall("sm-reader", "load-entity", entity)
   return ywidNewWidget(entity, nil)
end

function sukeScreeenNewWid(entity)
   yeCreateInt(75, yeGet(yeGet(entity, "entries"), 0), "size")

   local cnt = ywidNewWidget(entity, "contener")
   ywidBind(cnt, "action", "sks-action")
   return cnt
end

function initSukeScreen(entity)
   local init = yeCreateArray(nil, nil)
   yeCreateString("sukeban-screen", init, "name")
   yeCreateFunction("sukeScreeenNewWid", init, "callback")

   local sksAction = yeCreateFunction("sksAction", entity, "action")
   ywidAddCallback(ywidCreateCallback("sks-action", sksAction))
   ywidAddSubType(init)

   init = yeCreateArray(nil, nil) -- this has been destroy by ywidAddSubType
   yeCreateString("sukeban-map", init, "name")
   yeCreateFunction("sukeNewMap", init, "callback")
   ywidAddSubType(init)
end

