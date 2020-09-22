local txt_anim_field = Entity.new_string("anim_txt")
local AWAIT_CMD = 0
local PJ_ATTACK = 1
local ENEMY_ATTACK = 2
local ENEMY_WIN = 3
local PJ_WIN = 4
local lpcs = Entity.wrapp(ygGet("lpcs"))
local modPath = Entity.wrapp(ygGet("jrpg-fight.$path")):to_string()
local us_per_frm = 130000
local good_orig_pos = {1, 1}
local bad_orig_pos = {1, 3}
local objects = nil
local combots = Entity.wrapp(ygGet("jrpg-fight.game.player.combots"))
local chooseTargetNone = 0
local chooseTargetLeft = 100
local chooseTargetRight = 530
local chooseTargetY = 1
local chooseTargetFunc = nil

local cur_player = 0

local enemy_idx = 0
local enemies_not_on_screen = 0

local LPCS_T = 0
local SPRITES_T = 1

local BAR_PIX_MULT = 30

local is_menu_off = false

local function getCanvas(main)
   return Canvas.wrapp(main.entries[0])
end

local function getMenu(main)
   return main.entries[1].entries[0]
end

local function mk_location(wid_h, start_y)
   local y_carac = wid_h / 2 + start_y
   local base_threshold = 100

   --print("WID H:", wid_h)
   if (wid_h < 500) then base_threshold = 80 end
   return {{y_carac}, {base_threshold + wid_h / 3, base_threshold + wid_h - wid_h / 3},
      {y_carac - base_threshold - 10, y_carac, y_carac + base_threshold}}
end

local function reset_life_b(main, handler, dmg)
   local canvas = getCanvas(main)
   local new_life = handler.char.life - dmg
   local max_life = handler.char.max_life
   local lb = CanvasObj.wrapp(handler.life_b)
   local x = lb:pos():x()
   local y = lb:pos():y()

   canvas:remove(lb.ent)
   if new_life > 0 then
      handler.life_b = canvas:new_rect(x, y, "rgba: 0 255 30 255",
				       Pos.new(50 * new_life / max_life,
					       10).ent).ent
      ywCanvasSetWeight(canvas.ent, handler.life_b, 10);
   end
end

local function  rm_handler(main, h)
   local canvas = getCanvas(main)

   canvas:remove(h.life_b)
   canvas:remove(h.life_b0)
   canvas:remove(h.canvas)
end

local function new_handler(main, guy, y, x, orig)
   local canvas = getCanvas(main)
   local guy = newDefaultGuy(guy, true)
   local bg_h = nil

   if guy.sprite then
      local sp = guy.sprite
      local s = yeGetIntAt(sp, "size")

      bg_h = Entity.new_array()
      bg_h.type = SPRITES_T
      bg_h.canvas = canvas:new_img(x, y, sp.path:to_string(),
				   Rect.new(yeGetIntAt(sp, "x"),
					    yeGetIntAt(sp, "y"),
					    s, s)):cent()
      bg_h.char = guy
      if sp.enlarge then
	 local enlarge = yeGetInt(sp.enlarge)
	 local fsize_pos = Size.new(s * enlarge / 100,
				   s * enlarge / 100).ent

	 ywCanvasForceSize(bg_h.canvas, fsize_pos)
      end
   else
      bg_h = Entity.wrapp(ylpcsCreateHandler(guy, canvas.ent))
      ylpcsHandlerSetOrigXY(bg_h, orig[1], orig[2])
      ylpcsHandlerRefresh(bg_h)
      ylpcsHandlerMove(bg_h, Pos.new(x, y).ent)
      bg_h.type = LPCS_T
   end

   local life = guy.life
   local max_life = guy.max_life
   --print("l ", life, " - ml ", max_life )
   bg_h.life_b0 = canvas:new_rect(x, y - 25,
				  "rgba: 255 0 30 255",
				  Pos.new(50, 10).ent).ent
   bg_h.life_b = canvas:new_rect(x, y - 25,
				 "rgba: 0 255 30 255",
				 Pos.new(50 * life / max_life,
					 10).ent).ent

   return bg_h
end


local function compute_time(frm)
   return frm * us_per_frm
end

local time_acc = 0

local function reset_cmb_bar(main, anim, target, cmb_idx)
   --print(anim.combots)
   local cur_cmb_anim = anim.combots[cmb_idx].anim
   local cur_cmb = anim.combots[cmb_idx].touch
   local tot_bar_len = BAR_PIX_MULT * cur_cmb:len()
   local part_len = tot_bar_len / cur_cmb:len()
   local last_frm = cur_cmb:len()
   local canvas = getCanvas(main)
   local can_print_loader = true

   anim.animation_frame = 0
   if main.atk_state:to_int() == ENEMY_ATTACK and
   target.char.can_guard:to_int() == 0 then
      can_print_loader = false
   end
   anim.last_mv_time = compute_time(last_frm)

   if cur_cmb_anim.to then
      local bp = Pos.wrapp(anim.base_pos)
      local tp = Pos.new_copy(ylpcsHandePos(target))
      if (tp:x() < bp:x()) then
	 tp:add(lpcs.w_sprite:to_int() / 2, 0)
      else
	 tp:add(-lpcs.w_sprite:to_int() / 2, 0)
      end
      anim.to_pos = tp.ent
      local dis = Pos.new_copy(anim.to_pos)
      dis:sub(anim.base_pos)
      anim.to_pos_dis = dis.ent
   end

   anim.isPush = 0
   if can_print_loader then
      anim.loaders = Entity.new_array()
      local i = 0
      while i < cur_cmb:len() do
	 local cmb_bar = Entity.new_array()
	 -- print block...
	 cmb_bar[0] = Pos.new(part_len, 15).ent
	 --print(Pos.new(part_len, 15):tostring(), cmb_bar)
	 if cur_cmb[i]:to_int() == 1 then
	    cmb_bar[1] = "rgba: 30 30 255 255"
	 elseif cur_cmb[i]:to_int() == 2 then
	    cmb_bar[1] = "rgba: 50 50 127 255"
	 else
	    cmb_bar[1] = "rgba: 255 30 30 255"
	 end
	 anim.loaders[i] = canvas:new_rect(25 + (i * part_len),
					   5, cmb_bar).ent
	 i = i + 1
      end
   end
   --print("Can print loader:", can_print_loader, main.atk_state:to_int(),
   --target.char.can_guard:to_int())

   --print(anim.combots)

end

local function attackCallback(main, eve)
   local cur_anim = main.attack_info
   local cur_cmb_idx = cur_anim.cur_cmb:to_int()
   --print(cur_cmb_idx)
   --print("cur_cmb.combots:", cur_anim.combots[cur_cmb_idx]:cent(), cur_cmb_idx)
   local cur_cmb = cur_anim.combots[cur_cmb_idx].touch
   local cur_cmb_anim = cur_anim.combots[cur_cmb_idx].anim
   local canvas = getCanvas(main)
   local tot_bar_len = BAR_PIX_MULT * cur_cmb:len()
   local last_frm = cur_cmb:len()
   local cur_val_pos = cur_anim.animation_frame:to_int()
   local new_frm = false
   local time_diff = ywidTurnTimer()
   local tot_time = cur_anim.last_mv_time:to_int()

   if cur_val_pos == cur_cmb:len() then
      cur_val_pos = cur_cmb:len() - 1
   end

   --print(cur_cmb, cur_val_pos)
   local cur_val = cur_cmb[cur_val_pos]:to_int()
   local can_print_loader = true
   local guy = cur_anim.guy
   local target = cur_anim.target

   time_acc = time_acc + time_diff
   if time_acc > us_per_frm then
      cur_anim.animation_frame = cur_anim.animation_frame + 1
      time_acc = time_acc % us_per_frm
      new_frm = true
   end
   local cur_time = compute_time(cur_anim.animation_frame:to_int()) + time_acc

   if main.atk_state:to_int() == ENEMY_ATTACK and
   target.char.can_guard:to_int() == 0 then
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

   if (cur_val == 2 and cur_anim.isPush < 1) then
      cur_anim.sucess = false
   end

   canvas:remove(cur_anim.loader_percent)
   if cur_cmb_anim.to and cur_time <= tot_time then
      local obj = CanvasObj.wrapp(guy.canvas)
      local tpd = cur_anim.to_pos_dis
      local tpdx = ywPosX(tpd)
      local tpdy = ywPosY(tpd)
      local advance = Pos.new(tpdx * time_diff / tot_time,
			      tpdy * time_diff / tot_time).ent

      obj:move(advance)
      ywCanvasMoveObj(guy.life_b0, advance)
      ywCanvasMoveObj(guy.life_b, advance)

   end

   if cur_cmb_anim.poses then
      local last = cur_cmb_anim.poses:len()
      local co_pos = cur_anim.animation_frame * last / last_frm
      if co_pos == last then
	 co_pos = last - 1
      end
      local cur_orig = Pos.wrapp(cur_cmb_anim.poses[co_pos])

      setOrig(guy, cur_orig:x(), cur_orig:y())
   end

   if cur_anim.animation_frame >= last_frm then

      local computer_sucess
      if (yuiRand() % 2) == 0 then
	 computer_sucess = true
      else
	 computer_sucess = false
      end

      if yIsNNil(cur_anim.loaders) then
	 local i = 0
	 while i < cur_cmb:len() do
	    --print("rm loader: ", cur_anim.loaders)
	    canvas:remove(cur_anim.loaders[i])
	    i = i + 1
	 end
	 cur_anim.loaders = nil
      end

      local txt = guy.char.name:to_string() .. " attack: "
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

      if target.char.can_guard:to_int() == 0 then
	 guard_sucess = false
      end

      startTextAnim(main, txt)
      if guard_sucess == false then
	 combatDmg(main, cur_anim)
      else
	 local p = ylpcsHandePos(target)
	 canvas:remove(main.wrong)
	 main.wrong = canvas:new_texture(ywPosX(p) -5, ywPosY(p),
					 main.wrong_txt).ent
	 main.explosion_time = 5
      end

      if cur_anim.sucess:to_int() == 1 then
	 txt = txt .. "SUCESS, " .. target.char.name:to_string() ..
	    " guard: "
	 if target.char.can_guard:to_int() == 0 then
	    txt = txt .. "CAN'T GUARD"
	 elseif guard_sucess then
	    txt = txt .. "SUCESS"
	 else
	    txt = txt .. "FAIL"
	 end
	 cur_anim.sucess = false
	 cur_anim.cur_cmb = cur_anim.cur_cmb + 1
	 print("NEXT CMB: ", cur_anim.cur_cmb - 1, cur_anim.cur_cmb,
	       cur_anim.combots:len())
	 if cur_anim.cur_cmb:to_int() < cur_anim.combots:len() then
	    reset_cmb_bar(main, cur_anim, target, cur_anim.cur_cmb:to_int())
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
      --print("can_print_loader:", can_print_loader, tot_bar_len * cur_time / tot_time)
      local cmb_bar = Entity.new_array()

      cmb_bar[0] = Pos.new(tot_bar_len * cur_time / tot_time, 15).ent
      cmb_bar[1] = "rgba: 0 255 0 50"
      cur_anim.loader_percent = canvas:new_rect(25, 5, cmb_bar).ent
      ywCanvasSetWeight(canvas.ent, cur_anim.loader_percent, 10);
   end
end

function fightAction(entity, eve)
   entity = Entity.wrapp(entity)
   eve = Event.wrapp(eve)

   --print("Last Turn Length: ", ywidTurnTimer())
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

   if entity.explosion_time then
      entity.explosion_time = entity.explosion_time - 1
      if entity.explosion_time:to_int() == 0 then
	 local canvas = getCanvas(entity)
	 entity.explosion_time = nil
	 canvas:remove(entity.explosion)
	 entity.explosion = nil
	 canvas:remove(entity.wrong)
	 entity.wrong = nil
	 canvas:remove(entity.heart)
	 entity.heart = nil
      end
      return YEVE_ACTION
   end

   if yeGetInt(entity.chooseTarget) > chooseTargetNone then
      return chooseTarget(entity, eve)
   end

   yDoAnimation(entity, txt_anim_field)

   if entity.atk_state:to_int() == PJ_ATTACK or
   entity.atk_state:to_int() == ENEMY_ATTACK then
      attackCallback(entity, eve)
      return YEVE_ACTION
   end

   -- change that to allow esc to quit
   if true then
      while eve:is_end() == false do
	 if eve:key() == Y_ESC_KEY then
	    yCallNextWidget(entity:cent());
	    return YEVE_ACTION
	 end
	 eve = eve:next()
      end
   end

   if yeGetStringAt(entity.gg_handlers[cur_player].char, "atk_mod") ==
   "berserker" then
      local t = entity.bg_handlers[yuiRand() % yeLen(entity.bg_handlers)]
      ywMenuClear(getMenu(entity))
      ywMenuPushEntry(getMenu(entity), "BERSERKERRR")
      attack(entity, entity.gg_handlers[cur_player], t, 3)
      entity.atk_state = PJ_ATTACK
      is_menu_off = true
      return YEVE_ACTION
   end
   print("atk state: ", entity.atk_state:to_int(),
	 entity.gg_handlers[cur_player].char.atk_mod)
   return YEVE_NOTHANDLE
end

local function create_clasic_menu(menu)
   ywMenuClear(menu)
   ywMenuPushEntry(menu, "attack", Entity.new_func(fightAttack))
   ywMenuPushEntry(menu, "strong attack", Entity.new_func(fightStrongAttack))
   ywMenuPushEntry(menu, "recover", Entity.new_func(fightRecover))
   ywMenuPushEntry(menu, "use_items", Entity.new_func(fightItems))
end

function setOrig(handler, x, y)
   if yeGetIntAt(handler, "type") == LPCS_T then
      ylpcsHandlerSetOrigXY(handler, x, y)
      ylpcsHandlerRefresh(handler)
   end
end

function menuGetMain(menu)
   return Entity.wrapp(ywCntWidgetFather(ywCntWidgetFather(menu)))
end


function combatDmgInternal(main, target, dmg)
   local canvas = getCanvas(main)
   local new_life = target.char.life - dmg
   local max_life = target.char.max_life
   local p = ylpcsHandePos(target)

   if new_life > max_life then
      new_life = max_life:to_int()
   end
   if dmg > 0 then
      canvas:remove(main.explosion)
      main.explosion = canvas:new_texture(ywPosX(p),
					  ywPosY(p), main.explosion_txt).ent
      main.explosion_time = 5
   elseif dmg < 0 then
      canvas:remove(main.heart)
      local heart = canvas:new_texture(ywPosX(p) -5, ywPosY(p),
				      main.heart_txt).ent
      main.heart = heart
      main.explosion_time = 5
      ywCanvasSetWeight(canvas.ent, heart, 10);
   end
   target.char.life = new_life
   reset_life_b(main, target, 0)
end

function combatDmg(main, cur_anim)
   local canvas = getCanvas(main)
   local dmg = 1

   if cur_anim.mod then
      dmg = cur_anim.mod
   end
   combatDmgInternal(main, cur_anim.target, dmg)
end

function endAnimationAttack(main, cur_anim)
   local obj = CanvasObj.wrapp(cur_anim.guy.canvas)
   local bpos = cur_anim.base_pos
   local guy = cur_anim.guy
   local is_enemy_next = false
   local bad_guys = main.bg_handlers
   local gg_hs = main.gg_handlers
   local l_gg_h = yeLen(gg_hs)
   local next_guy = cur_anim.target
   local next_target = guy
   local have_win = true

   print("THIS IS THE END")
   obj:set_pos(bpos)
   bpos = Pos.wrapp(bpos)
   ywCanvasObjSetPos(guy.life_b0, bpos:x(), bpos:y() - 25)
   ywCanvasObjSetPos(guy.life_b, bpos:x(), bpos:y() - 25)

   if yIsNNil(cur_anim.loaders) then
      local canvas = getCanvas(main)

      local i = 0
      while i < cur_anim.loaders:len() do
	 canvas:remove(cur_anim.loaders[i])
	 i = i + 1
      end
      cur_anim.loaders = nil
   end
   local have_lose = true
   local players = main.player
   i = 0
   while i < yeLen(players) do
      local p = players[i]
      if p.life > 0 then
	 have_lose = false
      end
      i = i + 1
   end
   if have_lose then
      main.atk_state = ENEMY_WIN
      return
   end

   local enemies = main.enemy
   local nb_enemies = yeLen(enemies)

   if yIsNil(enemies.max_life) then
      local i = 0
      while i < nb_enemies do
	 local i_on_screen = yeGetBoolAt(enemies[i], "on_screen")
	 if enemies[i].life > 0 then
	    have_win = false
	 elseif enemies_not_on_screen > 0 and
	 yeGetBoolAt(enemies[i], "on_screen") then
	    local j = 0
	    while j < nb_enemies do
	       local j_on_screen = yeGetBoolAt(enemies[j], "on_screen")
	       if enemies[j].life > 0 and j_on_screen == false then
		  local screen_idx = yeGetIntAt(enemies[i], "screen_idx")
		  local y = ywPosY(ylpcsHandePos(bad_guys[screen_idx]))

		  rm_handler(main, bad_guys[screen_idx])
		  bad_guys[screen_idx] = new_handler(main, enemies[j], y, 50,
						     bad_orig_pos)
		  print(j, " replace ", i, "enemies left:", enemies_not_on_screen)
		  enemies[j].screen_idx = screen_idx
		  enemies[j].on_screen = true
		  enemies[i].on_screen = false
		  reset_life_b(main, bad_guys[screen_idx], 0)
		  enemies_not_on_screen = enemies_not_on_screen - 1
		  have_win = false
		  break
	       end
	       print(j, enemies[j].life,
		     yeGetBoolAt(enemies[j], "on_screen"))
	       j = j + 1
	    end
	 elseif i_on_screen then -- he's dead, he ceased to exist, he's no more
	    local screen_idx = yeGetIntAt(enemies[i], "screen_idx")
	    rm_handler(main, bad_guys[screen_idx])
	 end
	 i = i + 1
      end
   elseif enemies.life > 0 then
      have_win = false
   end

   if have_win then
      print("WIN !")
      main.atk_state = PJ_WIN
      return
   end

   if main.atk_state:to_int() == PJ_ATTACK and
   cur_player < l_gg_h - 1 then
      print('stuff: ', cur_player, l_gg_h)
      cur_player = cur_player + 1
   elseif main.atk_state:to_int() == PJ_ATTACK or
   yeLen(bad_guys) > enemy_idx then
      local all_dead = true
      cur_player = 0
      while (yeLen(bad_guys) > enemy_idx) do
	 if bad_guys[enemy_idx].char.life > 0 then
	    all_dead = false
	    break
	 end
	 enemy_idx = enemy_idx + 1
      end
      if all_dead == false then
	 local gg_hs = main.gg_handlers
	 next_guy = bad_guys[enemy_idx]
	 next_target = gg_hs[yuiRand() % l_gg_h]
	 enemy_idx = enemy_idx + 1
	 is_enemy_next = true
      else
	 enemy_idx = 0
      end
   else
      enemy_idx = 0
   end

   if is_enemy_next then
      if main.atk_state:to_int() == ENEMY_ATTACK then
	 setOrig(guy, bad_orig_pos[1], bad_orig_pos[2])
      else
	 setOrig(guy, good_orig_pos[1], good_orig_pos[2])
	 main.atk_state = ENEMY_ATTACK
      end
      local r = 0
      if cur_anim.target.char.life < (cur_anim.target.char.max_life / 2) then
	 r = yuiRand() % 3
      else
	 r = yuiRand() % 2
      end
      if r < 2 then
	 local tmp = guy
	 yeIncrRef(tmp)
	 attack(main, next_guy, next_target, (yAnd(r, 1)) + 1)
	 yeDestroy(tmp)
      else
	 fightRecoverInternal(main, next_guy, next_target)
      end
      --print(cur_anim.guy.name, cur_anim.target.name)
   else
      print("set orig not enemy next")
      if cur_player == 0 then
	 setOrig(guy, bad_orig_pos[1], bad_orig_pos[2])
      else
	 setOrig(guy, good_orig_pos[1], good_orig_pos[2])
      end
      main.atk_state = AWAIT_CMD
      if is_menu_off then
	 create_clasic_menu(getMenu(main))
	 is_menu_off = 0
      end
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

function mk_anim(main, guy, target)
   local anim = Entity.new_array()
   local bp = Pos.new_copy(ylpcsHandePos(guy))

   anim.base_pos = bp.ent
   anim.guy = guy
   anim.target = target
   anim.animation_frame = 0
   return anim
end

function attack(main, attacker, attacked, mod)
   local anim = mk_anim(main, attacker, attacked)

   print("ATTACK ATTACK ATTACK")
   anim.sucess = false
   anim.combots = attacker.char._combots
   anim.cmb_len = attacker.char._combots:len()
   anim.cur_cmb = 0
   anim.mod = mod
   if mod and mod > 1 then
      attacker.char.can_guard = false
   else
      attacker.char.can_guard = true
   end
   main.attack_info = anim
   reset_cmb_bar(main, anim, attacked, 0)
   return anim
end

function fightAttack(entity, eve)
   local main = menuGetMain(entity)
   chooseTargetLoc(main, chooseTargetLeft,
		   yeLen(main.bg_handlers), chooseTargetY)
   chooseTargetFunc = attack
   return YEVE_ACTION
end

local function strong_attack(main, guy, oguy)
   return attack(main, guy, oguy, 2)
end

function fightStrongAttack(entity, eve)
   local main = menuGetMain(entity)
   chooseTargetLoc(main, chooseTargetLeft,
		   yeLen(main.bg_handlers), chooseTargetY)
   chooseTargetFunc = strong_attack
   return YEVE_ACTION
end

function fightRecoverInternal(main, guy, target)
   local anime = attack(main, guy, target)
   local heal = 1 + yeGetInt(guy.char.recover_level)

   combatDmgInternal(main, guy, -heal)
   endAnimationAttack(main, anime)
end

function fightRecover(entity, eve)
   local main = menuGetMain(entity)

   main.atk_state = PJ_ATTACK
   fightRecoverInternal(main, main.gg_handlers[cur_player],
			main.bg_handlers[cur_player])
   return YEVE_ACTION
end

function useItem(main, user, target)
   local item = main.inUseItem
   local stPlus = Entity.wrapp(yeGet(item, "stats+"))
   local dmg = Entity.wrapp(yeGet(item, "dmg"))
   local hasAction = false
   local cin = main.cur_item_nb

   yeSetInt(cin, cin:to_int() - 1)

   if stPlus then
      local i = 0
      while i < yeLen(stPlus) do
	 if yeGetKeyAt(stPlus, i) == "life" then
	    combatDmgInternal(main, target, -stPlus[i]:to_int())
	 end
	 i = i + 1
      end
      hasAction = true
   end
   if dmg then
      combatDmgInternal(main, target, dmg:to_int())
   end
   local anime = mk_anim(main, user, main.bg_handlers[cur_player])

   endAnimationAttack(main, anime)
   return YEVE_ACTION
end

function chooseTargetLoc(main, side, nb_handles, y)
   local canvas = getCanvas(main)
   local location = mk_location(canvas.ent["wid-pix"].h,
				yeGetInt(main.ychar_start))
   local arrow = nil

   if (main.chooseTargetArrow) then
      canvas:remove(main.chooseTargetArrow)
   end

   if side == chooseTargetLeft then
      arrow = Entity.new_string("<--")
   else
      arrow = Entity.new_string("-->")
   end

   main.chooseTarget = side
   main.chooseTargetArrow = canvas:new_text(side,
					    location[nb_handles][y],
					    arrow).ent
   return YEVE_ACTION
end

function checkCanChooseTarget(main, nb_enemy)
   return main.chooseTarget:to_int() == chooseTargetLeft and nb_enemy > 1
end

function chooseTarget(main, eve)
   local canvas = getCanvas(main)
   local location = mk_location(canvas.ent["wid-pix"].h,
				yeGetInt(main.ychar_start))
   local nb_enemy = yeLen(main.bg_handlers)

   while eve:is_end() == false do
      if eve:is_key_left() then
	 if nb_enemy == 3 then
	    chooseTargetY = 2
	 else
	    chooseTargetY = 1
	 end
	 return chooseTargetLoc(main, chooseTargetLeft, nb_enemy, chooseTargetY)
      elseif eve:is_key_right() then
	 return chooseTargetLoc(main, chooseTargetRight, 1, 1)
      elseif eve:type() == YKEY_DOWN and eve:is_key_up() and
      checkCanChooseTarget(main, nb_enemy) then
	 chooseTargetY = chooseTargetY - 1
	 if chooseTargetY < 1 then
	    chooseTargetY = nb_enemy
	 end
	 return chooseTargetLoc(main, chooseTargetLeft, nb_enemy, chooseTargetY)
      elseif eve:type() == YKEY_DOWN and eve:is_key_down() and
      checkCanChooseTarget(main, nb_enemy) then
	 chooseTargetY = chooseTargetY + 1
	 if chooseTargetY > nb_enemy then
	    chooseTargetY = 1
	 end
	 return chooseTargetLoc(main, chooseTargetLeft, nb_enemy, chooseTargetY)
      elseif eve:type() == YKEY_UP and eve:key() == Y_ESC_KEY then
	 goto clean
      elseif eve:type() == YKEY_DOWN and eve:key() == Y_ENTER_KEY then
	 local target = nil
	 if main.chooseTarget:to_int() == chooseTargetLeft then
	    target = main.bg_handlers[chooseTargetY - 1]
	 else
	    target = main.gg_handlers[0]
	 end
	 chooseTargetFunc(main, main.gg_handlers[cur_player], target)
	 main.atk_state = PJ_ATTACK
	 goto clean
      end
      eve = eve:next()
   end

   -- without the if I got an error with the label
   if true then return YEVE_NOTHANDLE end

   :: clean ::
   canvas:remove(main.chooseTargetArrow)
   main.chooseTargetArrow = nil
   main.chooseTarget = chooseTargetNone
   main.inUseItem = nil
   main.cur_item_nb = nil
   return YEVE_ACTION
end

function useItemCallback(menu, eve)
   local main = menuGetMain(menu)
   local curItem = Entity.wrapp(ywMenuGetCurrentEntry(menu))

   if curItem.it_nb < 1 then
      return useItemBack(menu)
   end

   local item = objects[curItem.it_name:to_string()]
   local canvas = getCanvas(main)

   --local ret = useItem(main, item, main.gg_handler)
   if yeGetString(item.default_target) == "enemy" then
      chooseTargetLoc(main, chooseTargetLeft, 1, 1)
   else
      chooseTargetLoc(main, chooseTargetRight, 1, 1)
   end
   chooseTargetFunc = useItem
   main.inUseItem = item
   yeReplaceBack(main, curItem.it_nb, "cur_item_nb")
   useItemBack(menu)
   return ret
end

function useItemBack(menu)
   local mnCnt = ywCntWidgetFather(menu)
   ywCntPopLastEntry(mnCnt)
   return YEVE_ACTION
end

function fightItems(entity, func)
   local main = menuGetMain(entity)
   local pc = main.gg_handler.char
   local menuCnt = ywCntWidgetFather(entity)
   local itemsMenu = Menu.new_entity().ent

   yeGetPush(menuCnt, itemsMenu, "background");
   local ui = pc.usable_items
   local i = 0
   ywMenuPushEntry(itemsMenu, "<-- back", Entity.new_func("useItemBack"))

   while i < yeLen(ui) do
      local nb_i_ent = ui[i]
      local nb_i = math.floor(yeGetInt(nb_i_ent))
      local item = objects[yeGetKeyAt(ui, i)]
      local entry = ywMenuPushEntry(itemsMenu,
				    yeGetKeyAt(ui, i) .. ": " .. nb_i,
				    Entity.new_func("useItemCallback"))
      entry = Entity.wrapp(entry)
      entry.it_name = yeGetKeyAt(ui, i)
      yePushBack(entry, nb_i_ent, "it_nb")
      i = i + 1
   end
   ywPushNewWidget(menuCnt, itemsMenu);
end

function newDefaultGuy(guy, isEnemy)
   local ret = guy

   if guy._combots == nil then
      local cmb = nil
      if guy.attack then
	 cmb = combots[guy.attack:to_string()]
      elseif yIsNNil(combots[0].touch) then
	 cmb = combots
      else
	 cmb = combots[0]
      end

      guy._combots = {}
      yeCopy(cmb, guy._combots)
      if isEnemy then
	 local j = 0
	 while j < yeLen(guy._combots[j]) do
	    cmb = guy._combots[j]
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
   end
   ret.can_guard = true
   return ret
end

function fightKboum(ent)
   enemy_idx = 0
   chooseTargetY = 1
end

function try_mk_array_of_guys(guys)
   if yIsNil(guys.max_life) then
      -- ifc max_life is not present, we assume it's an array
      return guys
   end
   local ret = Entity.new_array()

   ret[0] = guys
   return ret
end

function fightInit(entity)
   entity = Entity.wrapp(entity)
   entity.action = Entity.new_func("fightAction")
   entity.destroy = Entity.new_func("fightKboum")
   entity.background = "rgba: 255 255 255 255"
   entity.current = 1
   entity["turn-length"] = Y_REQUEST_ANIMATION_FRAME
   entity.entries = {}
   entity.atk_state = AWAIT_CMD
   ywTextureNewImg(modPath .. "/explosion.png",
		   Rect.new(512 + 45, 32, 64, 64).ent,
		   entity, "explosion_txt")
   ywTextureNewImg(modPath .. "/image0009.png",
		   nil,  entity, "wrong_txt")
   ywTextureNewImg(modPath .. "/image0007.png",
		   nil,  entity, "heart_txt")
   objects = Entity.wrapp(ygGet("jrpg-fight:objects"))

   local canvas = Entity.new_array(entity.entries)
   canvas["<type>"] = "canvas"
   --canvas.background = "\"" .. modPath .. "BG City.jpg\""
   canvas.size = 75
   canvas.objs = {}
   local menuCnt = Entity.new_array(entity.entries)
   menuCnt["<type>"] = "container"
   menuCnt.background = "rgba: 255 0 255 255"
   menuCnt.entries = {}
   menuCnt["cnt-type"] = "vertical"
   local menu = Entity.new_array(menuCnt.entries)
   menu["<type>"] = "menu"
   create_clasic_menu(menu)

   local ret = ywidNewWidget(entity, "container")
   ywCanvasEnableWeight(canvas)
   local wid_pix = canvas["wid-pix"]
   entity.gg_handlers = nil
   entity.bg_handlers = nil
   local wid_h = wid_pix.h
   local y_start = yeGetInt(entity.ychar_start)
   if y_start > 0 then
      local wid_h = wid_pix.h - y_start
   end
   local y_carac = wid_h / 2 + y_start
   local locations = mk_location(wid_h, y_start)

   local bg = ywCanvasNewImg(canvas, 0, 0, modPath .. "BG_City.jpg")
   ywCanvasForceSize(bg, Size.new(ywRectW(wid_pix) + 50,
				  ywRectH(wid_pix) + 50).ent)

   local good_guys = try_mk_array_of_guys(entity.player)
   entity.player = good_guys
   local gg_handlers = Entity.new_array(entity, "gg_handlers")
   entity.gg_handlers = gg_handlers

   local i = 0
   local nb_gg = yeLen(good_guys)
   while i < nb_gg do
      local good_guy = newDefaultGuy(good_guys[i], false)
      local y = locations[nb_gg][i + 1]

      local gg_h = new_handler(entity, good_guy, y, wid_pix.w - 100,
			       good_orig_pos)
      gg_handlers[i] = gg_h
      i = i + 1
   end

   chooseTargetRight = wid_pix.w - 130

   local bad_guys = try_mk_array_of_guys(entity.enemy)
   entity.enemy = bad_guys

   i = 0
   local nb_bg = yeLen(bad_guys)
   if nb_bg > 3 then
      enemies_not_on_screen = nb_bg - 3
      nb_bg = 3
   else
      enemies_not_on_screen = 0
   end
   canvas = Canvas.wrapp(canvas)
   local bg_handlers = Entity.new_array(entity, "bg_handlers")
   while i < nb_bg do
      local bad_guy = newDefaultGuy(bad_guys[i], true)
      local y = locations[nb_bg][i + 1]
      local bg_h = new_handler(entity, bad_guy, y, 50, bad_orig_pos)

      bad_guy.screen_idx = i
      bad_guy.on_screen = true
      bg_handlers[i] = bg_h
      i = i + 1
   end

   -- if I want to implement initiative, I need to change it here
   cur_player = 0
   return ret
end

function setCombots(path)
   combots = Entity.wrapp(ygGet(ylovePtrToString(path)))
end

function getWinner(wid, id)
   if id == PJ_WIN then
      return yeGet(wid, "enemy");
   else
      return yeGet(wid, "player");
   end
end

function getLooser(wid, id)
   if id == PJ_WIN then
      return yeGet(wid, "player");
   else
      return yeGet(wid, "enemy");
   end
end

function initFight(mod)
   local init = yeCreateArray()
   yuiRandInit()
   yeCreateString("jrpg-fight", init, "name")
   yeCreateFunction("fightInit", init, "callback")
   ygRegistreFunc(1, "setCombots", "yJrpgFightSetCombots")
   ygRegistreFunc(1, "getWinner", "yJrpgGetWinner")
   ygRegistreFunc(1, "getLooser", "yJrpgGetLooser")
   ywidAddSubType(init)
end
