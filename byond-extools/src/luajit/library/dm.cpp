#include "../luajit.h"
// metatables time

void dm_push_table(lua_State * L, std::string path);

int dm_table_index(lua_State * L) {
	std::string path = lua_tostring(L, lua_upvalueindex(1));
	dm_push_table(L, path+"/"+luaL_checkstring(L, 2));
	return 1;
}

int dm_table_newindex(lua_State * L) {
	std::string path = lua_tostring(L, lua_upvalueindex(1));
	std::string fullpath = path + "/" + luaL_checkstring(L, 2);
	ExtoolsLuajit::lp.Debug("Attempting to push value for "+fullpath);
	if (lua_type(L, 3) == LUA_TFUNCTION) {
		auto id = ExtoolsLuajit::GetID(L);
		ExtoolsLuajit::lp.Debug(fullpath+" got id "+std::to_string(id));
		ExtoolsLuajit::RegisterFunction(L, id, 3);
		ExtoolsLuajit::Inject(fullpath, id);
		// finally, we're done
		//lua_pushvalue(L, 3);
		return 0;
	}
	luaL_typerror(L, 3, "function");
	return 0;
}

int dm_table_call(lua_State * L) {
	std::string path = lua_tostring(L, lua_upvalueindex(1));
	ExtoolsLuajit::lp.Debug("Trying to call proc "+path);
	Core::Proc * proc_p = Core::try_get_proc(path);
	if (proc_p == nullptr) {
		luaL_error(L, "proc %s not found", path.c_str());
	}
	int nargs = lua_gettop(L)-1;
	std::vector<Value> args = std::vector<Value>(nargs);
	//ExtoolsLuajit::lp.Debug(luaL_typename(L, 2));
	for (size_t i=0; i<nargs; i++) {
		args[i] = ExtoolsLuajit::LuaToByond(L, i+2);
	}
	ExtoolsLuajit::lp.Debug("Precall "+path);
	Value retval = (*proc_p).call(args);

	ExtoolsLuajit::lp.Debug("Postcall "+path);
	ExtoolsLuajit::ByondToLua(L, retval);
	return 1;
}

void dm_push_table(lua_State * L, std::string path) {
	lua_newtable(L);
	lua_newtable(L);
	lua_pushstring(L, "__index");
	lua_pushstring(L, path.c_str());
	lua_pushcclosure(L, dm_table_index, 1);
	lua_settable(L, -3);
	lua_pushstring(L, "__newindex");
	lua_pushstring(L, path.c_str());
	lua_pushcclosure(L, dm_table_newindex, 1);
	lua_settable(L, -3);
	lua_pushstring(L, "__call");
	lua_pushstring(L, path.c_str());
	lua_pushcclosure(L, dm_table_call, 1);
	lua_settable(L, -3);
	lua_setmetatable(L, -2);
}

void ExtoolsLuajit::LoadDMLibrary(lua_State * L) {
	lp.Log("ExtoolsLuajit: Loading DM interop library...");
	int top = lua_gettop(L);
	lua_pushstring(L, "dm");
	dm_push_table(L, "");
	//lua_setglobal(L, "dm");
	lua_settable(L, -3);
	lua_settop(L, top); // can't ever be too sure when you're as dumb as me
}