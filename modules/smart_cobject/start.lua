
local lpcs = nil
local sprite_man = nil

local LPCS_LEFT = nil
local LPCS_DOWN = nil
local LPCS_RIGHT = nil
local LPCS_UP = nil
local LPCS_DEAD = nil

local LPCS_ATK_UP = 12
local LPCS_ATK_DOWN = 15

local CHAR_I = 0 -- only for canvas layers
local WID_I = 1 -- only for canvas layers
local CVS_OBJ_I = 2
local POS_I = 3

function yGenericCurCanvas(handler)
   handler = Entity.wrapp(handler)
   local type = yeGetString(handler.char.type)
   if  type == "layer-canvas" then
      return handler.cvs_objs[0]
   end
   return handler.canvas
end

local function refresh_txt_array(handler)
   local pos = handler.pos
   local txts = handler.txts[handler.cur_array:to_string()]
   local canvas = ywCanvasNewImgFromTexture(handler.wid, ywPosX(pos), ywPosY(pos),
					    yeGet(txts, yeGetIntAt(handler, "cur_txt")))
   handler.canvas = canvas
   if handler.flip > 0 then
      ywCanvasHFlip(canvas);
   end

   if yIsNNil(handler.colorMod) then
      local col_mod = handler.colorMod
      ywCanvasSetColorModRGBA(canvas, yeGetIntAt(col_mod, 0), yeGetIntAt(col_mod, 1),
			      yeGetIntAt(col_mod, 2), yeGetIntAt(col_mod, 3))
   end

   if yIsNNil(handler[POS_I]) then
      ywCanvasObjReplacePos(handler.canvas, handler[POS_I]);
   end
end


function yGenericTextureArrayCheck(handler, what)
   handler = Entity.wrapp(handler)
   return yIsNNil(handler.txts[what])
end


function yGenericTextureArraySet(handler, what)
   handler = Entity.wrapp(handler)
   if yIsNil(handler.txts[what]) then
      return
   end
   handler.cur_array = what
   handler.cur_txt = 0
end

function yGenericNewTexturesArray(wid, array, char, pos, parent, name)
   if yIsNil(array) then
      return nil
   end
   local ret = Entity.new_array(parent, name)
   ret.char = char
   ret.char.type = "textures-array"
   ret.wid = wid
   yeCreateHash(ret, "txts")
   ret.txts.base = array
   yePushBack(ret, pos, "pos")
   ret.cur_array = "base"
   ret.cur_txt = 0
   ret.flip = 0
   refresh_txt_array(ret)
   return ret
end

function yGenericUsePos(handler, pos)
   handler = Entity.wrapp(handler);
   local type = yeGetString(handler.char.type)

   if type == "textures-array" then
      ywCanvasObjReplacePos(handler.canvas, pos);
      yePushAt2(handler, pos, POS_I, "pos");
   else
      print("yGenericUsePos not implemented")
   end
end

function yGenericNewCanvasLayer(wid, canvasArray, char, parent, name)
   if yIsNil(canvasArray) then
      return nil
   end
   canvasArray = Entity.wrapp(canvasArray)
   local ret = Entity.new_array(parent, name)
   local pos = Entity.wrapp(ywCanvasObjPos(canvasArray[0]))

   ret.char = char
   ret.wid = wid
   ret.cvs_objs = canvasArray
   yePushBack(ret, pos, "pos")

   for i = 1, yeLen(canvasArray) -1 do
      yePushAt2(canvasArray[i], pos, YCANVAS_POS_IDX, "pos");
   end
   return ret;
end

function distanceToDir(x, y)
   if math.abs(x) > math.abs(y) then
      if x > 0 then
	 return LPCS_RIGHT
      else
	 return LPCS_LEFT
      end
   else
      if y > 0 then
	 return LPCS_DOWN
      else
	 return LPCS_UP
      end
   end
end

function lpcsDirToXY(dir)
   if yIsLuaNum(dir) == false then
      dir = yeGetInt(dir)
   end

   if dir == LPCS_LEFT then
      return -1,0
   elseif dir == LPCS_RIGHT then
      return 1,0
   elseif dir == LPCS_UP then
      return 0,-1
   end
   return 0,1
end

function lpcsStrToDir(sdir)
   if sdir == "left" then
      return LPCS_LEFT
   elseif sdir == "right" then
      return LPCS_RIGHT
   elseif sdir == "down" then
      return LPCS_DOWN
   end
   return LPCS_UP
end

function lpcsDirToStr(sdir)
   if sdir == LPCS_LEFT then
      return "left"
   elseif sdir == LPCS_RIGHT then
      return "right"
   elseif sdir == LPCS_DOWN then
      return "down"
   elseif sdir == LPCS_UP then
      return "up"
   end
   return "unknow"
end

function yGenericHandlerRmCanva(npc)
   npc = Entity.wrapp(npc)
   local type = yeGetString(npc.char.type)
   if type == "sprite" then
      sprite_man.handlerRemoveCanva(npc)
   elseif type == "layer-canvas" then
      local cvs = npc[CVS_OBJ_I]
      for i = 0, yeLen(cvs) - 1 do
	 ywCanvasRemoveObj(npc.wid, cvs[i])
      end
   else
      lpcs.handlerRemoveCanva(npc)
   end
end

function sprite_getdir(npc)
   local sdispo = yeGetString(npc.sp.disposition)
   local x_off_idx = yeGetIntAt(npc, "y_offset");

   if yeGetString(npc.sp.disposition) == "uldr" then
      if x_off_idx == 32 then
	 return LPCS_LEFT
      elseif x_off_idx == 96 then
	 return LPCS_RIGHT
      elseif x_off_idx == 64 then
	 return LPCS_DOWN
      end
      return LPCS_UP
   elseif yeGetString(npc.sp.disposition) == "urdl" then
      if x_off_idx == 96 then
	 return LPCS_LEFT
      elseif x_off_idx == 32 then
	 return LPCS_RIGHT
      elseif x_off_idx == 64 then
	 return LPCS_DOWN
      end
      return LPCS_UP
   else
      if x_off_idx == 32 then
	 return LPCS_LEFT
      elseif x_off_idx == 64 then
	 return LPCS_RIGHT
      elseif x_off_idx == 96 then
	 return LPCS_UP
      end
      return LPCS_DOWN
   end
end

function yGenericHandlerShowDead(npc)
   npc = Entity.wrapp(npc)
   local npc_char = npc.char
   local type = yeGetString(npc_char.type)

   if type == "layer-canvas" then
      return false
   elseif type == "sprite" then
      local npc_c_sprite = npc_char.sprite
      local dead_idx = yeGetInt(npc_c_sprite['dead-txt-idx'])
      if dead_idx < 1 then
	 return false
      end
      local x_off_idx = yeGetIntAt(npc, "y_offset") / 32;

      npc.text_idx = dead_idx
      sprite_man.handlerSetAdvancement(npc, yeGetIntAt(npc_c_sprite['dead-pos'], x_off_idx))
      sprite_man.handlerRefresh(npc)
      return true
   end
   npc.y = LPCS_DEAD
   npc.x = 5
   lpcs.handlerRefresh(npc)
   return true
end

function yGenericHandlerRefresh(npc)
   npc = Entity.wrapp(npc)
   local type = yeGetString(npc.char.type)
   if type == "sprite" then
      sprite_man.handlerRefresh(npc)
   elseif type == "textures-array" then
      ywCanvasRemoveObj(npc.wid, npc.canvas)
      npc.canvas = nil
      refresh_txt_array(npc)
   elseif type ~= "layer-canvas" then
      lpcs.handlerRefresh(npc)
   end
end

function yGenericHandlerNullify(npc)
   npc = Entity.wrapp(npc)
   local type = yeGetString(npc.char.type)
   if type == "sprite" then
      sprite_man.handlerNullify(npc)
   elseif type == "textures-array" then
      ywCanvasRemoveObj(npc.wid, npc.canvas)
      npc.canvas = nil
   elseif type == "layer-canvas" then
      yGenericHandlerRmCanva(npc)
      yeRemoveChild(h, "char");
      yeRemoveChild(h, "cvs_objs");
   else
      lpcs.handlerNullify(npc)
   end
end

function yGenericHandlerPos(npc)
   npc = Entity.wrapp(npc)
   if yIsNil(npc) or npc.char == nil then
      return
   end
   local type = yeGetString(npc.char.type)
   if type == "sprite" then
      return sprite_man.handlerPos(npc)
   elseif type == "layer-canvas" or
      type == "textures-array" then -- "layer-canvas"
      return npc[POS_I]
   else
      return ylpcsHandlerPos(npc)
   end
end

function yGenericHandlerSize(npc)
   npc = Entity.wrapp(npc)
   if yIsNil(npc) or npc.char == nil then
      return
   end
   local type = yeGetString(npc.char.type)
   if type == "sprite" then
      return sprite_man.handlerSize(npc)
   elseif type == "layer-canvas" then
      return ywCanvasObjSize(NULL, npc[CVS_OBJ_I][0])
   elseif type == "textures-array" then
      return ywCanvasObjSize(NULL, npc.canvas)
   else
      return ylpcsHandlerSize(npc)
   end
end

function yGenericNext(npc)
   npc = Entity.wrapp(npc)
   local type = yeGetString(npc.char.type)
   if type == "textures-array" then
      local txts = npc.txts[npc.cur_array:to_string()]
      yeAddInt(npc.cur_txt, 1)
      if npc.cur_txt > yeLen(txts) - 1 then
	 yeSetInt(npc.cur_txt, 0)
      end
      return
   elseif type == "layer-canvas" or
      type == "sprite" then
      return
   end
   ylpcsHandlerNextStep(npc)
end

function yGenericSetAttackPos(npc)
   local type = yeGetString(npc.char.type)
   if type == "layer-canvas" or
      type == "textures-array" or
      type == "sprite" then
      return
   end
   local type = yeGetString(npc.char.type)
   local dir = yeGetInt(npc.y)
   if dir >= LPCS_ATK_UP and dir < LPCS_ATK_DOWN + 1  then
      -- already in atk pos
      return
   end
   dir = dir % 4
   npc.y = LPCS_ATK_UP + dir
end

function yGenericSetDir(npc, dir)
   npc = Entity.wrapp(npc)
   local type = yeGetString(npc.char.type)

   if type == "layer-canvas" or
      type == "textures-array" then
      return
   end

   if yIsLuaString(dir) then
      dir = lpcsStrToDir(dir)
   end

   if type == "sprite" then

      if (yIsLuaNum(dir) == false) then
	 dir = yeGetInt(dir)
      end

      -- uldr = up left down right
      if yeGetString(npc.sp.disposition) == "uldr" then
	 if dir == LPCS_LEFT then
	    yeSetAt(npc, "y_offset", 32)
	 elseif dir == LPCS_RIGHT then
	    yeSetAt(npc, "y_offset", 96)
	 elseif dir == LPCS_DOWN then
	    yeSetAt(npc, "y_offset", 64)
	 else
	    yeSetAt(npc, "y_offset", 0)
	 end
      -- urdl = up right down left
      elseif yeGetString(npc.sp.disposition) == "urdl" then
	 if dir == LPCS_LEFT then
	    yeSetAt(npc, "y_offset", 96)
	 elseif dir == LPCS_RIGHT then
	    yeSetAt(npc, "y_offset", 32)
	 elseif dir == LPCS_DOWN then
	    yeSetAt(npc, "y_offset", 64)
	 else
	    yeSetAt(npc, "y_offset", 0)
	 end
      else
	 if dir == LPCS_LEFT then
	    yeSetAt(npc, "y_offset", 32)
	 elseif dir == LPCS_RIGHT then
	    yeSetAt(npc, "y_offset", 64)
	 elseif dir == LPCS_UP then
	    yeSetAt(npc, "y_offset", 96)
	 else
	    yeSetAt(npc, "y_offset", 0)
	 end
	 print("set offset: ", dir, npc.y_offset);
      end
   else
      lpcs.handlerSetOrigXY(npc, 0, dir)
      yGenericHandlerRefresh(npc)
   end
end

function yGenericSetPos(npc, pos)
   npc = Entity.wrapp(npc)
   local type = yeGetString(npc.char.type)

   if type == "sprite" then
      sprite_man.handlerSetPos(npc, pos)
   elseif type == "layer-canvas" or
      type == "textures-array" then
      -- the pos of each canvas need to be a ref to a pos in the layer-canvas
      ywPosSet(npc[POS_I], pos)
   else
      ylpcsHandlerSetPos(npc, pos)
   end
end

function yGenericHandlerMove(npc, add)
   ywPosAdd(yGenericHandlerPos(npc), add)
end

function yGenericHandlerMoveXY(npc, x, y)
   ywPosAddXY(yGenericHandlerPos(npc), x, y)
end

function modinit_post(mod)
   if yeGetInt(ygGet("mods_config.smart_cobject.no_submodule")) < 1 then
      lpcs = Entity.wrapp(ygGet("lpcs"))
      sprite_man = Entity.wrapp(ygGet("sprite-man"))

      LPCS_LEFT = ygGetInt("lpcs.LEFT")
      LPCS_DOWN = ygGetInt("lpcs.DOWN")
      LPCS_RIGHT = ygGetInt("lpcs.RIGHT")
      LPCS_UP = ygGetInt("lpcs.UP")
      LPCS_DEAD = ygGetInt("lpcs.DEAD")
   else
      LPCS_LEFT = 0
      LPCS_DOWN = 1
      LPCS_RIGHT = 2
      LPCS_UP = 3
      LPCS_DEAD = 4
   end
end

function mod_init(mod)
   mod = Entity.wrapp(mod)

   mod.name = "smart_cobject"
   if yeGetInt(ygGet("mods_config.smart_cobject.no_submodule")) < 1 then
      ygAddModule(Y_MOD_YIRL, mod, "sprite-manager");
      ygAddModule(Y_MOD_YIRL, mod, "Universal-LPC-spritesheet");
   end
   ygRegistreFunc(2, "yGenericTextureArrayCheck", "yGenericTextureArrayCheck");
   ygRegistreFunc(2, "yGenericTextureArraySet", "yGenericTextureArraySet");
   ygRegistreFunc(6, "yGenericNewTexturesArray", "yGenericNewTexturesArray");
   ygRegistreFunc(1, "yGenericHandlerRmCanva", "yGenericHandlerRmCanva");
   ygRegistreFunc(1, "yGenericHandlerShowDead", "yGenericHandlerShowDead");
   ygRegistreFunc(1, "yGenericHandlerRefresh", "yGenericHandlerRefresh");
   ygRegistreFunc(1, "yGenericHandlerNullify", "yGenericHandlerNullify");
   ygRegistreFunc(1, "yGenericHandlerPos", "yGenericHandlerPos");
   ygRegistreFunc(1, "yGenericHandlerSize", "yGenericHandlerSize");
   ygRegistreFunc(2, "yGenericSetDir", "yGenericSetDir");
   ygRegistreFunc(2, "yGenericSetPos", "yGenericSetPos");
   ygRegistreFunc(2, "yGenericUsePos", "yGenericUsePos");
   ygRegistreFunc(2, "yGenericHandlerMove", "yGenericHandlerMove");
   ygRegistreFunc(3, "yGenericHandlerMoveXY", "yGenericHandlerMoveXY");
   ygRegistreFunc(5, "yGenericNewCanvasLayer", "yGenericNewCanvasLayer");
   ygRegistreFunc(1, "yGenericCurCanvas", "yGenericCurCanvas");
   ygRegistreFunc(1, "yGenericSetAttackPos", "yGenericSetAttackPos");
   ygRegistreFunc(1, "yGenericNext", "yGenericNext");
   yeCreateFunction("modinit_post", mod, "init-post-action");
   return mod
end
