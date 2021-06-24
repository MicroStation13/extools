-- Module def
local cothreads = {}

-- Internal
local thds = {}
local coros = {}

local function cothreads_init()

end

local function cothd_reschedule(coro, thd)
	if (coro.thread) then
		for i=1, #coro.thread.coros do
			if (coro.thread.coros[i] == coro) then
				table.remove(coro.thread.coros, i)
				goto continue
			end
		end
		console.warning("Living coroutine with no thread found.")
		::continue::
	else
		for i=1, #coros do
			if (coros[i] == coro) then
				table.remove(coros, i)
				goto continue
			end
		end
		console.warning("Unscheduled coroutine forced into scheduling.")
		::continue::
	end
	coro.thread = thd
	thd.coros[#thd.coros+1] = coro
end

local function scheduler_tick(sched_id, max_dt)
	local end_time = os.clock()+max_dt
	local crun = {}
	local assigned_coros = thds[sched_id].coros
	for i=1, #assigned_coros do
		local c = assigned_coros[i]
		if (c.wake >= os.clock()) then
			xpcall(function()
				local res = cothreads.resume(c)
				if cothreads.status(c) == "dead" then
					cothreads.deschedule(c)
				else
					c.wake = os.clock()+(res or 0)
				end
			end, function(err)
				if (c.err_handler) then
					console.warning("Error in cothread: "..err.."\n"..debug.traceback())
					local ok, err = pcall(c.err_handler(err))
					if not ok then
						console.error("Error in error handler (seriously?): "..err..". Descheduling coroutine.")
						cothreads.deschedule(c)
					end
				end
			end)
		end
		local panic = thds[sched_id].panic
		if (os.clock() > end_time) then
			if (panic > 5) then
				console.fatal("Thread "..sched_id.." is overloaded! Server is overloaded!")
			elseif (panic > 3) then
				console.error("Thread "..sched_id.." is overloaded!")
			elseif (panic > 1) then
				console.warning("Thread "..sched_id .. " is overloaded.")
			end
			thd[sched_id].panic = panic+1
			return
		end
	end
	thd[sched_id].panic = thd[sched_id].panic - 1
	if (thd[sched_id].panic < 0) then
		thd[sched_id].panic = 0
	end
	-- wow, we have time left, find a new coro
	for i=1, #thds do
		-- steal one of their coros
		if thds[i].panic > 0 then
			cothd_reschedule(thds[i].coros[#thds[i].coros], thd[sched_id])
			return
		end
	end
	-- no coros found? let's check the waiting list
	if (#coros > 0) then
		cothd_reschedule(coros[1], thd[sched_id])
	end
end

local max_dt = math.huge

local function cothread_thread(id)
	while true do
		if (#thd[id].coros == 0) then

		end
		scheduler_tick(id, max_dt)
	end
end

-- Public API
function cothreads.create(f)

end

return cothreads