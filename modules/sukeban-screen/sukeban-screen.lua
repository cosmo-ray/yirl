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

function elemPos(elem)
   return yeGet(elem, "pos")
end

function sukeAdvence(wid, from, x, y, guy)
   local newPos = ywPosCreate(from);
   ywPosAdd(newPos, x, y);
   local nObjsCase = ywMapGetCase(sukeMapObjsLayer(wid), newPos)
   local nCaracsCase = ywMapGetCase(sukeMapCaracLayer(wid), newPos)

   if yeLen(nObjsCase) == 0 and yeLen(nCaracsCase) == 0 then
      ywMapAdvence(sukeMapCaracLayer(wid), from, x, y, guy)
   else
      local i = 0
      while i < yeLen(nCaracsCase) do
	 local elem = yeGet(nCaracsCase, i)
	 actionCall((yeGet(elem, "action")), wid, id, elem)
	 i = i + 1
      end
      while i < yeLen(nObjsCase) do
	 local elem = yeGet(nObjsCase, i)

	 actionCall((yeGet(elem, "action")), wid, id, elem)
	 i = i + 1
      end
   end
end

function doBadGuyStuff(wid, carac, badGuy)
   badGuyPos = elemPos(badGuy)
   sukeAdvence(wid, badGuyPos, 1, 0, badGuy)
end

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

function startDialogue(wid, carac, elem, id)
   local gc = yeCreateArray()
   local speaker_background = yeCreateString("rgba: 255 255 255 255", gc)
   local dialogues = ygGet("sukeban.dialogues")
   local ent = yNewDialogueEntity(yeGet(dialogues, yeGetInt(id)),
				  nil, nil, speaker_background)
   yePushBack(ent, wid, "-sukeban-map");
   ywPushNewWidget(wid, ent, 1)
   yeSetAt(wid, "-inside-dialogue", 1)
   yeDestroy(gc)
   return YEVE_ACTION
end

function backToMap(wid, eve, arg)
   local sWid = yeGet(yDialogueGetMain(wid), "-sukeban-map")
   ywCntPopLastEntry(sWid)
   yeSetAt(sWid, "-inside-dialogue", 0)
end

function gotoMap(wid, carac, elem, arg1, arg2)
   local str = "sukeban.scenes." .. yeGetString(arg1)
   local nextMap = ygGet(str)
   local screen = yeGet(wid, "screen")
   --local entries = yeGet(screen, "entries")
   local newPos
   local cLayer = sukeMapCaracLayer(wid)
   local oldPos = elemPos(yeGet(nextMap, "start_id"))

   yeReplaceBack(nextMap, screen, "screen");
   yeReplaceBack(nextMap, carac, "start_id");
   ywMapRemove(cLayer, elemPos(carac), carac)
   if yLovePtrToNumber(arg2) ~= 0 then
      newPos = arg2
   else
      newPos = yeGet(nextMap, "start_pos")
   end

   if yLovePtrToNumber(oldPos) == 0 then
      ywPosSet(yeGet(nextMap, "start_pos"), newPos)
   else
      cLayer = sukeMapCaracLayer(nextMap)
      ywPosSet(elemPos(carac), newPos)
      ywMapPushElem(cLayer, carac, newPos)
   end
   --yeReplace(entries, yeGet(entries, 0), nextMap)
   ywReplaceEntry(screen, 0, nextMap)
end

function actionCall(action, wid, carac, elem)
   if yeType(action) == YSTRING then
      yesCall(ygGet(yeGetString(action)), wid, carac, elem)
   elseif yeType(action) == YARRAY then
      yesCall(ygGet(yeGet(action, 0)),
	      wid, carac, elem, yeGet(action, 1),
	      yeGet(action, 2), yeGet(action, 3))
   end
end

function sukeMapAction(wid, eve, arg)
   local x = 0
   local y = 0
   local ret = YEVE_NOTHANDLE

   if (yeGetInt(yeGet(wid, "-inside-dialogue")) == 1) then
      return YEVE_NOTHANDLE
   end
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
	 local oldPos = elemPos(id);
	 local npcs = yeGet(wid, "npc")

	 sukeAdvence(wid, oldPos, x, y, id)
	 yeDestroy(newPos)
	 local i = 0
	 while i < yeLen(npcs) do
	    local npc = yeGet(npcs, i)
	    local auto_action = yeGet(npc, "auto-action")
	    actionCall(auto_action, wid, id, npc)
	    i = i + 1
	 end
      end
      eve = ywidNextEve(eve)
   end
   return ret
end

function sukeNewMap(entity)
   local npc = yeGet(entity, "npc");
   local actionables = yeGet(entity, "actionables");
   local i = 0

   ygCall("sm-reader", "load-entity", entity)
   if yLovePtrToNumber(yeGet(entity, "background")) == 0 then
      yeCreateString("rgba: 255 255 255 255", entity, "background")
   end
   if yLovePtrToNumber(elemPos(yeGet(entity, "start_id"))) == 0 then
      ywPosCreate(yeGet(entity, "start_pos"), yeGet(entity, "start_id"), "pos")
   else
      ywPosSet(elemPos(yeGet(entity, "start_id")), yeGet(entity, "start_pos"))
   end
   while i < yeLen(npc) do
      ywMapPushElem(sukeMapCaracLayer(entity), yeGet(npc, i))
      i = i + 1
   end
   i = 0
   while i < yeLen(actionables) do
      local action = yeGet(actionables, i)
      local action_case = ywMapGetCase(sukeMapObjsLayer(entity),
				       elemPos(action))
      local j = 0

      while j < yeLen(action_case) do
	 local elem = yeGet(action_case, j)

	 if ywMapGetResourceId(entity, action) == ywMapGetIdByElem(elem) then
	    if yeType(elem) == YINT then
	       local tmp = yeCreateArray();

	       yePushBack(tmp, elem, "id");
	       elem = yeReplaceAtIdx(action_case, tmp, j);
	       yeDestroy(tmp);
	    end
	    yeReplaceBack(elem, yeGet(action, "action"), "action")
	 end
	 j = j + 1
      end
      i = i + 1
   end
   local cnt = ywidNewWidget(entity)
   ywidBind(cnt, "action", "sukeban-screen:map-action")
   yeCreateInt(0, entity, "-inside-dialogue");
   return cnt
end

function sukeScreenGetMap(screen)
   return ywCntGetEntry(screen, 0);
end


function sukeScreenNewWid(entity)
   yeCreateInt(80, yeGet(yeGet(entity, "entries"), 0), "size")

   local cnt = ywidNewWidget(entity, "contener")
   ywidBind(cnt, "action", "sukeban-screen:action")
   yePushBack(sukeScreenGetMap(entity), entity, "screen")
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
   yeCreateFunction("gotoMap", entity, "goto")
   yeCreateFunction("backToMap", entity, "back-to-map")
   yeCreateFunction("doBadGuyStuff", entity, "do-bad-guy-stuff")

   init = yeCreateArray() -- this has been destroy by ywidAddSubType
   yeCreateString("sukeban-map", init, "name")
   yeCreateFunction("sukeNewMap", init, "callback")
   ywidAddSubType(init)
end

