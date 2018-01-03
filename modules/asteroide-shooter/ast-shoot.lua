
function action(entity, eve, arg)
   local eve = Event.wrapp(eve)
   local canvas = Canvas.wrapp(entity)
   local move = canvas.ent.move

   while eve:is_end() == false do
      if eve:type() == YKEY_DOWN then
	 if eve:key() == Y_ESC_KEY then
	    yFinishGame()
	    return YEVE_ACTION
	 elseif eve:type() == Y_UP_KEY then move.up_down = -1
	 elseif eve:type() == Y_DOWN_KEY then move.up_down = 1
	 elseif eve:type() == Y_LEFT_KEY then move.left_right = -1
	 elseif eve:type() == Y_RIGHT_KEY then move.left_right = 1
	 end
      end
      eve = eve:next()
   end

   print(canvas.ent.ship)
   return YEVE_NOTHANDLE
end

function createAstShoot(entity)
   local canvas = Canvas.wrapp(entity)
   local ent = canvas.ent

   Entity.new_func("action", canvas.ent, "action")
   canvas.ent.background = "rgba: 255 255 255 255"
   ent.ship = canvas:new_img(50, 50, "./DurrrSpaceShip.png"):cent()
   ent.move = {};
   ent.move.up_down = 0;
   ent.move.left_right = 0; 
   return canvas:new_wid()
end

function initAsteroideShooter(entity)
   local e = Entity.wrapp(entity)

   --Entity.new_func("snakeAction", e)
   --Entity.new_func("snakeDie", e)
   --Entity.new_func("snakeWarp", e)

   Widget.new_subtype("asteroide-shooter", "createAstShoot")
   Entity.new_int(0, e, "score");
end
