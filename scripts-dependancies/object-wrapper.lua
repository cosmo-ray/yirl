Widget = {}

Container = {}

Canvas = {}

CanvasObj = {}

Event = {}

Pos = {}

function Event:cent()
   return self.ent:cent()
end

function Event:type()
   return ywidEveType(self:cent())
end

function Event:mouse_pos()
   return Pos.wrapp(ywidEveMousePos(self:cent()))
end

function Event:key()
   return ywidEveKey(self:cent())
end

function Event:is_key_up()
   return self:key() == Y_UP_KEY or self:key() == Y_W_KEY
end

function Event:is_key_down()
   return self:key() == Y_DOWN_KEY or self:key() == Y_S_KEY
end

function Event:is_key_left()
   return self:key() == Y_LEFT_KEY or self:key() == Y_A_KEY
end

function Event:is_key_right()
   return self:key() == Y_RIGHT_KEY or self:key() == Y_D_KEY
end

function Event:next()
   return Event.wrapp(ywidNextEve(self:cent()))
end

function Event:is_end()
   local ret = ywidEveIsEnd(self:cent())
   return ret
end

function Event.wrapp(ent)
   local ret = { is_end=Event.is_end, cent=Event.cent , next=Event.next,
		 type = Event.type, key=Event.key, mouse_pos=Event.mouse_pos,
		 is_key_up=Event.is_key_up, is_key_down=Event.is_key_down,
		 is_key_left=Event.is_key_left, is_key_right=Event.is_key_right }

   ret.ent = Entity.wrapp(ent)
   return ret;
end

function Pos:tostring()
   return ywPosToString(self.ent:cent())
end

function Pos:x()
   return ywPosX(self.ent:cent())
end

function Pos:y()
   return ywPosY(self.ent:cent())
end

function Pos._init_(ent)
   ent.tostring = Pos.tostring
   ent.x = Pos.x
   ent.y = Pos.y
   return ent
end

function Pos.wrapp(p)
   local ret = {ent = Entity.wrapp(p)}
   return Pos._init_(ret)
end

function Pos.new(x, y, father, name)
   local ent = ywPosCreate(x, y, father, name)
   local needDestroy = false

   if father == nil then
      needDestroy = true
   end
   ent = Entity._wrapp_(ent, needDestroy)
   local ret = {ent = ent}
   return Pos._init_(ret)
end

function CanvasObj:cent()
   return self.ent:cent()
end

function CanvasObj:move(pos)
   ywCanvasMoveObj(self:cent(), pos.ent:cent())
end

function CanvasObj:pos()
   return Pos.wrapp(ywCanvasObjPos(self:cent()))
end

function CanvasObj:colide_with(other)
   return ywCanvasObjectsCheckColisions(self:cent(), other:cent())
end

function CanvasObj:size()
   return Pos.wrapp(ywCanvasObjSize(nil, self:cent()))
end

function CanvasObj:angle()
   return ywCanvasObjAngle(self:cent())
end

function CanvasObj:advance(speed, direction)
   return ywCanvasAdvenceObj(self:cent(), speed, direction)
end

function CanvasObj:point_top_to(point)
   ywCanvasObjPointTopTo(self:cent(), point.ent:cent())
end

function CanvasObj:point_right_to(point)
   ywCanvasObjPointRightTo(self:cent(), point.ent:cent())
end

function CanvasObj:force_size(size)
   return ywCanvasForceSize(self:cent(), size.ent:cent())
end

-- Warning !, Attention ! Achtung !
-- if you are using a map, it's not the angles of the map
function CanvasObj:rotate(angle)
   return ywCanvasRotate(self:cent(), angle)
end

function CanvasObj.wrapp(ent)
   local ret = { ent=Entity.wrapp(ent) }
   ret.cent = CanvasObj.cent
   ret.move = CanvasObj.move
   ret.pos = CanvasObj.pos
   ret.size = CanvasObj.size
   ret.angle = CanvasObj.angle
   ret.advance = CanvasObj.advance
   ret.force_size = CanvasObj.force_size
   ret.rotate = CanvasObj.rotate
   ret.point_top_to = CanvasObj.point_top_to
   ret.point_right_to = CanvasObj.point_right_to
   ret.colide_with = CanvasObj.colide_with
   return ret
end

function Canvas:new_img(x, y, path)
   local ret = ywCanvasNewImg(self.ent:cent(), x, y, path)
   return CanvasObj.wrapp(ret)
end

function Canvas:new_obj(x, y, objId)
   local ret = ywCanvasNewObj(self.ent:cent(), x, y, objId)
   return CanvasObj.wrapp(ret)
end

function Canvas:new_wid()
   local ret = ywidNewWidget(self.ent:cent(), "canvas")
   return ret
end

function Canvas:remove(ent)
   local e = ent:cent()
   return ywCanvasRemoveObj(self.ent:cent(), e)
end

function Canvas:is_out(obj)
   return ywCanvasObjIsOut(self.ent:cent(), obj:cent())
end

function Canvas.init_entity(ent)
   local ret = Canvas.wrapp(ent)

   if ret.ent["<type>"] == nil then
      ret.ent["<type>"] = "canvas"
   end
   return ret
end

function Canvas.new_entity(father, name)
   local ret = Entity.new_array(father, name)

   ret = Canvas.init_entity(ret);
   return ret
end

function Canvas.wrapp(ent)
   local ret = { ent=Entity.wrapp(ent) }
   ret.new_img=Canvas.new_img
   ret.new_obj=Canvas.new_obj
   ret.new_wid=Canvas.new_wid
   ret.remove=Canvas.remove
   ret.is_out=Canvas.is_out
   return ret
end

function Container.init_entity(entity, cnt_type)
   local conntainer = Container.wrapp(entity)
   local ent = conntainer.ent

   ent.cnt_type = cnt_type
   ent.entries = {}
   if ent["<type>"] == nil then
      ent["<type>"] = "container"
   end
   return conntainer
end

function Container.new_entity(cnt_type, father, name)
   local ret = Entity.new_array(father, name)

   ret = Container.init_entity(ret, cnt_type);
   return ret
end

function Container:new_wid()
   local ret = ywidNewWidget(self.ent:cent(), "container")
   return ret
end

function Container.wrapp(ent)
   local ret = { ent=Entity.wrapp(ent), new_wid=Container.new_wid}
   return ret
end

function Widget.new_subtype(name, functionName)
   local init = Entity.new_array()

   Entity.new_string(name, init, "name")
   Entity.new_func(functionName, init, "callback")
   yeIncrRef(init:cent())
   ywidAddSubType(init:cent())
end
