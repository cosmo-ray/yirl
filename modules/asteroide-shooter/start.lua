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
local axeman_up = 780
local axeman_down = 650

local TURN_LENGTH = 100000

function action(entity, eve)
   local canvas = Canvas.wrapp(entity)
   local move = canvas.ent.move
   local ship = CanvasObj.wrapp(canvas.ent.ship)

   if yevIsKeyDown(eve, Y_ESC_KEY) then
      if canvas.ent.quit then
	 canvas.ent.quit(canvas.ent)
	 return YEVE_ACTION
      end
      yFinishGame()
      return YEVE_ACTION
   elseif yevIsGrpDown(eve, upKeys) then move.up_down = -1
   elseif yevIsGrpDown(eve, downKeys) then move.up_down = 1
   elseif yevIsGrpDown(eve, leftKeys) then move.left_right = -1
   elseif yevIsGrpDown(eve, rightKeys) then move.left_right = 1
   end
   if yevIsGrpUp(eve, upKeys) or yevIsGrpUp(eve, downKeys) then
      move.up_down = 0
   elseif yevIsGrpUp(eve, leftKeys) or yevIsGrpUp(eve, rightKeys) then
      move.left_right = 0
   end
   local pos = yevMousePos(eve)
   if pos then
      -- I should add an api for that :p
      pos = Pos.wrapp(pos)
      if version ~= AXEMAN_SHOOTER then
	 ship:point_top_to(pos)
      else
	 canvas:remove(canvas.ent.ship)
	 local a = ywPosAngle(ship:pos().ent, pos.ent)
	 local aa = math.abs(a)
	 local r = Rect.new(0, 0, 70, 60).ent
	 local sp = ship:pos()

	 if (aa > 45 and aa < 135) then
	    if a > 0 then
	       ywRectSetY(r, axeman_up)
	    else
	       ywRectSetY(r, axeman_down)
	    end
	 else
	    if (aa > 90) then
	       ywRectSetY(r, axeman_right)
	    else
	       ywRectSetY(r, axeman_left)
	    end
	 end
	 canvas.ent.ship = canvas:new_img(sp:x(), sp:y(), modPath .. "/axeman.png", r).ent
	 ship = CanvasObj.wrapp(canvas.ent.ship)
      end
   end
   local ret, button = yevMouseDown(eve, button);
   if ret then
      local laser = canvas:new_obj(ship:pos():x() + ship:size():x() / 2 - fire_threshold,
				      ship:pos():y() + ship:size():y() / 2  - fire_threshold, 0)

      local angle = -90
      if version == ASTEROID_SHOOTER and button == 3 then
	 angle = 90
      end
      laser:point_right_to(pos)
      yeCreateFloat(laser:angle() + angle, laser.ent:cent(), "angle")
      canvas.ent.lasers:push_back(laser:cent())
   end

   local lasers = canvas.ent.lasers
   local asteroides = canvas.ent.asteroides
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
		  asteroides[i].life = asteroides[i].life:to_int() - 1
		  canvas.ent.score = canvas.ent.score + 1
		  canvas:remove(canvas.ent.score_canvas)
		  canvas.ent.score_canvas =
		     canvas:new_text(10, 10,
				     Entity.new_string("score: "..
						       math.floor(canvas.ent.score:to_int()))):cent()
		  if asteroides[i].life:to_int() == 0 then
		     removeObj(canvas, asteroides,
			       CanvasObj.wrapp(asteroides[i]))
		  else
		     if (asteroides[i].speed < 30) then
			asteroides[i].speed = asteroides[i].speed:to_int() + 1
		     end
		     asteroides[i].angle:set_float(laser.ent.angle:to_float())
		     local p = CanvasObj.wrapp(asteroides[i]):pos()
		     local bigAst = canvas:new_obj(p:x(), p:y(), 1)

		     --bigAst:force_size(Pos:new(20, 20))
		     if asteroides[i].life < 0 then
			bigAst.ent.life = canvas.ent.score:to_int() / 10 + 1
		     elseif asteroides[i].life > 1 then
			bigAst.ent.life = asteroides[i].life - 1
		     else
			bigAst.ent.life = 1
		     end
		     bigAst.ent.speed = asteroides[i].speed
		     yeCreateFloat(-asteroides[i].angle:to_float(),
				   bigAst.ent, "angle")
 		     asteroides:push_back(bigAst:cent())
		  end
		  removeObj(canvas, lasers, laser)
		  break;
	       end
	    end
	 end
      end
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
   return YEVE_ACTION
end

function removeObj(wid, container, obj)
   wid:remove(obj)
   container:remove(obj.ent)
end

function createAstShoot(entity)
   local canvas = Canvas.wrapp(entity)
   local ent = canvas.ent

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
   else
      resource["img"] = modPath .. "jswars_gfx/shot.png"
      fire_threshold = 5
      up_threshold = 90
   end
   ent.resources[1] = {}
   resource = ent.resources[1]
   resource["img"] = modPath .. "jswars_gfx/asteroid.png"

   Entity.new_func("action", ent, "action")
   canvas.ent.background = "rgba: 255 255 255 255"
   local ship
   if version == AXEMAN_SHOOTER then
      ship = canvas:new_img(150, 150, modPath .. "/axeman.png", Rect.new(0, axeman_left,
									 70, 60).ent)
   else
      ship = canvas:new_img(150, 150, modPath .. "/DurrrSpaceShip.png")
      local shipSize = Pos.new(40, 40)
      ship:force_size(shipSize)
   end
   ent.ship = ship:cent()
   ent.asteroides = {}
   local bigAst = canvas:new_obj(350, 50, 1)
   bigAst.ent.life = -1
   bigAst.ent.speed = 0
   yeCreateFloat(0, bigAst.ent:cent(), "angle")
   ent.asteroides:push_back(bigAst:cent())

   ent.lasers = {}
   ent.score = 0
   ent.score_canvas =
      canvas:new_text(10, 10, Entity.new_string("score: 0")):cent()
      ent.move = {}
   ent.move.up_down = 0
   ent.move.left_right = 0
   ent["turn-length"] = TURN_LENGTH
   return canvas:new_wid()
end

function mod_init(entity)
   local e = Entity.wrapp(entity)
   upKeys = Event.CreateGrp(Y_UP_KEY, Y_W_KEY)
   downKeys = Event.CreateGrp(Y_DOWN_KEY, Y_S_KEY)
   leftKeys = Event.CreateGrp(Y_LEFT_KEY, Y_A_KEY)
   rightKeys = Event.CreateGrp(Y_RIGHT_KEY, Y_D_KEY)
   modPath = e["$path"]:to_string()
   Widget.new_subtype("asteroide-shooter", "createAstShoot")
   return Y_TRUE
end
