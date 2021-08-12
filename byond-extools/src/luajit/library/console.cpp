#include "../luajit.h"

Logging::LogProvider logp = Logging::LogProvider("Lua VM");

int console_log(lua_State * L) {
	std::string msg = luaL_checkstring(L, 1);
	logp.Log(msg);
	return 0;
}

int console_debug(lua_State * L) {
	std::string msg = luaL_checkstring(L, 1);
	logp.Debug(msg);
	return 0;
}

int console_warning(lua_State * L) {
	std::string msg = luaL_checkstring(L, 1);
	logp.Warning(msg);
	return 0;
}

int console_error(lua_State * L) {
	std::string msg = luaL_checkstring(L, 1);
	logp.Error(msg);
	return 0;
}

int console_fatal(lua_State * L) {
	std::string msg = luaL_checkstring(L, 1);
	logp.Fatal(msg);
	return 0;
}

luaL_Reg console_lj[] = {
	{"log", console_log},
	{"debug", console_debug},
	{"warning", console_warning},
	{"error", console_error},
	{"fatal", console_fatal},
	{NULL, NULL}
};

void ExtoolsLuajit::LoadConsoleLibrary(lua_State * L) {
	lp.Log("ExtoolsLuajit: Loading console library...");
	int top = lua_gettop(L);
	lua_newtable(L);
	int i = 0;
	while (console_lj[i].name != NULL) {
		lua_pushstring(L, console_lj[i].name);
		lua_pushcfunction(L, console_lj[i].func);
		lua_settable(L, -3);
		i++;
	}
	//LoadDMLibrary(L);
	lua_setglobal(L, "console");
	lua_settop(L, top);
}