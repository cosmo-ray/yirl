
function action(entity, eve, arg)
   local eve = Event.wrapp(eve)
   local canvas = Canvas.wrapp(entity)
   local move = canvas.ent.move

   while eve:is_end() == false do
      if eve:type() == YKEY_DOWN then
	 if eve:key() == Y_ESC_KEY then
	    yFinishGame()
	    return YEVE_ACTION
	 elseif eve:key() == Y_UP_KEY then move.up_down = -1
	 elseif eve:key() == Y_DOWN_KEY then move.up_down = 1
	 elseif eve:key() == Y_LEFT_KEY then move.left_right = -1
	 elseif eve:key() == Y_RIGHT_KEY then move.left_right = 1
	 end
      elseif eve:type() == YKEY_UP then
	 if eve:key() == Y_UP_KEY or eve:key() == Y_DOWN_KEY then move.up_down = 0
	 elseif eve:key() == Y_LEFT_KEY or eve:key() == Y_RIGHT_KEY then
	 move.left_right = 0
	 end
      end
      eve = eve:next()
   end

   local pos = Pos.new(move.left_right * 10, move.up_down * 10)
   local ship = CanvasObj.wrapp(canvas.ent.ship)
   ship:move(pos)
   return YEVE_ACTION
end

function createAstShoot(entity)
   local canvas = Canvas.wrapp(entity)
   local ent = canvas.ent

   Entity.new_func("action", ent, "action")
   canvas.ent.background = "rgba: 255 255 255 255"
   ent.ship = canvas:new_img(150, 150, "./DurrrSpaceShip.png"):cent()
   ent.move = {}
   ent.move.up_down = 0
   ent.move.left_right = 0
   return canvas:new_wid()
end

function initAsteroideShooter(entity)
   local e = Entity.wrapp(entity)
   Widget.new_subtype("asteroide-shooter", "createAstShoot")
   Entity.new_int(0, e, "score");
end
