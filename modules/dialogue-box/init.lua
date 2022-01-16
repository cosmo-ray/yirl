local dir_path = Entity.wrapp(ygGet("DialogueBox.$path")):to_string()
local arrow_path = dir_path .. "/arrow_sheet.png"
local default_color = "rgba: 255 255 255 255"
local border_threshold = 10
local txt_threshold = 10
local arrow_size = 25
local arrow_threshold = 6
local arrow_tot = arrow_size + arrow_threshold

local BOX_IDX = 0
local CAN_RECT_IDX = 1
local SIZE_IDX = 2
local ARROW_IDX = 3
local DIALOG_IDX = 4
local POS_IDX = 5



function getDialogue(box)
   return yeGet(box, DIALOG_IDX)
end

function getAnswers(box)
   local d = getDialogue(box)
   if yeType(d) == YARRAY then
      return yeGet(d, "answers")
   end
   return nil
end

function pushAnswers(box, answers)
   local ret = yeReplaceBack(getDialogue(box), answers, "answers")
   return ret
end

function getAnswer(box, idx)
   idx = yLovePtrToNumber(idx)
   local answers = getAnswers(box)
   local i = 0
   local j = 0
   local len = yeLen(answers)
   while i < len do
      local answer = yeGet(answers, i)
      if yeType(answer) == YARRAY then
	 local hiden = yeGetInt(yeGet(answer, "hiden"))
	 if (hiden ~= 1) then
	    if j == idx then
	       return answer
	    end
	    j = j + 1
	 end
      elseif yeType(answer) == YSTRING  then
	 if j == idx then
	    return answer
	 end
	 j = j + 1
      end
      i = i + 1
   end
   return nil
end

function getPos(box)
   return yeGetInt(yeGet(box, POS_IDX))
end

function posArray(box, idx)
   idx = yLovePtrToInt32(idx)
   local pos = ywCanvasObjPos(yeGet(yeGet(box, BOX_IDX), idx + 1))

   if idx < 0 or yLovePtrToNumber(pos) == 0 then
      if idx >= 0  and getAnswer(box, idx) ~= nil then
	 yeSetInt(yeGet(box, POS_IDX), idx);
      end
      return
   end

   --ywPosPrint(pos)
   pos = ywPosCreate(pos)
   --ywPosPrint(pos)
   ywPosAdd(pos, -arrow_tot, 0)
   --ywPosPrint(pos)
   ywCanvasObjSetPos(yeGet(box, ARROW_IDX), pos)
   yeSetInt(yeGet(box, POS_IDX), idx);
   yeDestroy(pos)
end

function newTextAndAnswerDialogue(canvas, x, y, dialogue, father, name)
   name = ylovePtrToString(name)
   local ret = yeCreateArray(father, name)

   reloadTextAndAnswerDialogue(canvas, x, y, dialogue, ret)
   return ret
end

local function getText(dialogue, gc)
   local tmpText = yeGet(dialogue, "text")

   if yeType(tmpText) == YARRAY then
      local array = tmpText
      local j = 0

      tmpText = yeCreateString("", gc)
      while j < yeLen(array) do
	 yeStringAddNl(tmpText, yeGetStringAt(array, j))
	 j = j + 1
      end
   end
   if (yIsNil(yeGetString(tmpText))) then
      print("BUG: CAN'T RETRIVE \"text\"")
      yeSetString(tmpText, "BUG: CAN'T RETRIVE \"text\"");
      return tmpText
   end

   return tmpText
end

function reloadTextAndAnswerDialogue(canvas, x, y, dialogue, ret)
   if (yIsNil(canvas)) then
      return
   end

   local gc = yeCreateArray()
   local b0 = yeCreateArray(gc)
   x = yLovePtrToNumber(x)
   y = yLovePtrToNumber(y)
   local tmp0 = nil
   local size = nil
   local arrow = nil

   print("in 0")
   if yeType(dialogue) == YSTRING then
      local tmpText = yeCreateYirlFmtString(dialogue, gc)
      tmp0 = ywCanvasNewText(canvas, x + border_threshold,
			     y + border_threshold, tmpText)
      size = ywCanvasObjSize(canvas, tmp0)
      size = ywSizeCreate(size, gc)
      yePushBack(b0, tmp0)
   else
      local answers = yeGet(dialogue, "answers")
      local len = yeLen(answers)
      local name = yeGet(dialogue, "name")
      local i = 0
      local tmpText = yeGet(dialogue, "text")

      tmpText = getText(dialogue, gc)
      tmpText = yeCreateYirlFmtString(tmpText, gc)

      tmp0 = ywCanvasNewText(canvas, x + border_threshold,
			     y + border_threshold, tmpText)
      yePushBack(b0, tmp0)
      size = ywCanvasObjSize(canvas, tmp0)
      local x0 = ywPosX(ywCanvasObjPos(tmp0))
      local w = ywSizeW(size)
      local h = ywSizeH(size)
      while i < len do
	 local txt = yeGet(answers, i)
	 if yeType(txt) == YSTRING then
	    yeConvert(txt, YARRAY)
	    yeRenameIdxStr(txt, 0, "text")
	 end
	 local hiden = yeGetInt(yeGet(txt, "hiden"))
	 if (hiden ~= 1) then
	    local x = x0 + arrow_tot
	    local y = ywPosY(ywCanvasObjPos(tmp0)) + txt_threshold + ywSizeH(size)

	    tmpText = getText(txt, gc)
	    tmpText = yeCreateYirlFmtString(tmpText, gc)
	    tmp0 = ywCanvasNewText(canvas, x, y, tmpText)
	    size = ywCanvasObjSize(canvas, tmp0)
	    if ywSizeW(size) + arrow_tot > w then
	       w = ywSizeW(size) + arrow_tot
	    end
	    h = h + ywSizeH(size) + txt_threshold
	    yePushBack(b0, tmp0)
	 end
	 i = i + 1
      end
      size = ywSizeCreate(w, h, gc)
      arrow = ywCanvasNewImg(canvas, x, y, arrow_path,
			     ywRectCreate(0, 0, 25, 20, gc))
   end

   local rect = yeCreateArray(gc)
   ywSizeCreate(ywSizeW(size) + border_threshold * 2,
		ywSizeH(size) + border_threshold * 2, rect);
   yeCreateString(default_color, rect);
   local tmp1 = ywCanvasNewRect(canvas, x, y, rect);
   ywCanvasSwapObj(canvas, yeGet(b0, 0), tmp1)

   yePushAt(ret, b0, BOX_IDX)
   yePushAt(ret, tmp1, CAN_RECT_IDX)
   yePushAt(ret, size, SIZE_IDX)
   yePushAt(ret, arrow, ARROW_IDX)
   yePushAt(ret, dialogue, DIALOG_IDX)
   if yeGetInt(yeGet(ret, POS_IDX)) == 0 then
      local i = yeCreateInt(0, gc);
      yePushAt(ret, i, POS_IDX)
   end
   posArray(ret, getPos(ret))
   yeDestroy(gc)
end

function newEmptyDialogue(canvas, x, y, father, name)
   local ret = yeCreateArray(father, ylovePtrToString(name))
   local rect = yeCreateArray()
   local dialogue = yeCreateArray()
   ywSizeCreate(1, 1, rect);
   yeCreateString(default_color, rect);
   x = yLovePtrToNumber(x)
   y = yLovePtrToNumber(y)
   local tmp1 = ywCanvasNewRect(canvas, x, y, rect)
   yePushAt(ret, tmp1, 1)
   yePushAt(ret, dialogue, 4)
   local i = yeCreateInt(0)
   yePushAt(ret, i, 5)
   yeDestroy(dialogue)
   yeDestroy(rect)
   yeDestroy(i)
   return ret
end

function newTextDialogue(canvas, x, y, text, father, name)
   local ret
   text = ylovePtrToString(text)
   local e_txt = yeCreateString(text)
   ret = newTextAndAnswerDialogue(canvas, x, y, e_txt, father, name)
   yeDestroy(e_txt)
   return ret
end

function getTextDialogue(box)
   if yLovePtrToNumber(box) == 0 then
      return nil
   end

   local txt = yeGet(box, DIALOG_IDX)
   if yeType(txt) == YSTRING then
      return txt
   end
      return yeGet(txt, "text")
end

function rmTextDialogue(canvas, box)
   local b0 = yeGet(box, BOX_IDX)
   local len = yeLen(b0)
   local i = 0

   while i < len do
      ywCanvasRemoveObj(canvas, yeGet(b0, i))
      i = i + 1
   end
   ywCanvasRemoveObj(canvas, yeGet(box, CAN_RECT_IDX))
   ywCanvasRemoveObj(canvas, yeGet(box, ARROW_IDX))
end

function reload(canvas, box)
   local dialogue = getDialogue(box)
   local pos = ywCanvasObjPos(yeGet(box, CAN_RECT_IDX))
   local x = ywPosX(pos)
   local y = ywPosY(pos)

   rmTextDialogue(canvas, box)
   reloadTextAndAnswerDialogue(canvas, x, y, dialogue, box)
end

function setPos(box, x, y)
   if yLovePtrToNumber(box) == 0 then
      return
   end
   local b0 = yeGet(box, BOX_IDX)
   local len = yeLen(b0)
   local i = 0
   x = yLovePtrToNumber(x)
   y = yLovePtrToNumber(y)

   while i < len do
      ywCanvasObjSetPos(yeGet(b0, i), x, y)
      i = i + 1
   end
   ywCanvasObjSetPos(yeGet(box, CAN_RECT_IDX), x, y)
   ywCanvasObjSetPos(yeGet(box, ARROW_IDX), x, y)
end

function getCurAnswer(box)
   return getAnswer(box, getPos(box))
end

function initDialogueBox(mod)
   yeCreateFunction("newTextAndAnswerDialogue", mod, "new_menu")
   yeCreateFunction("newTextDialogue", mod, "new_text")
   yeCreateFunction("getTextDialogue", mod, "get_text")
   yeCreateFunction("newEmptyDialogue", mod, "new_empty")
   yeCreateFunction("rmTextDialogue", mod, "remove")
   yeCreateFunction("rmTextDialogue", mod, "rm")
   yeCreateFunction("posArray", mod, "moveAnswer")
   yeCreateFunction("getAnswer", mod, "getAnswer")
   yeCreateFunction("getCurAnswer", mod, "getCurAnswer")
   yeCreateFunction("getAnswers", mod, "getAnswers")
   yeCreateFunction("getDialogue", mod, "getDialogue")
   yeCreateFunction("reload", mod, "reload")
   yeCreateFunction("getPos", mod, "pos")
   yeCreateFunction("setPos", mod, "set_pos")
   yeCreateFunction("pushAnswers", mod, "pushAnswers")
   yeCreateInt(6, mod, "privateDataSize")
end
