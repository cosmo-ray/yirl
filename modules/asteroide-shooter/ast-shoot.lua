local modPath = Entity.wrapp(ygGet("asteroide-shooter.$path")):to_string()

function action(entity, eve, arg)
   local eve = Event.wrapp(eve)
   local canvas = Canvas.wrapp(entity)
   local move = canvas.ent.move
   local ship = CanvasObj.wrapp(canvas.ent.ship)

   while eve:is_end() == false do
      if eve:type() == YKEY_DOWN then
	 if eve:key() == Y_ESC_KEY then
	    if canvas.ent.quit then
	       canvas.ent.quit(canvas.ent)
	       return YEVE_ACTION
	    end
	    yFinishGame()
	    return YEVE_ACTION
	 elseif eve:is_key_up() then move.up_down = -1
	 elseif eve:is_key_down() then move.up_down = 1
	 elseif eve:is_key_left() then move.left_right = -1
	 elseif eve:is_key_right() then move.left_right = 1
	 end
      elseif eve:type() == YKEY_UP then
	 if eve:is_key_up() or eve:is_key_down() then move.up_down = 0
	 elseif eve:is_key_left() or eve:is_key_right() then
	 move.left_right = 0
	 end
      elseif eve:type() == YKEY_MOUSEMOTION then
	 -- I should add an api for that :p
	 ship:point_top_to(eve:mouse_pos())
      elseif eve:type() == YKEY_MOUSEDOWN then
	 local laser = canvas:new_obj(ship:pos():x() + ship:size():x() / 2 - 5,
				      ship:pos():y() + ship:size():y() / 2  -5, 0)

	 laser:point_right_to(eve:mouse_pos())
	 yeCreateFloat(laser:angle() - 90, laser.ent:cent(), "angle")
	 canvas.ent.lasers:push_back(laser:cent())
      end
      eve = eve:next()
   end

   local lasers = canvas.ent.lasers
   local asteroides = canvas.ent.asteroides
   for i = 0, lasers:len() do
      if lasers[i] then
	 local laser = CanvasObj.wrapp(lasers[i])

	 laser:advance(25, laser.ent.angle:to_float())
	 if (canvas:is_out(laser) == 1) then
	    print("kboum", canvas:is_out(laser))
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
							  canvas.ent.score:to_int())):cent()
		  print("life:", asteroides[i].life)
		  if asteroides[i].life:to_int() == 0 then
		     removeObj(canvas, asteroides,
			       CanvasObj.wrapp(asteroides[i]))
		  else
		     print(asteroides[i].life)
		     print(asteroides[i].speed)
		     print(asteroides[i].angle:to_float(),
			   laser.ent.angle:to_float())
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
		     print("l: ", asteroides:len())
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
   resource["img"] = modPath .. "jswars_gfx/shot.png"
   ent.resources[1] = {}
   resource = ent.resources[1]
   resource["img"] = modPath .. "jswars_gfx/asteroid.png"

   Entity.new_func("action", ent, "action")
   canvas.ent.background = "rgba: 255 255 255 255"
   local ship = canvas:new_img(150, 150, modPath .. "/DurrrSpaceShip.png")
   local shipSize = Pos.new(40, 40)
   ship:force_size(shipSize)
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
      canvas:new_text(10, 10, Entity.new_string("score: "..
						ent.score:to_int())):cent()
      ent.move = {}
   ent.move.up_down = 0
   ent.move.left_right = 0
   ent["turn-length"] = 100000
   return canvas:new_wid()
end

function initAsteroideShooter(entity)
   local e = Entity.wrapp(entity)
   Widget.new_subtype("asteroide-shooter", "createAstShoot")
end
