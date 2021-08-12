#include "luajit.h"

trvh ExtoolsLuajit::LuaToByond(lua_State * L, int pos) {
	//lp.Debug(luaL_typename(L, -1));
	if (pos < 0)
		pos = lua_gettop(L)+pos+1;
	lp.Debug("Sending a "+std::string(luaL_typename(L, pos))+ " to BYOND...");
	switch (lua_type(L, pos)) {
		case LUA_TNIL:
		case LUA_TNONE:
			return Value::Null();
		case LUA_TNUMBER:
			return Value(lua_tonumber(L, pos));
		case LUA_TSTRING:
			return Value(std::string(lua_tostring(L, pos)));
		case LUA_TBOOLEAN:
			return lua_toboolean(L, pos) ? Value::True() : Value::False();
		case LUA_TUSERDATA:
			{
				lua_getmetatable(L, -1);
				lua_pushstring(L, "__name");
				lua_gettable(L, -2);
				std::string name = lua_tostring(L, -1);
				lua_pop(L, 2);
				if (name == "datum") {
					struct LuaJIT_Boxed_Value * bv = ((struct LuaJIT_Boxed_Value *) lua_touserdata(L, -1));
					return Value(bv->type, bv->value);
				}
			}
		default:
			return Value("Incompatible Lua value");
	}
}

void ExtoolsLuajit::ByondToLua(lua_State * L, Value val) {
	switch(val.type) {
		// simple shit
		case DataType::STRING:
			lua_pushstring(L, (static_cast<std::string>(val)).c_str()); // this should be right?
			break;
		case DataType::NUMBER:
			lua_pushnumber(L, (static_cast<float>(val))); // i hope this is right
			break;
		case DataType::NULL_D:
			lua_pushnil(L);
			break;

		// all the lists can go to brazil

		// goddamn type paths
		case DataType::MOB_TYPEPATH:
		case DataType::OBJ_TYPEPATH:
		case DataType::TURF_TYPEPATH:
		case DataType::AREA_TYPEPATH:
		case DataType::DATUM_TYPEPATH:
		case DataType::SAVEFILE_TYPEPATH:
		case DataType::LIST_TYPEPATH:
		case DataType::CLIENT_TYPEPATH:
		case DataType::IMAGE_TYPEPATH:
			lua_pushstring(L, Core::stringify(val).c_str());
			break;

		// datum and shit
		case DataType::OBJ:
		case DataType::TURF:
		//case DataType::MOB:
		//case DataType::AREA:
		case DataType::DATUM:
			ExtoolsLuajit::PushValue(L, val);
			break;
		default: //brazil
			{
				if ((val.type >= 0x0F && val.type <= 0x1c) ||
					(val.type >= 0x2c && val.type <= 0x39) ||
					(val.type >= 0x40 && val.type <= 0x52) ||
					val.type == 0x54) {
					ExtoolsLuajit::PushList(L, val);
					return;
				}
				//lua_pushlightuserdata(L, &(val));
				ExtoolsLuajit::PushValue(L, val, false); // ya know what, fuck around and find out. i don't promise things not breaking.
			}
	}
}

void ExtoolsLuajit::track_value(lua_State * L, Value value, struct LuaJIT_Boxed_Value * bv) {
	bv->value = value.value;
	bv->type = value.type;
	IncRefCount(value.type, value.value);
}

void ExtoolsLuajit::untrack_value(lua_State * L, struct LuaJIT_Boxed_Value * bv) {
	/*if (bv->id == -1) return;
	for (auto it=ref[L].begin(); it != ref[L].end(); ++it) {
		if (&(*it) == &(*bv->value)) {
			DecRefCount((*bv->value).type, (*bv->value).value);
			bv->id = -1;
			ref[L].erase(it);
		}
	}*/
	DecRefCount(bv->type, bv->value);
}