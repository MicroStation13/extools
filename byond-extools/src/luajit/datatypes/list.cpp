#include "../luajit.h"

/*void ExtoolsLuajit::ConvertList(lua_State * L, RawList *list) {
	lua_newtable(L);
	Value *vals = list->vector_part;
	for (size_t i=0; i<list->length; ++i) {
		lua_pushinteger(L, i+1);
		ByondToLua(L, vals[i]);
		lua_settable(L, -3);
	}
	if (list->is_assoc()) {
		auto asoc = list->map_part;
		while (asoc->right != nullptr) {
			// is this how i do it? probably not
			ByondToLua(L, asoc->key);
			ByondToLua(L, asoc->value);
			lua_settable(L, -3);
			asoc = asoc->right;
		}
	}
}*/

int list_index(lua_State * L) {
	struct LuaJIT_Boxed_Value * bv = (struct LuaJIT_Boxed_Value *) lua_touserdata(L, 1);
	Container list = Value(bv->type, bv->value);
	if (lua_type(L, 2) == LUA_TNUMBER) {
		float fval = lua_tonumber(L, 2);
		int ival = (int)fval;
		if (ival == fval) {
			ExtoolsLuajit::ByondToLua(L, list.at(ival-1));
			return 1;
		}
	}
	Value val = ExtoolsLuajit::LuaToByond(L, 2);
	ExtoolsLuajit::ByondToLua(L, list.at(val));
	return 1;
}

int list_newindex(lua_State * L) {
	struct LuaJIT_Boxed_Value * bv = (struct LuaJIT_Boxed_Value *) lua_touserdata(L, 1);
	Container list = Value(bv->type, bv->value);
	if (lua_type(L, 2) == LUA_TNUMBER) {
		float fval = lua_tonumber(L, 2);
		int ival = (int)fval;
		if (ival == fval) {
			//ExtoolsLuajit::ByondToLua(L, list.at(ival));
			ContainerProxy prox = list[ival];
			//list.set(ival, ExtoolsLuajit::LuaToByond(L, 3));
			prox = ExtoolsLuajit::LuaToByond(L, 3);
			return 0;
		}
	}
	Value val = ExtoolsLuajit::LuaToByond(L, 2);
	//ExtoolsLuajit::ByondToLua(L, list.at(val));
	ContainerProxy prox = list[val];
	prox = ExtoolsLuajit::LuaToByond(L, 3);
	return 0;
}

int list_gc(lua_State * L) {
	struct LuaJIT_Boxed_Value * bv = (struct LuaJIT_Boxed_Value *) lua_touserdata(L, 1);
	ExtoolsLuajit::untrack_value(L, bv);
	return 0;
}


// we have to do the jank
void ExtoolsLuajit::PushList(lua_State * L, Value value) {
	struct LuaJIT_Boxed_Value * bv = (struct LuaJIT_Boxed_Value *) lua_newuserdata(L, sizeof(LuaJIT_Boxed_Value)); 
	bv->id = 0; // for datums, because i'm just reusing a value i never previously used.
	track_value(L, value, bv);
	std::string typepath = "/list";
	lua_newtable(L);
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, list_gc);
	lua_settable(L, -3);
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, list_index);
	lua_settable(L, -3);
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, list_newindex);
	lua_settable(L, -3);
	lua_pushstring(L, "__typepath");
	lua_pushstring(L, typepath.c_str());
	lua_settable(L, -3);
	lua_pushstring(L, "__name");
	lua_pushstring(L, "datum");
	lua_settable(L, -3);
	lua_setmetatable(L, -2);
}