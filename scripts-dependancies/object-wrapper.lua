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
		 type = Event.type, key=Event.key }

   ret.ent = Entity.wrapp(ent)
   return ret;
end

function CanvasObj:cent()
   return self.ent:cent()
end

function Pos.new(x, y, father, name)
   local ent = ywPosCreate(x. y, father, name)
   local needDestroy = false

   if father == nil then
      needDestroy = true
   end
   ent = Entity._wrapp_(ent, needDestroy)
   local ret = {ent = ent}
end

function CanvasObj.wrapp(ent)
   local ret = { ent=Entity.wrapp(ent) }
   ret.cent = CanvasObj.cent
   return ret
end

function Canvas:new_img(x, y, path)
   local ret = ywCanvasNewImg(self.ent:cent(), x, y, path)
   return CanvasObj.wrapp(ret)
end

function Canvas:new_wid()
   local ret = ywidNewWidget(self.ent:cent(), "canvas")
   return ret
end

function Canvas.wrapp(ent)
   local ret = { ent=Entity.wrapp(ent) }
   ret.new_img=Canvas.new_img
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