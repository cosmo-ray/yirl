Widget = {}

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

function Event:next()
   return Event.wrapp(ywidNextEve(self:cent()))
end

function Event:is_end()
   local ret = ywidEveIsEnd(self:cent())
   return ret
end

function Event.wrapp(ent)
   local ret = { is_end=Event.is_end, cent=Event.cent , next=Event.next,
		 type = Event.type, key=Event.key, mouse_pos=Event.mouse_pos }

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
   return ret
end

function Canvas:new_img(x, y, path)
   local ret = ywCanvasNewImg(self.ent:cent(), x, y, path)
   return CanvasObj.wrapp(ret)
end

function Canvas:new_obj(x, y, objId)
   local ret = ywCanvasNewObj(self.ent:cent(), x, y, obj)
   return CanvasObj.wrapp(ret)
end

function Canvas:new_wid()
   local ret = ywidNewWidget(self.ent:cent(), "canvas")
   return ret
end

function Canvas.wrapp(ent)
   local ret = { ent=Entity.wrapp(ent) }
   ret.new_img=Canvas.new_img
   ret.new_obj=Canvas.new_obj
   ret.new_wid=Canvas.new_wid
   return ret
end

function Widget.new_subtype(name, functionName)
   local init = Entity.new_array()

   Entity.new_string(name, init, "name")
   Entity.new_func(functionName, init, "callback")
   yeIncrRef(init:cent())
   ywidAddSubType(init:cent())
end
