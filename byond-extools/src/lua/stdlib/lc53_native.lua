local libext = (jit.os == "Windows") and "dll" or "so"
local function load_pkg(lib)
	local ok, lt = pcall(package.loadlib("lua/stdlib/compat_native.so", "luaopen_"..lib))
	if ok then
		package.loaded["compat53."..lib] = lt
	else
		console.warning("Failed to load "..lib)
	end
end

load_pkg("string")
load_pkg("utf8")
load_pkg("compat53_string")
load_pkg("table")
require("compat53")
for k, v in pairs(package.loaded["compat53.compat53_string"]) do
	string[k] = v
end