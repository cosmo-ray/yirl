
function action(entity, eve, arg)
   local eve = Event.wrapp(eve)

   while eve:is_end() == false do
      if eve:type() == YKEY_DOWN and eve:key() == Y_Q_KEY then
	 yFinishGame()
	 return YEVE_ACTION
      end
      eve = eve:next()
   end
   return YEVE_NOTHANDLE
end

function createAstShoot(entity)
   local canvas = Entity.wrapp(entity)

   Entity.new_func("action", canvas, "action")
   return Canvas.new_wid(canvas)
end

function initAsteroideShooter(entity)
   local e = Entity.wrapp(entity)

   --Entity.new_func("snakeAction", e)
   --Entity.new_func("snakeDie", e)
   --Entity.new_func("snakeWarp", e)

   Widget.new_subtype("asteroide-shooter", "createAstShoot")
   Entity.new_int(0, e, "score");
end
