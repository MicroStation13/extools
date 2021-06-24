local function try_load(lib)
	local ok, lt = pcall(require, lib)
	if not ok then
		console.error("failed to load "..lib..": "..lt)
	else
		return lt
	end
end

lanes = require("lanes").configure({
	demote_full_userdata = true
}) -- if this fails to load, then shit's fucked yo
try_load("lc53_native")
cothreads = try_load("cothreads")
try_load("async")
try_load("threaded")
event = try_load("event")
conveyor = try_load("conveyor")