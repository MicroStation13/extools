-- does not actually configure things here. we load a config.toml file
config = {}
local toml = require("toml")

_CFG = {}

function config.load(path)
	local f = io.open(path, "r")
	if not f then
		error("File not found.")
	end
	local cdat = f:read("*a")
	f:close()
	local cfg, err = toml.parse(f, {strict=false})
	if not cfg and err then error("toml parsing error: "..err) end
	for k, v in pairs(cfg) do
		_CFG[k] = v
	end
end

-- or just index _CFG

function config.get(key)
	local root = _CFG
	for match in key:gmatch("([A-Za-z_][%w_]*)%.") do
		if not root[match] or type(root[match]) ~= "table" then error("unable to find "..key) end
		root = root[match]
	end
	local sk = key:match("%.([A-Za-z_][%w_]*)$") or key:match("[A-Za-z_][%w_]*")
	if not sk then error("unable to find "..key) end
end

function byond.dm.lua_config(key)
	return config.get(key)
end

config.load("whitesands.toml")