Widget = {}

Canvas = {}

Event = {}

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

function Canvas.new_wid(ent)
   return ywidNewWidget(ent:cent(), "canvas")
end

function Widget.new_subtype(name, functionName)
   local init = Entity.new_array()

   Entity.new_string(name, init, "name")
   Entity.new_func(functionName, init, "callback")
   yeIncrRef(init:cent())
   ywidAddSubType(init:cent())
end