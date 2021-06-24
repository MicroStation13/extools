--local calls = {}

--[[cothreads.schedule(cothreads.create(function()
	while true do
		xpcall(function()
			if #calls == 0 then goto continue end
			local call = calls[1]
			table.remove(calls, 1)
			local res = table.pack(call.f())
			if (call.c) then call.c(table.unpack(res)) end
			::continue::
		end, function(err)
			console.warning("error in async call: "..err.."\n"..debug.traceback())
		end)
		cothread.yield()
	end
end))]]

function async(func, callback)
	cothreads.schedule(cothreads.create(function()
		xpcall(function() callback(func()) end, function(err) console.warning("error in async call: "..err.."\n"..debug.traceback()) end)
	end))
end