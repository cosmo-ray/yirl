local txt_anim_field = Entity.new_string("anim_txt")
local AWAIT_CMD = 0
local PJ_ATTACK = 1
local ENEMY_ATTACK = 2
local ENEMY_WIN = 3
local PJ_WIN = 4
local lpcs = Entity.wrapp(ygGet("lpcs"))
local frm_mult = 10
local good_orig_pos = {1, 1}
local bad_orig_pos = {1, 3}
local combots = nil

function fightAction(entity, eve)
   entity = Entity.wrapp(entity)
   eve = Event.wrapp(eve)
   local ret = YEVE_NOTHANDLE

   if entity.atk_state:to_int() == PJ_WIN or
   entity.atk_state:to_int() == ENEMY_WIN then

      if entity.endCallback then
	 entity.endCallback(entity,
			    entity.endCallbackArg,
			    entity.atk_state:to_int())
      else
	 yCallNextWidget(entity:cent());
      end
      return YEVE_ACTION
   end
   if yDoAnimation(entity, txt_anim_field) == Y_TRUE and
   yHasAnimation(entity, txt_anim_field) == Y_FALSE then
      ret = YEVE_NOACTION
   end
   if entity.atk_state:to_int() == PJ_ATTACK or
   entity.atk_state:to_int() == ENEMY_ATTACK then
      attackCallback(entity, eve)
      return YEVE_ACTION
   end
   while eve:is_end() == false do
      if eve:key() == Y_ESC_KEY then
	 yCallNextWidget(entity:cent());
	 return YEVE_ACTION
      end
      eve = eve:next()
   end
   return ret
end

function menuGetMain(menu)
   return Entity.wrapp(ywCntWidgetFather(ywCntWidgetFather(menu)))
end

function combatDmg(main, cur_anim)
   local canvas = getCanvas(main)
   local dmg = 1

   if cur_anim.mod then
      dmg = cur_anim.mod
   end
   local new_life = cur_anim.target.char.life - dmg
   local max_life = cur_anim.target.char.max_life
   cur_anim.target.char.life = new_life
   local lb = cur_anim.target.life_b
   lb = CanvasObj.wrapp(lb)
   local x = lb:pos():x()
   local y = lb:pos():y()
   canvas:remove(lb.ent)
   cur_anim.target.life_b = canvas:new_rect(x, y, "rgba: 0 255 30 255",
					    Pos.new(50 * new_life / max_life,
						    10).ent).ent
end

function endAnimationAttack(main, cur_anim)
   local obj = CanvasObj.wrapp(cur_anim.guy.canvas)
   local bpos = cur_anim.base_pos

   obj:set_pos(cur_anim.base_pos)
   bpos = Pos.wrapp(bpos)
   ywCanvasObjSetPos(cur_anim.guy.life_b0, bpos:x(), bpos:y() - 25)
   ywCanvasObjSetPos(cur_anim.guy.life_b, bpos:x(), bpos:y() - 25)

   if cur_anim.target.char.life <= 0 then
      if main.atk_state:to_int() == PJ_ATTACK then
	 main.atk_state = PJ_WIN
      else
	 main.atk_state = ENEMY_WIN
      end
      return
   end
   if main.atk_state:to_int() == PJ_ATTACK then
      ylpcsHandlerSetOrigXY(cur_anim.guy, good_orig_pos[1],
			     good_orig_pos[2])
      ylpcsHandlerRefresh(cur_anim.guy)
      main.atk_state = ENEMY_ATTACK
      local tmp = cur_anim.guy
      yeIncrRef(tmp)
      attack(main, cur_anim.target, cur_anim.guy, (yuiRand() % 2) + 1)
      yeDestroy(tmp)
      --print(cur_anim.guy.name, cur_anim.target.name)
   else
      ylpcsHandlerSetOrigXY(cur_anim.guy, bad_orig_pos[1],
			     bad_orig_pos[2])
      ylpcsHandlerRefresh(cur_anim.guy)
      main.atk_state = AWAIT_CMD
      ylpcsHandlerSetOrigXY(cur_anim.guy, bad_orig_pos[1],
			     bad_orig_pos[2])
   end
end

function attackCallback(main, eve)
   local cur_anim = main.attack_info
   local cur_cmb_idx = cur_anim.cur_cmb:to_int()
   local cur_cmb = cur_anim.combots[cur_cmb_idx].touch
   local cur_cmb_anim = cur_anim.combots[cur_cmb_idx].anim
   local canvas = getCanvas(main)
   local tot_bar_len = 30 * cur_cmb:len()
   local last_frm = cur_cmb:len() * frm_mult
   local cur_val_pos = cur_anim.animation_frame / frm_mult
   if cur_val_pos == cur_cmb:len() then
      cur_val_pos = cur_cmb:len() - 1
   end
   local cur_val = cur_cmb[cur_val_pos]:to_int()
   local can_print_loader = true

   cur_anim.animation_frame = cur_anim.animation_frame + 1
   if main.atk_state:to_int() == ENEMY_ATTACK and
   cur_anim.target.char.can_guard:to_int() == 0 then
      can_print_loader = false
   end
   while eve:is_end() == false do
      if eve:type() == YKEY_DOWN and eve:key() == Y_SPACE_KEY or
      eve:type() == YKEY_DOWN and eve:key() == Y_ENTER_KEY then
	 if (cur_val == 1) then
	    cur_anim.sucess = true
	    cur_anim.isPush = 1
	 elseif cur_val == 0 then
	    cur_anim.sucess = false
	 end
      elseif eve:type() == YKEY_UP and eve:key() == Y_SPACE_KEY then
	 cur_anim.isPush = 0
      end
      eve = eve:next()
   end
   if cur_anim.animation_frame:to_int() == 0 then
      local i = 0
      local part_len = tot_bar_len / cur_cmb:len()

      if cur_cmb_anim.to then
	 cur_anim.last_mv_frm = last_frm
	 local bp = Pos.new_copy(ylpcsHandePos(cur_anim.guy))
	 cur_anim.base_pos = bp.ent
	 local tp = Pos.new_copy(ylpcsHandePos(cur_anim.target))
	 if (tp:x() < bp:x()) then
	    tp:add(lpcs.w_sprite:to_int() / 2, 0)
	 else
	    tp:add(-lpcs.w_sprite:to_int() / 2, 0)
	 end
	 cur_anim.to_pos = tp.ent
	 local dis = Pos.new_copy(cur_anim.to_pos)
	 dis:sub(cur_anim.base_pos)
	 cur_anim.mv_per_frm = Pos.new(dis:x() / cur_anim.last_mv_frm,
				       dis:y() / cur_anim.last_mv_frm).ent
      end

      cur_anim.isPush = 0
      cur_anim.loaders = Entity.new_array()
      while i < cur_cmb:len() do
	 local cmb_bar = Entity.new_array()
	 -- print block...
	 cmb_bar[0] = Pos.new(part_len, 15).ent
	 if cur_cmb[i]:to_int() == 1 then
	    cmb_bar[1] = "rgba: 30 30 255 255"
	 elseif cur_cmb[i]:to_int() == 2 then
	    cmb_bar[1] = "rgba: 50 50 127 255"
	 else
	    cmb_bar[1] = "rgba: 255 30 30 255"
	 end
	 if can_print_loader then
	    cur_anim.loaders[i] = canvas:new_rect(25 + (i * part_len),
						  5, cmb_bar).ent
	 end
	 i = i + 1
      end
   end
   if (cur_val == 2 and cur_anim.isPush < 1) then
      cur_anim.sucess = false
      --print("kboum !", cur_val, cur_anim.isPush)
   end
   canvas:remove(cur_anim.loader_percent)
   if cur_cmb_anim.to and
   cur_anim.animation_frame < cur_anim.last_mv_frm then
      local obj = CanvasObj.wrapp(cur_anim.guy.canvas)

      obj:move(cur_anim.mv_per_frm)
      ywCanvasMoveObj(cur_anim.guy.life_b0, cur_anim.mv_per_frm)
      ywCanvasMoveObj(cur_anim.guy.life_b, cur_anim.mv_per_frm)
   end
   if cur_cmb_anim.poses then
      local last = cur_cmb_anim.poses:len()
      local co_pos = cur_anim.animation_frame * last / last_frm
      local cur_orig = Pos.wrapp(cur_cmb_anim.poses[co_pos])

      ylpcsHandlerSetOrigXY(cur_anim.guy, cur_orig:x(), cur_orig:y())
      ylpcsHandlerRefresh(cur_anim.guy)
   end
   if cur_anim.animation_frame >= last_frm then
      local i = 0
      local computer_sucess
	 if (yuiRand() % 2) == 0 then
	     computer_sucess = true
	 else
	    computer_sucess = false
	 end
      while i < cur_cmb:len() do
	 canvas:remove(cur_anim.loaders[i])
	 i = i + 1
      end
      local txt = cur_anim.guy.char.name:to_string() .. " attack: "
      if main.atk_state:to_int() == ENEMY_ATTACK then
	 if cur_anim.sucess:to_int() == 1 then
	    guard_sucess = true
	 else
	    guard_sucess = false
	 end
	 cur_anim.sucess = computer_sucess
      else
	 guard_sucess = computer_sucess
      end
      if cur_anim.target.char.can_guard:to_int() == 0 then
	 guard_sucess = false
      end
      startTextAnim(main, txt)
      if guard_sucess == false then
	 combatDmg(main, cur_anim)
      end

      if cur_anim.sucess:to_int() == 1 then
	 txt = txt .. "SUCESS, " .. cur_anim.target.char.name:to_string() ..
	    " guard: "
	 if cur_anim.target.char.can_guard:to_int() == 0 then
	    txt = txt .. "CAN'T GUARD"
	 elseif guard_sucess then
	    txt = txt .. "SUCESS"
	 else
	    txt = txt .. "FAIL"
	 end
	 cur_anim.sucess = false
	 cur_anim.cur_cmb = cur_anim.cur_cmb + 1
	 if cur_anim.cur_cmb:to_int() < cur_anim.combots:len() then
	    cur_anim.animation_frame = -1
	 else
	    -- deal extra domages if sucess last combot
	    if cur_anim.sucess:to_int() then
	       combatDmg(main, cur_anim)
	    end
	    endAnimationAttack(main, cur_anim)
	 end
      else
	 txt = txt .. "FAIL"
	 endAnimationAttack(main, cur_anim)
      end
      startTextAnim(main, txt)
      return
   end
   if can_print_loader then
      local cmb_bar = Entity.new_array()

      cmb_bar[0] = Pos.new(tot_bar_len *  cur_anim.animation_frame
			      / last_frm, 15).ent
      cmb_bar[1] = "rgba: 0 255 0 50"
      cur_anim.loader_percent = canvas:new_rect(25, 5, cmb_bar).ent
   end
end

function printTextAnim(main, cur_anim)
   main = Entity.wrapp(main)
   cur_anim = Entity.wrapp(cur_anim)
   local canvas = getCanvas(main)

   if cur_anim.animation_frame:to_int() == 0 then
      cur_anim.txt_c = canvas:new_text(50, 50, cur_anim.txt).ent
   end
   if cur_anim.animation_frame >= 30 then
      canvas:remove(cur_anim.txt_c)
      yEndAnimation(main, txt_anim_field)
      return Y_FALSE
   end
   return Y_TRUE
end

function startTextAnim(main, txt)
   local anim = Entity.new_array()
   if main[txt_anim_field:to_string()] then
      local canvas = getCanvas(main)
      canvas:remove(main[txt_anim_field:to_string()].txt_c)
      yEndAnimation(main, txt_anim_field)
   end
   anim.txt = txt
   yInitAnimation(main, anim, Entity.new_func("printTextAnim"),
		  txt_anim_field)
end

function attack(main, attacker, attacked, mod)
   local anim = Entity.new_array()
   anim.sucess = false
   anim.combots = attacker.char.combots
   anim.cmb_len = attacker.char.combots:len()
   anim.cur_cmb = 0
   anim.animation_frame = -1
   anim.mod = mod
   if mod and mod > 1 then
      attacker.char.can_guard = false
   else
      attacker.char.can_guard = true
   end
   anim.guy = attacker
   anim.target = attacked
   main.attack_info = anim
end

function fightAttack(entity, eve)
   local main = menuGetMain(entity)
   main.atk_state = PJ_ATTACK
   return attack(main, main.gg_handler, main.bg_handler)
end

function fightStrongAttack(entity, eve)
   local main = menuGetMain(entity)
   main.atk_state = PJ_ATTACK
   return attack(main, main.gg_handler, main.bg_handler, 2)
end

function newDefaultGuy(guy, name, isEnemy)
   local ret = guy

   if guy.combots == nil then
      local cmb = nil
      print("cmbs:", combots)
      if guy.attack then
	 cmb = combots[guy.attack:to_string()]
      else
	 cmb = combots[0]
      end
      guy.combots = {}
      yeCopy(cmb, guy.combots)
      if isEnemy then
	 local j = 0
	 while j < yeLen(guy.combots[j]) do
	    cmb = guy.combots[j]
	    local i = 0
	    local poses = cmb.anim.poses
	    while i < yeLen(poses) do
	       local c_pos = poses[i]
	       poses[i][1] = poses[i][1] + 2
	       i = i + 1
	    end
	    j = j + 1
	 end
      end
      print("cmb !!!:", guy.combots)
   end
   ret.can_guard = true
   return ret
end

function getCanvas(main)
   return Canvas.wrapp(main.entries[0])
end

function fightInit(entity)
   entity = Entity.wrapp(entity)
   entity.action = Entity.new_func("fightAction")
   entity.background = "rgba: 255 255 255 255"
   entity.current = 1
   entity["turn-length"] = 50000
   entity.entries = {}
   entity.good_guy = newDefaultGuy(entity.player, "the good", false)
   entity.bad_guy = newDefaultGuy(entity.enemy, "the bad", true)
   entity.atk_state = AWAIT_CMD

   local canvas = Entity.new_array(entity.entries)
   canvas["<type>"] = "canvas"
   canvas.background = "rgba: 255 255 0 255"
   canvas.size = 70
   canvas.objs = {}
   local menuCnt = Entity.new_array(entity.entries)
   menuCnt["<type>"] = "container"
   menuCnt.background = "rgba: 255 0 255 255"
   menuCnt.entries = {}
   menuCnt["cnt-type"] = "vertical"
   local menu = Entity.new_array(menuCnt.entries)
   menu["<type>"] = "menu"
   menu.entries = {}
   menu.entries[0] = {}
   menu.entries[0].text = "attack"
   menu.entries[0].action = Entity.new_func("fightAttack")
   menu.entries[1] = {}
   menu.entries[1].text = "strong attack"
   menu.entries[1].action = Entity.new_func("fightStrongAttack")
   local ret = ywidNewWidget(entity, "container")
   local wid_pix = canvas["wid-pix"]
   entity.gg_handler = nil
   entity.bg_handler = nil
   local y_carac = wid_pix.h / 2
   ylpcsCreateHandler(entity.good_guy, canvas, entity, "gg_handler")
   ylpcsHandlerSetOrigXY(entity.gg_handler, 1, 1)
   ylpcsHandlerRefresh(entity.gg_handler)
   ylpcsHandlerMove(entity.gg_handler,
		     Pos.new(wid_pix.w - 100, y_carac).ent)


   ylpcsCreateHandler(entity.bad_guy, canvas, entity, "bg_handler")
   ylpcsHandlerSetOrigXY(entity.bg_handler, 1, 3)
   ylpcsHandlerRefresh(entity.bg_handler)
   ylpcsHandlerMove(entity.bg_handler, Pos.new(50, y_carac).ent)
   canvas = Canvas.wrapp(canvas)

   local life = entity.good_guy.life
   local max_life = entity.good_guy.max_life
   entity.gg_handler.life_b0 = canvas:new_rect(wid_pix.w - 100, y_carac - 25,
					       "rgba: 255 0 30 255",
					       Pos.new(50, 10).ent).ent
   entity.gg_handler.life_b = canvas:new_rect(wid_pix.w - 100, y_carac - 25,
					       "rgba: 0 255 30 255",
					       Pos.new(50 * life / max_life,
						       10).ent).ent

   local life = entity.bad_guy.life
   local max_life = entity.bad_guy.max_life
   entity.bg_handler.life_b0 = canvas:new_rect(50, y_carac - 25,
					       "rgba: 255 0 30 255",
					       Pos.new(50, 10).ent).ent
   entity.bg_handler.life_b = canvas:new_rect(50, y_carac - 25,
					       "rgba: 0 255 30 255",
					       Pos.new(50 * life / max_life,
						       10).ent).ent
   return ret
end

function setCombots(path)
   combots = Entity.wrapp(ygGet(ylovePtrToString(path)))
end

function initFight(mod)
   local init = yeCreateArray()
   yuiRandInit()
   yeCreateString("jrpg-fight", init, "name")
   yeCreateFunction("fightInit", init, "callback")
   ygRegistreFunc(1, "setCombots", "yJrpgFightSetCombots")
   ywidAddSubType(init)
end
