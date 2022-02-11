--
--Copyright (C) 2021 Matthias Gatto
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

local loading_bar = nil
local loading_atk = 0
local loading_atk_per_turn = 2

local modPath = nil
local upKeys = nil
local downKeys = nil
local leftKeys = nil
local rightKeys = nil

local fire_threshold = 0
local up_threshold = 0

local ASTEROID_SHOOTER = 0
local AXEMAN_SHOOTER = 1

local version = ASTEROID_SHOOTER

local axeman_left = 585
local axeman_right = 715
local axeman_up = 522
local axeman_down = 650

local STEP_X_THRESHOLD = 64
local step_cnt = 0
local STEP_NB_SPRITE = 9

local sprite_y = axeman_left

local TURN_LENGTH = 50000

local CCK_SIZE_X = 60
local CCK_SIZE_Y = 100

local level_gap = 2
local next_level = 0

local enemy_apparition = 0

local shojou_attack = 0
local shojou_lvl = 0

local old_tl = 0

local function destroy()
   ywSetTurnLengthOverwrite(old_tl);
end

local function push_enemy(canvas, x, y, a, speed, img)
   local ent = canvas.ent
   local bigAst = nil
   if img == nil then
      bigAst = canvas:new_obj(x, y, 1)
   else
      bigAst = canvas:new_img(x, y, img)
   end

   bigAst.ent.life = 1
   bigAst.ent.speed = speed
   yeCreateFloat(a, bigAst.ent:cent(), "angle")
   if version == AXEMAN_SHOOTER then
      local s = Size.new(CCK_SIZE_X, CCK_SIZE_Y)
      bigAst:force_size(s)
   end
   ent.asteroides:push_back(bigAst:cent())
   return bigAst
end

local function increase_atk_speed()
   print("increase_atk_speed")
   loading_atk_per_turn = loading_atk_per_turn + 2
end

local function attack_nb_1()
   print("recive toss, spike")
   shojou_lvl = shojou_lvl + 1
end

function action(entity, eve)
   local canvas = Canvas.wrapp(entity)
   local ent = canvas.ent
   local move = ent.move
   local ship = CanvasObj.wrapp(ent.ship)

   if yevIsKeyDown(eve, Y_ESC_KEY) then
      if ent.quit then
	 ent.quit(ent)
	 return YEVE_ACTION
      end
      yFinishGame()
      return YEVE_ACTION
   end
   if yevIsGrpDown(eve, upKeys) then move.up_down = -1
   elseif yevIsGrpDown(eve, downKeys) then move.up_down = 1
   end
   if yevIsGrpDown(eve, leftKeys) then move.left_right = -1
   elseif yevIsGrpDown(eve, rightKeys) then move.left_right = 1
   end
   if yevIsGrpUp(eve, upKeys) or yevIsGrpUp(eve, downKeys) then
      move.up_down = 0
   end
   if yevIsGrpUp(eve, leftKeys) or yevIsGrpUp(eve, rightKeys) then
      move.left_right = 0
   end
   local pos = yevMousePos(eve)

   if version == AXEMAN_SHOOTER and (yeGetInt(move.up_down) ~= 0 or yeGetInt(move.left_right) ~= 0) then
      step_cnt = step_cnt + 1
      step_cnt = step_cnt % STEP_NB_SPRITE
   end

   if pos then
      -- I should add an api for that :p
      pos = Pos.wrapp(pos)
      if version ~= AXEMAN_SHOOTER then
	 ship:point_top_to(pos)
      else
	 local a = ywPosAngle(ship:pos().ent, pos.ent)
	 local aa = math.abs(a)

	 if (aa > 45 and aa < 135) then
	    if a > 0 then
	       sprite_y = axeman_up
	    else
	       sprite_y = axeman_down
	    end
	 else
	    if (aa > 90) then
	       sprite_y = axeman_right
	    else
	       sprite_y = axeman_left
	    end
	 end
      end
   end

   if version == AXEMAN_SHOOTER then
      local r = Rect.new(step_cnt * STEP_X_THRESHOLD,
			 sprite_y, STEP_X_THRESHOLD, 60).ent

      canvas:remove(ent.ship)
      local sp = ship:pos()
      ent.ship = canvas:new_img(sp:x(), sp:y(), modPath .. "/axeman.png", r).ent
      ship = CanvasObj.wrapp(ent.ship)
   end

   shojou_attack = shojou_attack - 1
   if shojou_lvl > 0 and shojou_attack < 1 then
      local time_sub = shojou_lvl
      if time_sub > 40 then
	 time_sub = 40
      end
      local nb_atk = 1 + shojou_lvl / 4
      for i = 1, nb_atk do
	 local laser = canvas:new_obj(ship:pos():x() + ship:size():x() / 2 - fire_threshold,
				      ship:pos():y() + ship:size():y() / 2  - fire_threshold, 2)

	 local angle = yuiRand() % 360 - 180 
	 yeCreateFloat(laser:angle() + angle, laser.ent:cent(), "angle")
	 ent.lasers:push_back(laser:cent())
      end
      shojou_attack = 45 - shojou_lvl
   end

   local ret, button = yevMouseDown(eve, button);
   if ret and (version == ASTEROID_SHOOTER or loading_atk > 99 ) then
      local laser = canvas:new_obj(ship:pos():x() + ship:size():x() / 2 - fire_threshold,
				      ship:pos():y() + ship:size():y() / 2  - fire_threshold, 0)

      local angle = -90
      if version == ASTEROID_SHOOTER and button == 3 then
	 angle = 90
      else
	 loading_atk = 0
      end
      laser:point_right_to(pos)
      yeCreateFloat(laser:angle() + angle, laser.ent:cent(), "angle")
      ent.lasers:push_back(laser:cent())
   end

   local lasers = ent.lasers
   local asteroides = ent.asteroides
   for i = 0, lasers:len() do
      if lasers[i] then
	 local laser = CanvasObj.wrapp(lasers[i])

	 laser:advance(25, laser.ent.angle:to_float())
	 if (canvas:is_out(laser) == 1) then
	    removeObj(canvas, lasers, laser)
	    laser = nil
	 else
	    local ast_l = asteroides:len()
	    for i = 0, ast_l do

	       if (asteroides[i]) and laser:colide_with(asteroides[i]) then
		  ent.score = canvas.ent.score + 1

		  if ent.score > next_level and yIsNNil(ent.lvlup) then

		     level_gap = level_gap + 1
		     next_level = next_level + level_gap

		     local lvl_event = ent.lvlup[yuiRand() % yeLen(ent.lvlup)]
		     local base_path = ""
		     local bmp = lvl_event["base-mod-path"]
		     if yIsNNil(bmp) then
			base_path = yeGetStringAt(ygGet(yeGetString(bmp)), "$path")
		     end
		     local lvl_img = base_path .. yeGetString(lvl_event.img)
		     local c = nil

		     if yIsNNil(lvl_img) then
			c = canvas:new_img(0, 0, lvl_img)
		     end
		     ywidActions(ent, lvl_event, nil)

		     if yIsNNil(lvl_img) then
			ygUpdateScreen()
			local events = ywidGenericPollEvent()
			local mouse_down = false

			while yevIsKeyDown(events, Y_ENTER_KEY) == false and
			   mouse_down == false do
			   events = ywidGenericPollEvent()
			   mouse_down, button = yevMouseDown(events, button)
			end

			canvas:remove(c)
		     end
		  end

		  asteroides[i].life = asteroides[i].life:to_int() - 1
		  canvas:remove(ent.score_canvas)

		  ent.score_canvas =
		     canvas:new_text(10, 10,
				     Entity.new_string("score: "..
						       math.floor(ent.score:to_int()))):cent()
		  if asteroides[i].life:to_int() == 0 then
		     local ast_i = CanvasObj.wrapp(asteroides[i])
		     if version == AXEMAN_SHOOTER then
			local astip = ast_i:pos()
			local exp = Entity.new_array(ent.explosion)
			local exp_c = canvas:new_img(astip:x(), astip:y(), modPath .. "/boum_0.png")

			exp[0] = 0
			exp[1] = exp_c.ent
			local s = Size.new(CCK_SIZE_X, CCK_SIZE_Y)
			exp_c:force_size(s)
		     end
		     removeObj(canvas, asteroides, ast_i)
		  elseif version == ASTEROID_SHOOTER then
		     if (asteroides[i].speed < 30) then
			asteroides[i].speed = asteroides[i].speed:to_int() + 1
		     end
		     asteroides[i].angle:set_float(laser.ent.angle:to_float())
		     local p = CanvasObj.wrapp(asteroides[i]):pos()
		     local bigAst = push_enemy(canvas, p:x(), p:y(),
					       -asteroides[i].angle:to_float(),
					       asteroides[i].speed)

		     --bigAst:force_size(Pos:new(20, 20))
		     if asteroides[i].life < 0 then
			bigAst.ent.life = ent.score:to_int() / 10 + 1
		     elseif asteroides[i].life > 1 then
			bigAst.ent.life = asteroides[i].life - 1
		     else
			bigAst.ent.life = 1
		     end
		  else
		     local ast_i = CanvasObj.wrapp(asteroides[i])
		     local astip = ast_i:pos()
		     local life = asteroides[i].life
		     local ne = push_enemy(canvas, astip:x(), astip:y(), ywPosAngle(astip.ent,
										    ship:pos().ent) - 90,
					   
					   ast_i.ent.speed + 1,
					   modPath .. "z-cvck-hurt.png")
		     ne.ent.life = life
		     removeObj(canvas, asteroides, ast_i)
		  end
		  removeObj(canvas, lasers, laser)
		  break;
	       end -- laser colide with enemy
	    end -- foreach enemies
	 end -- laser not out
      end -- lasers[i]
   end -- for laser ?

   if version == AXEMAN_SHOOTER then
      for i = 0, yeLen(ent.explosion) - 1 do
	 local exp = ent.explosion[i]
	 local expip = nil
	 local img_path = nil
	 local exp_c = nil
	 local s = nil
	 if yIsNil(exp) then
	    goto next_
	 end

	 expip = CanvasObj.wrapp(exp[1]):pos()

	 if exp[0] > 10 then
	    canvas:remove(exp[1])
	    ent.explosion:remove(exp)
	    goto next_
	 elseif exp[0] > 3 and exp[0] < 5 then
	    img_path = modPath .. "/boum_1.png"
	 elseif exp[0] > 6 and exp[0] < 8 then
	    img_path = modPath .. "/boum_2.png"
	 else
	    goto incr
	 end
	 exp_c = canvas:new_img(expip:x(), expip:y(), img_path)
	 canvas:remove(exp[1])
	 s = Size.new(CCK_SIZE_X, CCK_SIZE_Y)
	 exp_c:force_size(s)
	 exp[1] = exp_c.ent
	 :: incr ::
	 exp[0] = exp[0] + 1
	 :: next_ ::
      end
end
   enemy_apparition = enemy_apparition - 1
   if version == AXEMAN_SHOOTER and enemy_apparition < 0 then
      local screen_side = yuiRand() % 4
      local score_sub = canvas.ent.score:to_int()
      local life = 2
      local speed = 6

      if score_sub > 295 then
	 score_sub = score_sub - 302
	 life = 4
	 speed = 20
      elseif score_sub > 195 then
	 score_sub = score_sub - 205
	 life = 3
	 speed = 12
      elseif score_sub > 100 then
	 score_sub = score_sub - 110
	 life = 3
	 speed = 7
      end
      enemy_apparition = 100 - score_sub

      local x = 0
      local y = 0
      if screen_side < 2 then
	 x = yuiRand() % canvas.ent['wid-pix'].w:to_int()
	 if screen_side == 0 then
	    y = canvas.ent['wid-pix'].h - 20
	 end
      else
	 y = yuiRand() % canvas.ent['wid-pix'].h:to_int()
	 if screen_side == 3 then
	    x = canvas.ent['wid-pix'].w - 20
	 end
      end

      local p = Pos.new(x, y)
      local en = push_enemy(canvas, x, y, ywPosAngle(p.ent, ship:pos().ent) - 90, speed)
      en.ent.life = life
   end

   for i = 0, asteroides:len() do
      if (asteroides[i]) then
	 --print(asteroides[i].speed, asteroides[i].angle)
	 local ast = CanvasObj.wrapp(asteroides[i])
	 ast:advance(ast.ent.speed:to_int(), ast.ent.angle:to_float())
	 if asteroides[i] and ship:colide_with(asteroides[i]) then
	    if canvas.ent.die then
	       canvas.ent.die(canvas.ent, canvas.ent.score:to_int())
	    elseif canvas.ent.next then
	       ywidNext(canvas.ent.next)
	    else
	       yFinishGame()
	    end
	    return YEVE_ACTION
	 end

	 if canvas:is_out(ast) == 1 then
	    ast.ent.angle:set_float(ast.ent.angle:to_float() + 90 +
				       (yuiRand() % 70))
	    ast:advance(ast.ent.speed:to_int(), ast.ent.angle:to_float())
	 end
      end
   end

   local pos = Pos.new(move.left_right * 10, move.up_down * 10)
   local sp = ship:pos()
   local size = ship:size()
   ship:move(pos)
   if (sp:x() < 0 or sp:x() + size:x() > canvas.ent['wid-pix'].w
       or sp:y() < 0 or sp:y() + size:y() > canvas.ent['wid-pix'].h) then
      ship:move(Pos.new(-pos:x(), -pos:y()))
   end

   if version == AXEMAN_SHOOTER then
      loading_bar.setPercent(canvas.ent.lb, loading_atk)
      loading_atk = loading_atk + loading_atk_per_turn
      if loading_atk > 100 then
	 loading_atk = 100
      end
      
   end
   return YEVE_ACTION
end

function removeObj(wid, container, obj)
   wid:remove(obj)
   container:remove(obj.ent)
end

function createAstShoot(entity)
   local canvas = Canvas.wrapp(entity)
   local ent = canvas.ent
   local enemy_img_path = modPath .. "jswars_gfx/asteroid.png"

   yuiRandInit()
   ent.resources = {}
   ent.resources[0] = {}
   local resource = ent.resources[0]
   if yeGetString(ent.type) == "skull-breaker" then 
      resource["img"] = modPath .. "axe_10.png"
      resource["img-src-rect"] = {0, 615, 20, 20}
      fire_threshold = 20
      up_threshold = -90
      version = AXEMAN_SHOOTER
      enemy_img_path = modPath .. "z-cvck.png"
   else
      resource["img"] = modPath .. "jswars_gfx/shot.png"
      fire_threshold = 5
      up_threshold = 90
   end
   ent.resources[1] = {}
   resource = ent.resources[1]
   resource["img"] = enemy_img_path

   ent.resources[2] = {}
   resource = ent.resources[2]
   resource["img"] = modPath .. "Tennisball.png"

   Entity.new_func("action", ent, "action")
   Entity.new_func(destroy, ent, "destroy")
   canvas.ent.background = "rgba: 255 255 255 255"
   local ship
   if version == AXEMAN_SHOOTER then
      ship = canvas:new_img(150, 150, modPath .. "/axeman.png", Rect.new(step_cnt * STEP_X_THRESHOLD,
									 axeman_left,
									 70, 60).ent)
   else
      ship = canvas:new_img(150, 150, modPath .. "/DurrrSpaceShip.png")
      local shipSize = Pos.new(40, 40)
      ship:force_size(shipSize)
   end
   ent.ship = ship:cent()
   ent.asteroides = {}
   local ast = push_enemy(canvas, 350, 50, 0, 0)
   if version == ASTEROID_SHOOTER then
      ast.ent.life = -1
   end

   ent.lasers = {}
   ent.score = 0
   ent.score_canvas =
      canvas:new_text(10, 10, Entity.new_string("score: 0")):cent()
      ent.move = {}
   ent.move.up_down = 0
   ent.move.left_right = 0

   ent.explosion = {}

   next_level = level_gap

   enemy_apparition = 100

   shojou_attack = 0
   shojou_lvl = 0

   ent["turn-length"] = TURN_LENGTH
   old_tl = ywGetTurnLengthOverwrite();
   ywSetTurnLengthOverwrite(TURN_LENGTH);
   loading_bar = Entity.wrapp(ygGet("loading-bar"))

   local ret = canvas:new_wid()
   if version == AXEMAN_SHOOTER then
      local lb = loading_bar.create(canvas.ent, 10, canvas.ent['wid-pix'].h - 50);
      local bar_size = Size.new(200, 30)
      ent.lb = lb
      lb = CanvasObj.wrapp(lb)
      lb:force_size(bar_size)
   end
   return ret
end

function mod_init(entity)
   local e = Entity.wrapp(entity)

   upKeys = Event.CreateGrp(Y_UP_KEY, Y_W_KEY)
   downKeys = Event.CreateGrp(Y_DOWN_KEY, Y_S_KEY)
   leftKeys = Event.CreateGrp(Y_LEFT_KEY, Y_A_KEY)
   rightKeys = Event.CreateGrp(Y_RIGHT_KEY, Y_D_KEY)
   modPath = e["$path"]:to_string()
   e["pre-load"] = {}
   e["pre-load"][0] = {}
   e["pre-load"][0]["path"] = "YIRL_MODULES_PATH/loading_bar/"
   e["pre-load"][0]["type"] = "module"
   e.increase_atk_speed = Entity.new_func(increase_atk_speed)
   e.attack_nb_1 = Entity.new_func(attack_nb_1)
   Widget.new_subtype("asteroide-shooter", "createAstShoot")
   return Y_TRUE
end
