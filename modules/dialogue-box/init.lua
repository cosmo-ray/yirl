local default_color = "rgba: 255 255 255 255"
local border_threshold = 10

function newTextAndAnswerDialogue(canvas, x, y, dialogue, father, name)
   name = ylovePtrToString(name)
   local ret = yeCreateArray(father, name)
   local b0 = yeCreateArray(ret)
   x = yLovePtrToNumber(x)
   y = yLovePtrToNumber(y)
   local tmp0
   local size

   if yeType(dialogue) == YSTRING then
      tmp0 = ywCanvasNewText(canvas, x + border_threshold,
			     y + border_threshold, dialogue)
      size = ywCanvasObjSize(canvas, tmp0)
      size = ywSizeCreate(size)
      yePushBack(b0, tmp0)
   else
      tmp0 = ywCanvasNewText(canvas, x + border_threshold,
			     y + border_threshold, yeGet(dialogue, "text"))
      size = ywCanvasObjSize(canvas, tmp0)
      size = ywSizeCreate(size)
      yePushBack(b0, tmp0)
   end

   local rect = yeCreateArray()
   ywSizeCreate(ywSizeW(size) + border_threshold,
		ywSizeH(size) + border_threshold, rect);
   yeCreateString(default_color, rect);
   local tmp1 = ywCanvasNewRect(canvas, x, y, rect);
   ywCanvasSwapObj(canvas, tmp0, tmp1)
   yeDestroy(rect)
   yePushBack(ret, tmp1)
   yePushBack(ret, size)
   yeDestroy(size)
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
   ywCanvasRemoveObj(canvas, yeGet(box, 1))
end

function initDialogueBox(mod)
   yeCreateFunction("newTextAndAnswerDialogue", mod, "new_menu")
   yeCreateFunction("newTextDialogue", mod, "new_text")
   yeCreateFunction("rmTextDialogue", mod, "remove")
end
