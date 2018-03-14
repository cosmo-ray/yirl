local arrow_path = YIRL_MODULES_PATH .. "dialogue-box/arrow_sheet.png"
local default_color = "rgba: 255 255 255 255"
local border_threshold = 10
local txt_threshold = 10
local arrow_size = 25
local arrow_threshold = 5
local arrow_tot = arrow_size + arrow_threshold

function posArray(box, idx)
   idx = yLovePtrToNumber(idx)
   local pos = ywCanvasObjPos(yeGet(yeGet(box, 0), idx + 1))

   if yLovePtrToNumber(pos) == 0 then
      return
   end

   ywPosPrint(pos)
   pos = ywPosCreate(pos)
   ywPosPrint(pos)
   ywPosAdd(pos, -arrow_tot, 0)
   ywPosPrint(pos)
   ywCanvasObjSetPos(yeGet(box, 3), pos)
   yeDestroy(pos)
end

function newTextAndAnswerDialogue(canvas, x, y, dialogue, father, name)
   name = ylovePtrToString(name)
   local gc = yeCreateArray()
   local ret = yeCreateArray(father, name)
   local b0 = yeCreateArray(ret)
   x = yLovePtrToNumber(x)
   y = yLovePtrToNumber(y)
   local tmp0 = nil
   local size = nil
   local arrow = nil

   if yeType(dialogue) == YSTRING then
      tmp0 = ywCanvasNewText(canvas, x + border_threshold,
			     y + border_threshold, dialogue)
      size = ywCanvasObjSize(canvas, tmp0)
      size = ywSizeCreate(size, gc)
      yePushBack(b0, tmp0)
   else
      local answers = yeGet(dialogue, "answers")
      local len = yeLen(answers)
      local i = 0
      tmp0 = ywCanvasNewText(canvas, x + border_threshold,
			     y + border_threshold, yeGet(dialogue, "text"))
      yePushBack(b0, tmp0)
      size = ywCanvasObjSize(canvas, tmp0)
      local x0 = ywPosX(ywCanvasObjPos(tmp0))
      local w = ywSizeW(size)
      local h = ywSizeH(size)
      while i < len do
	 local x = x0 + arrow_tot
	 local y = ywPosY(ywCanvasObjPos(tmp0)) + txt_threshold + ywSizeH(size)
	 local txt = yeGet(answers, i)

	 tmp0 = ywCanvasNewText(canvas, x, y, yeGet(txt, "text"))
	 size = ywCanvasObjSize(canvas, tmp0)
	 if ywSizeW(size) + arrow_tot > w then
	    w = ywSizeW(size) + arrow_tot
	 end
	 h = h + ywSizeH(size) + txt_threshold
	 i = i + 1
	 yePushBack(b0, tmp0)
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

   yePushBack(ret, tmp1)
   yePushBack(ret, size)
   yePushBack(ret, arrow)
   posArray(ret, 0)
   yeDestroy(gc)
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

function rmTextDialogue(canvas, box)
   local b0 = yeGet(box, 0)
   local len = yeLen(b0)
   local i = 0

   while i < len do
      ywCanvasRemoveObj(canvas, yeGet(b0, i))
      i = i + 1
   end
   print(yeGet(box, 1), yeGet(box, 2))
   ywCanvasRemoveObj(canvas, yeGet(box, 1))
   ywCanvasRemoveObj(canvas, yeGet(box, 3))
end

function initDialogueBox(mod)
   yeCreateFunction("newTextAndAnswerDialogue", mod, "new_menu")
   yeCreateFunction("newTextDialogue", mod, "new_text")
   yeCreateFunction("rmTextDialogue", mod, "remove")
   yeCreateFunction("posArray", mod, "moveAnswer")
end
