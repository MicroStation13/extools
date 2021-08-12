#include "../luajit.h"
// Never actually "serialized", just a closure

int ExtoolsLuajit::CallProc(lua_State * L) {
	Value * dat = (Value *) lua_touserdata(L, lua_upvalueindex(1));
	//std::string path = lua_tostring(L, lua_upvalueindex(2));
	std::string path = lua_tostring(L, lua_upvalueindex(2));
	std::vector<Value> vals;
	int n = lua_gettop(L);
	for (int i = 0; i <= n; i++) {
		//vals.push_back(LuaToByond(L, i));
	}
	// init values here
	//ByondToLua(L, proc->call(vals, dat));
	return 1;
}