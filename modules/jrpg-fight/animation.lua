function initAnimationObj(main, anim, anime_func, field)
   main = Entity.wrapp(main)
   field = Entity.wrapp(field)
   anim = Entity.wrapp(anim)

   main[field:to_string()] = anim
   anim.animation_frame = 0
   anim.func = anime_func
   doAnimation(main, field)
   return anim
end

function startAnimation(main, anime, field)
   main = Entity.wrapp(main)
   field = Entity.wrapp(field)

   main[field] = {}
   local cur_anim = main[field:to_string()]
   cur_anim.animation_frame = 0
   cur_anim.func = anime
   doAnimation(main, field)
   return cur_anim
end

function doAnimation(main, field, eve)
   main = Entity.wrapp(main)
   field = Entity.wrapp(field)
   local cur_anim = main[field:to_string()]

   if cur_anim then
      if cur_anim.func(main:cent(), cur_anim:cent(), eve) == Y_TRUE then
	 cur_anim.animation_frame = cur_anim.animation_frame + 1
      end
      return Y_TRUE
   end
   return Y_FALSE
end

function endAnimation(main, field)
   main = Entity.wrapp(main)
   field = Entity.wrapp(field)
   main[field:to_string()] = nil
end

function initAnimation(ent)
   print("init animeation !")
   ygRegistreFunc(4, "initAnimationObj", "yInitAnimation")
   ygRegistreFunc(3, "startAnimation", "yStartAnimation")
   ygRegistreFunc(3, "doAnimation", "yDoAnimation")
   ygRegistreFunc(2, "endAnimation", "yEndAnimation")
end

