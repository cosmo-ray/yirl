
-- Hightscore Json format:
--[[
{
	"name0": 1000,
	"name1": 100,
	"name2": 10,
	"name3": 1,
}
]]

function hightScorePush(hg, name, score)
   score = yLovePtrToNumber(score)
   name = ylovePtrToString(name)
   hg = Entity.wrapp(hg)
   local i = 0

   yeRemoveChild(hg, name)
   while i < yeLen(hg) do
      local val = yeGetIntAt(hg, i)

      if score > val then
	 local toPush = Entity.new_int(score)
	 yeInsertAt(hg, toPush, i, name)
	 return
      end
      i = i + 1
   end
   yeCreateInt(score, hg, name);
end

function hightScoreString(hg, str)
   hg = Entity.wrapp(hg)
   str = Entity.wrapp(str)
   local ret = ""
   local i = 0

   while i < yeLen(hg) do
      ret = ret .. yeGetKeyAt(hg, i) .. ": " .. hg[i]:to_int() .. "\n"
      i = i + 1
   end
   yeSetString(str, ret)
end

function init(mod)
   ygRegistreFunc(3, "hightScorePush", "yHightScorePush")
   ygRegistreFunc(2, "hightScoreString", "yHightScoreString")
end
