#pragma once

#include <lua.hpp>
#include <csetjmp>
#include <list>
#include "../core/core.h"
#include "../core/proc_management.h"
#include "../third_party/lc53/compat-5.3.h"

struct LuaJIT_Boxed_Value {
	DataType type;
	int value;
	int id;
}; // This is required since we need to be able to have pointers to Values that have __gc run on them

class ExtoolsLuajit {
public:
	//ExtoolsLuajit();
	//ExtoolsLuajit(lua_State * state);
	//~ExtoolsLuajit();
	static lua_State * L;
	static jmp_buf jmp;
	static bool jmp_set;
	static std::unordered_map<lua_State *, std::list<Value>> ref;
	static Logging::LogProvider lp;
	static void LoadLibraries(lua_State * Lv);
	static unsigned int opcode;
	//static unsigned int opcode_load;
	static void Inject(std::string proc, unsigned int id);
	static int GetID(lua_State * L);
	static lua_State * Init();
	static void ByondToLua(lua_State * Lv, Value val);
	static trvh LuaToByond(lua_State * Lv, int stackpos);
	static void LoadScript(lua_State * Lv, std::string path);
	static trvh LuaCall(lua_State * Lv, unsigned int id, unsigned int args_len, Value* args, Value src);
	static void LoadByondLibrary(lua_State * Lv);
	static void PushValue(lua_State * Lv, Value value, bool track_type=true);
	//static void ConvertList(lua_State * Lv, RawList *list);
	static int CallProc(lua_State * Lv);
	static void LoadDMLibrary(lua_State * Lv);
	static void LoadConsoleLibrary(lua_State * Lv);
	static void RegisterFunction(lua_State * Lv, int id, int stackpos);
	static int SetJmp();
	static void Jmp();
	static void HookDM();
	static void stack_dump(lua_State * L);
	static void track_value(lua_State * L, Value value, struct LuaJIT_Boxed_Value * bv);
	static void untrack_value(lua_State * L, struct LuaJIT_Boxed_Value * bv);
	static void PushList(lua_State * L, Value list);
};

extern "C" EXPORT void enable_luajit();
