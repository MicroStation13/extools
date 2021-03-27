#include "../luajit.h"

/*Value * get_var(RawDatum * d, std::string var) {
	//auto d = dat->
	short amt = d->len_vars;
	for (size_t i=0; i < amt; i++) {
		auto dvar = d->vars[i];
		std::string name = Core::GetStringFromId(dvar.id);
		if (name == var) {
			return &(dvar.value);
		}
	}
	return nullptr;
}

int datum_index(lua_State * L) {
	RawDatum * d = (RawDatum *) lua_touserdata(L, 1);
	std::string key = luaL_checkstring(L, 2);
	std::string type = *get_var(d, "type");
	Core::Proc * proc = Core::try_get_proc(type+"/"+key);
	if (proc != nullptr) {
		// C closures seems mildly cursed to me
		lua_pushvalue(L, 1);
		//lua_pushstring(L, (type+"/"+key).c_str());
		lua_pushlightuserdata(L, proc);
		lua_pushcclosure(L, ExtoolsLuajit::CallProc, 2);
		return 1;
	}

	Value * _v;
	if ((_v = get_var(d, key)) != nullptr) {
		ExtoolsLuajit::ByondToLua(L, *_v);
		return 1;
	}
	return 0;
}*/
int datum_func(lua_State * L) {
	//struct LuaJIT_Boxed_Value * bv = (struct LuaJIT_Boxed_Value *) lua_touserdata(L, lua_upvalueindex(1));
	struct LuaJIT_Boxed_Value * bv = (struct LuaJIT_Boxed_Value *) lua_touserdata(L, 1);
	std::string path = lua_tostring(L, lua_upvalueindex(1));
	int nargs = lua_gettop(L)-1;
	Value val = Value(bv->type, bv->value);
	std::vector<Value> args = std::vector<Value>(nargs);
	//ExtoolsLuajit::lp.Debug(luaL_typename(L, 2));
	for (size_t i=0; i<nargs; i++) {
		args[i] = ExtoolsLuajit::LuaToByond(L, i+2);
	}
	ExtoolsLuajit::ByondToLua(L, val.invoke(path, args));
	return 1;
}

int datum_index(lua_State * L) {
	struct LuaJIT_Boxed_Value * bv = (struct LuaJIT_Boxed_Value *) lua_touserdata(L, 1);
	std::string index = luaL_checkstring(L, 2);
	Value val = Value(bv->type, bv->value);
	ExtoolsLuajit::lp.Debug("Trying to get "+index+" "+std::to_string(bv->type)+":"+std::to_string(bv->value));
	ExtoolsLuajit::lp.Debug(Core::stringify(val.get("type")));

	auto vars = val.get_all_vars();
	/*if (vars.find(index) != vars.end()) {
		ExtoolsLuajit::lp.Log("found "+index);
	} else {
		ExtoolsLuajit::lp.Warning("didn't find "+index);
	}*/
	//if (Value var = val.get(index) != Value::Null()) {
	if (vars.find(index) != vars.end()) {
	//if (val.has_var(index)) {
		Value var = val.get(index);
		ExtoolsLuajit::ByondToLua(L, var);
		return 1;
	}
	//ExecutionContext * ctx = Core::get_context();
	//ExtoolsLuajit::lp.Error("CAUGHT EXCEPTION");
	//ExtoolsLuajit::lp.Error(std::to_string(ctx->paused));
	//ExtoolsLuajit::lp.Error(std::to_string(ctx->parent_context->paused));
	//lua_pushvalue(L, 1);
	lua_pushstring(L, index.c_str());
	lua_pushcclosure(L, datum_func, 1);
	return 1;
}

int datum_newindex(lua_State * L) {
	struct LuaJIT_Boxed_Value * bv = (struct LuaJIT_Boxed_Value *) lua_touserdata(L, 1);
	std::string index = luaL_checkstring(L, 2);
	Value v = Value(bv->type, bv->value);
	v.set(index, ExtoolsLuajit::LuaToByond(L, 3));
	return 1;
}

int datum_gc(lua_State * L) {
	struct LuaJIT_Boxed_Value * bv = (struct LuaJIT_Boxed_Value *) lua_touserdata(L, 1);
	ExtoolsLuajit::lp.Debug("GC on "+std::to_string(bv->type)+":"+std::to_string(bv->type));
	ExtoolsLuajit::untrack_value(L, bv);
	return 0;
}

void ExtoolsLuajit::PushValue(lua_State * L, Value value, bool track_type) {
	struct LuaJIT_Boxed_Value * bv = (struct LuaJIT_Boxed_Value *) lua_newuserdata(L, sizeof(LuaJIT_Boxed_Value));
	bv->id = 0; // for datums, because i'm just reusing a value i never previously used.
	track_value(L, value, bv);
	lua_newtable(L);
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, datum_gc);
	lua_settable(L, -3);
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, datum_index);
	lua_settable(L, -3);
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, datum_newindex);
	lua_settable(L, -3);
	lua_pushstring(L, "__name");
	lua_pushstring(L, "datum");
	lua_settable(L, -3);
	if (track_type) {
		std::string typepath = Core::stringify(value.get("type"));
		lua_pushstring(L, "__typepath");
		lua_pushstring(L, typepath.c_str());
		lua_settable(L, -3);
	}
	lua_setmetatable(L, -2);
}

//void ExtoolsLuajit::PullValue(lua_State * L, int index) {

//}