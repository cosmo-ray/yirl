local anim_field = "anim"
local AWAIT_CMD = 0
local PJ_ATTACK = 1
local ENEMY_ATTACK = 2
local lpcs = Entity.wrapp(ygGet("lpcs"))
local frm_mult = 5

function fightAction(entity, eve)
   entity = Entity.wrapp(entity)
   eve = Event.wrapp(eve)

   if yDoAnimation(entity:cent(),
		   Entity.new_string(anim_field):cent(),
		   eve:cent()) == Y_TRUE then
      return YEVE_ACTION
   end
   while eve:is_end() == false do
      if eve:key() == Y_ESC_KEY then
	 yCallNextWidget(entity:cent());
	 return YEVE_ACTION
      end
      eve = eve:next()
   end
   return YEVE_NOTHANDLE
end

function menuGetMain(menu)
   return Entity.wrapp(ywCntWidgetFather(ywCntWidgetFather(menu)))
end

function combatDmg(main, cur_anim)
   cur_anim.target.char.life = cur_anim.target.char.life - 1
end

function endAnimationAttack(main, cur_anim)
   local obj = CanvasObj.wrapp(cur_anim.guy.canvas)

   obj:set_pos(cur_anim.base_pos)

   if main.atk_state:to_int() == PJ_ATTACK then
	 main.atk_state = ENEMY_ATTACK
      	 cur_anim.sucess = false
	 cur_anim.cur_cmb = 0
	 local tmp = cur_anim.guy
	 yeIncrRef(tmp)
	 cur_anim.guy = cur_anim.target
	 cur_anim.target = tmp
	 yeDestroy(tmp)
	 --print(cur_anim.guy.name, cur_anim.target.name)
	 yInitAnimation(main:cent(), cur_anim:cent(),
			Entity.new_func("attackCallback"):cent(),
			Entity.new_string(anim_field):cent())
   else
      main.atk_state = AWAIT_CMD
      yEndAnimation(main:cent(), Entity.new_string(anim_field):cent())
   end
end

function attackCallback(main, cur_anim, eve)
   main = Entity.wrapp(main)
   cur_anim = Entity.wrapp(cur_anim)
   eve = Event.wrapp(eve)
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

   while eve:is_end() == false do
      if eve:type() == YKEY_DOWN and eve:key() == Y_SPACE_KEY then
	 if (cur_val == 1) then
	    cur_anim.sucess = true
	    cur_anim.isPush = 1
	 elseif cur_val == 0 then
	    cur_anim.sucess = false
	 end
	 return YEVE_ACTION
      elseif eve:type() == YKEY_UP and eve:key() == Y_SPACE_KEY then
	 cur_anim.isPush = 0
      end
      eve = eve:next()
   end
   if (cur_val == 2 and cur_anim.isPush < 1) then
      cur_anim.sucess = false
      --print("kboum !", cur_val, cur_anim.isPush)
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
	 print("wololo:", cur_anim.to_pos, cur_anim.base_pos, dis.ent)
      end

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
	 cur_anim.loaders[i] = canvas:new_rect(25 + (i * part_len),
					       5, cmb_bar).ent
	 i = i + 1
      end
   end
   canvas:remove(cur_anim.loader_percent)
   if cur_cmb_anim.to and
   cur_anim.animation_frame < cur_anim.last_mv_frm then
      local obj = CanvasObj.wrapp(cur_anim.guy.canvas)

      obj:move(cur_anim.mv_per_frm)
   end
   if cur_anim.animation_frame >= last_frm then
      local i = 0
      while i < cur_cmb:len() do
	 canvas:remove(cur_anim.loaders[i])
	 i = i + 1
      end
      combatDmg(main, cur_anim)
      if cur_anim.sucess:to_int() == 1 then
	 cur_anim.sucess = false
	 cur_anim.cur_cmb = cur_anim.cur_cmb + 1
	 if cur_anim.cur_cmb:to_int() < cur_anim.combots:len() then
	    yInitAnimation(main:cent(), cur_anim:cent(),
			   Entity.new_func("attackCallback"):cent(),
			   Entity.new_string(anim_field):cent())
	 else
	    endAnimationAttack(main, cur_anim)
	 end
	 print("SUCESS !!!!")
      else
	 print("FAIL !!!!")
	 endAnimationAttack(main, cur_anim)
      end
      return Y_FALSE
   end
   local cmb_bar = Entity.new_array()

   cmb_bar[0] = Pos.new(tot_bar_len *  cur_anim.animation_frame / last_frm, 15).ent
   cmb_bar[1] = "rgba: 0 255 0 50"
   cur_anim.loader_percent = canvas:new_rect(25, 5, cmb_bar).ent
   return Y_TRUE
end

function attack(main, attacker, attacked, mod)
   local anim = Entity.new_array()
   print("ore wa senshi !", main:cent(), attacker.char.combots[0]:len(),
	 attacked.life, mod)
   print("f0: ", anim_field)
   main.atk_state = PJ_ATTACK
   anim.sucess = false
   anim.combots = attacker.char.combots
   anim.cmb_len = attacker.char.combots:len()
   anim.cur_cmb = 0
   anim.mod = mod
   anim.guy = attacker
   anim.target = attacked
   yInitAnimation(main:cent(), anim:cent(),
		  Entity.new_func("attackCallback"):cent(),
		  Entity.new_string(anim_field):cent())
end

function fightAttack(entity, eve)
   local main = menuGetMain(entity)
   return attack(main, main.gg_handeler, main.bg_handeler)
end

function fightStrongAttack(entity, eve)
   print("atack attak attack")
   local main = menuGetMain(entity)
   return attack(main, main.gg_handeler, main.bg_handeler, 2)
end

function newDefaultGuy(name)
   local ret = Entity.new_array()

   ret.name = name
   ret.life = 10
   ret.combots = {}
   ret.combots[0] = {}
   ret.combots[0].anim = {}
   ret.combots[0].touch = { 0, 0, 0, 0, 0, 1, 1 }
   ret.combots[0].anim.to = "target"
   ret.combots[1] = {}
   ret.combots[1].anim = {}
   ret.combots[1].touch = { 0, 1, 1, 2, 2, 2, 2 }
   ret.sex = "female"
   ret.type = "darkelf"
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
   entity["turn-length"] = 30000
   entity.entries = {}
   entity.good_guy = newDefaultGuy("the good")
   entity.bad_guy = newDefaultGuy("the bad")

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
   entity.gg_handeler = nil
   entity.bg_handeler = nil
   ylpcsCreateHandeler(entity.good_guy, canvas, entity, "gg_handeler")
   ylpcsHandelerSetOrigXY(entity.gg_handeler, 1, 1)
   ylpcsHandelerRefresh(entity.gg_handeler)
   ylpcsHandelerMove(entity.gg_handeler,
		     Pos.new(wid_pix.w - 100, wid_pix.h / 2).ent)
   ylpcsCreateHandeler(entity.bad_guy, canvas, entity, "bg_handeler")
   ylpcsHandelerSetOrigXY(entity.bg_handeler, 1, 3)
   ylpcsHandelerRefresh(entity.bg_handeler)
   ylpcsHandelerMove(entity.bg_handeler, Pos.new(50, wid_pix.h / 2).ent)
   return ret
end

function initFight(mod)
   local init = yeCreateArray()
   yeCreateString("jrpg-fight", init, "name")
   yeCreateFunction("fightInit", init, "callback")
   ywidAddSubType(init)
end
