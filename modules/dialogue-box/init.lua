local default_color = "rbga: 255 255 255 255"

function newTextDIalogue(canvas, x, y, text)
   local ret = yeCreateArray()
   text = ylovePtrToString(text)
   x = yLovePtrToNumber(x)
   y = yLovePtrToNumber(y)
   local e_txt = yeCreateString(text)
   local tmp0 = ywCanvasNewText(canvas, x, y, e_txt)

   yePushBack(ret, tmp0)
   local rect = yeCreateArray()
   local size = ywCanvasObjSize(canvas, tmp0)
   ywSizeCreate(ywSizeW(size), ywSizeH(size), rect);
   yeCreateString("rgba: 255 255 255 255", rect);
   local tmp1 = ywCanvasNewRect(canvas, x, y, rect);
   ywCanvasSwapObj(canvas, tmp0, tmp1)
   yeDestroy(e_txt)
   yeDestroy(rect)
   yePushBack(ret, tmp0)
   yePushBack(ret, tmp1)
   return ret
end

function rmTextDIalogue(canvas, box)
   ywCanvasRemoveObj(canvas, yeGet(box, 0))
   ywCanvasRemoveObj(canvas, yeGet(box, 1))
end

function initDialogueBox(mod)
   yeCreateFunction("newTextDIalogue", mod, "new")
   yeCreateFunction("rmTextDIalogue", mod, "remove")
end
