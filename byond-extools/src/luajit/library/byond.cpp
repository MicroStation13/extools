#include "../luajit.h"

int byond_get_turf(lua_State * L) {
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	int z = luaL_checkint(L, 3);
	ExtoolsLuajit::ByondToLua(L, Core::get_turf(x, y, z));
	return 1;
}

int byond_alert(lua_State * L) {
	Core::Alert(luaL_checkstring(L, 1));
	return 0;
}

int byond_runtime(lua_State * L) {
	Runtime(luaL_checkstring(L, 1));
	return 0; // we'll never reach here
}

int byond_ccall(lua_State * L) {
	if (lua_pcall(L, 0, 0, 0)) {
		return 1;
	}
	return 0;
}

luaL_Reg byond_lj[] = {
	{"get_turf", byond_get_turf},
	{"alert", byond_alert},
	{"runtime", byond_runtime},
	{"ccall", byond_ccall},
	{NULL, NULL}
};

int byond_global_index(lua_State * L) {
	Value glob = Value::Global();
	Value val = glob.get(luaL_checkstring(L, 2));
	ExtoolsLuajit::ByondToLua(L, val);
	return 1;
}

int byond_global_newindex(lua_State * L) {
	Value glob = Value::Global();
	glob.set(luaL_checkstring(L, 2), ExtoolsLuajit::LuaToByond(L, 3));
	return 0;
}

void ExtoolsLuajit::LoadByondLibrary(lua_State * L) {
	lp.Log("ExtoolsLuajit: Loading core BYOND library...");
	int top = lua_gettop(L);
	lua_newtable(L);
	int i = 0;
	while (byond_lj[i].name != NULL) {
		lua_pushstring(L, byond_lj[i].name);
		lua_pushcfunction(L, byond_lj[i].func);
		lua_settable(L, -3);
		i++;
	}
	LoadDMLibrary(L);
	lua_pushstring(L, "global");
	lua_newtable(L);
	lua_newtable(L);
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, byond_global_index);
	lua_settable(L, -3);
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, byond_global_newindex);
	lua_settable(L, -3);
	lua_setmetatable(L, -2);
	lua_settable(L, -3);
	lua_setglobal(L, "byond");
	lua_settop(L, top);
}