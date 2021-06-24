local calls = {}

function byond.dm:luajit_pump_events(max_time)
	if (#calls == 0) then return 0 end
	max_time = max_time or math.huge
	local end_time = os.clock() + max_time
	repeat
		local call = calls[1]
		table.remove(calls, 1)
		local res = table.pack(call.f())
		call.c(table.unpack(res))
	until os.clock() > end_time or (#calls == 0)
	return #calls
end

function threaded(func, callback)
	calls[#calls+1] = {f=func, c=callback}
end