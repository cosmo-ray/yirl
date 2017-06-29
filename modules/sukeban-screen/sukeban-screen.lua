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
	 --print("key: ", ywidEveKey(eve))
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


function sukeMapCaracLayer(map)
   return ywCntGetEntry(map, 1);
end

function sukeMapObjsLayer(map)
   return ywCntGetEntry(map, 2);
end

function sukeMapAction(wid, eve, arg)
   local x = 0
   local y = 0
   local ret = YEVE_NOTHANDLE

   while ywidEveIsEnd(eve) == false do
      if ywidEveType(eve) == YKEY_DOWN then
	 if ywidEveKey(eve) == Y_UP_KEY then
	    y = -1
	    ret = YEVE_ACTION
	 elseif ywidEveKey(eve) == Y_DOWN_KEY then
	    y = 1
	    ret = YEVE_ACTION
	 elseif ywidEveKey(eve) == Y_LEFT_KEY then
	    x  = -1
	    ret = YEVE_ACTION
	 elseif ywidEveKey(eve) == Y_RIGHT_KEY then
	    x = 1
	    ret = YEVE_ACTION
	 end
      end
      if (ret == YEVE_ACTION) then
	 local id = yeGet(wid, "start_id");
	 local oldPos = yeGet(id, "pos");
	 local newPos = ywPosCreate(oldPos);
	 ywPosAdd(newPos, x, y);

	 if (yeLen(ywMapGetCase(sukeMapObjsLayer(wid), newPos)) == 0 and
	     yeLen(ywMapGetCase(sukeMapCaracLayer(wid), newPos)) == 0) then
	    ywMapAdvence(sukeMapCaracLayer(wid), oldPos, x, y, id)
	 end
	 yeDestroy(newPos)
      end
      eve = ywidNextEve(eve)
   end
   return ret
end

function sukeNewMap(entity)
   local npj = yeGet(entity, "npj");
   local i = 0

   ygCall("sm-reader", "load-entity", entity)
   yeCreateString("rgba: 255 255 255 255", entity, "background")
   ywPosCreate(yeGet(entity, "start_pos"), yeGet(entity, "start_id"), "pos")
   while i < yeLen(npj) do
      ywMapPushElem(sukeMapCaracLayer(entity), yeGet(npj, i))
      i = i + 1
   end
   local cnt = ywidNewWidget(entity)
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
   local init = yeCreateArray()
   yeCreateString("sukeban-screen", init, "name")
   yeCreateFunction("sukeScreenNewWid", init, "callback")
   ywidAddSubType(init)

   yeCreateFunction("sksAction", entity, "action")
   yeCreateFunction("sukeMapAction", entity, "map-action")
   yeCreateFunction("startDialogue", entity, "start-dialogue")

   init = yeCreateArray() -- this has been destroy by ywidAddSubType
   yeCreateString("sukeban-map", init, "name")
   yeCreateFunction("sukeNewMap", init, "callback")
   ywidAddSubType(init)
end

