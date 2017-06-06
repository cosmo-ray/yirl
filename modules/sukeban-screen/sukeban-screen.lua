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
	 print("key: ", ywidEveKey(eve))
	 if ywidEveKey(eve) == Q_KEY then
	    ygCall(nil, "FinishGame")
	    return  YEVE_ACTION
	 elseif ywidEveKey(eve) == 112 then
	    return  YEVE_ACTION
	 end
      end
      eve = ywidNextEve(eve)
   end
   return YEVE_NOTHANDLE
end


function sukeMapAction(wid, eve, arg)
   while ywidEveIsEnd(eve) == false do
      if ywidEveType(eve) == YKEY_DOWN then
	 print("to the ")
	 if ywidEveKey(eve) == Y_UP_KEY then
	    print("up")
	    return  YEVE_ACTION
	 elseif ywidEveKey(eve) == Y_DOWN_KEY then
	    print("down of victory")
	    return  YEVE_ACTION
	 elseif ywidEveKey(eve) == Y_LEFT_KEY then
	    print("left")
	    return  YEVE_ACTION
	 elseif ywidEveKey(eve) == Y_RIGHT_KEY then
	    print("right")
	    return  YEVE_ACTION
	 end
      end
      eve = ywidNextEve(eve)
   end
   return YEVE_NOTHANDLE
end

function sukeNewMap(entity)
   ygCall("sm-reader", "load-entity", entity)
   local cnt = ywidNewWidget(entity, nil)
   ywidBind(cnt, "action", "sukeban-screen:map-action")
   return cnt
end

function sukeScreenNewWid(entity)
   yeCreateInt(75, yeGet(yeGet(entity, "entries"), 0), "size")

   local cnt = ywidNewWidget(entity, "contener")
   ywidBind(cnt, "action", "sukeban-screen:action")
   return cnt
end

function initSukeScreen(entity)
   local init = yeCreateArray(nil, nil)
   yeCreateString("sukeban-screen", init, "name")
   yeCreateFunction("sukeScreenNewWid", init, "callback")
   ywidAddSubType(init)

   yeCreateFunction("sksAction", entity, "action")
   yeCreateFunction("sukeMapAction", entity, "map-action")

   init = yeCreateArray(nil, nil) -- this has been destroy by ywidAddSubType
   yeCreateString("sukeban-map", init, "name")
   yeCreateFunction("sukeNewMap", init, "callback")
   ywidAddSubType(init)
end

