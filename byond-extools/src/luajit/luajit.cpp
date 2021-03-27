#include "luajit.h"
#include "../dmdism/disassembly.h"


// because otherwise the dynamic linker throws a fit
lua_State * ExtoolsLuajit::L = NULL;
unsigned int ExtoolsLuajit::opcode = 0;
jmp_buf ExtoolsLuajit::jmp = {};
bool ExtoolsLuajit::jmp_set = false;
Logging::LogProvider ExtoolsLuajit::lp = Logging::LogProvider("LuaJIT");
std::unordered_map<lua_State *, std::list<Value>> ExtoolsLuajit::ref;

char extools_luajit_key = 'f';

void ExtoolsLuajit::stack_dump(lua_State * L) {
	size_t top = lua_gettop(L);
	std::string stack = "STACK DUMP: ("+std::to_string(top)+")\n";
	//Core::Alert(stack);
	if (SetJmp()) {
		stack += "<Error>";
		Core::Alert(stack);
		return;
	}
	for (size_t i=top; i > 0; --i) {
		//Core::Alert(std::to_string(i) + ": " + luaL_typename(L, i) + lua_tostring(L, i) + "\n");
		stack += std::to_string(i) + ": " + luaL_typename(L, i) + "\n";
		const void * ptr;
		/*if ((ptr = lua_topointer(L, i)) == NULL) {
			char ptr_hex[9];
			sprintf(ptr_hex, "%.8x", ptr);
			stack += "0x";
			stack += (ptr_hex);
		}
		else
			stack += lua_tostring(L, i);*/
		//Core::Alert(stack);
	}
	Core::Alert(stack);
}

void luajit_call(ExecutionContext * ctx) {
	unsigned int id = ctx->bytecode[ctx->current_opcode+1];
	//std::string msg = "ExtoolsLuajit: Attempt to call function with id ";
	//msg = msg + std::to_string(id) + "\nOpcode: " + std::to_string(ctx->current_opcode) + ", bytecode[0]: " + std::to_string(ctx->bytecode[0]) + ", bytecode[opcode]: " + std::to_string(ctx->bytecode[ctx->current_opcode]);
	//ctx->current_opcode++;
	//Core::Alert(msg.c_str());
	// do the funny
	if (ExtoolsLuajit::SetJmp()) {
		Core::stack_push(Value::Null());
		return;
	}
	Core::stack_push(ExtoolsLuajit::LuaCall(ExtoolsLuajit::L, id, ctx->constants->arg_count, ctx->constants->args, ctx->constants->src));
}

trvh luajit_call_hook(unsigned int arg_count, Value* args, Value src) {
	auto pid = Core::extended_profiling_insanely_hacky_check_if_its_a_new_call_or_resume; // pain
	//ExecutionContext * ctx = Core::get_context();
	Core::Proc &p = Core::get_proc(pid);
	unsigned int id = p.get_bytecode()[1];
	ExtoolsLuajit::lp.Debug("Executing ID "+std::to_string(id)+" ("+p.raw_path+")");
	// do the funny
	if (ExtoolsLuajit::SetJmp()) {
		return Value::Null();
	}
	Value rv = ExtoolsLuajit::LuaCall(ExtoolsLuajit::L, id, arg_count, args, src);
	ExtoolsLuajit::lp.Debug("pain");
	return rv;
}

void luajit_load(ExecutionContext * ctx) {
	//ExtoolsLuajit::LoadScript(ExtoolsLuajit::L, ctx->constants->args[0]);
}

/*
	PUSHI (ID)
	LUAJIT_CALL
	RET
	END
*/

void disass(Core::Proc &p, std::string &str) {
	Disassembly disass = p.disassemble();
	for (Instruction i : disass.instructions) {
		str += i.bytes_str() + " " + i.comment() + "\n";
	}
}

void ExtoolsLuajit::Inject(std::string proc, unsigned int id) {
	auto pr = Core::try_get_proc(proc);
	if (pr == nullptr) {
		//fprintf(stderr, "coudln't get proc for %s || id %u.\n", proc.c_str(), id);
		lp.Log("Couldn't get proc for "+proc+" ("+std::to_string(id)+")", LOGLEVEL_WARNING);
		luaL_error(L, "proc %s not found", proc.c_str());
		return;
	}
	int a = 0;
	int b = 0;
	//printf("ExtoolsLuajit: Setting bytecode of %s.\n", proc.c_str());
	//Core::Alert(proc);
	(*pr).set_bytecode({0, id, 0, 0 });
	(*pr).hook(luajit_call_hook);
	//std::string msg = "READBACK: Our function's bytecode is:\n";
	//disass(*pr, msg);
	//lp.Debug(msg);
	///Core::Alert(msg);
	//(*pr).set_bytecode({opcode, id, 0x12, 0});
}


int ExtoolsLuajit::SetJmp() {
	jmp_set = true;
	return setjmp(jmp);
}

void ExtoolsLuajit::Jmp() {
	if (!jmp_set) exit(1);
	jmp_set = false;
	longjmp(jmp, 1);
}

void luajit_stacktrace(lua_State* L)
{
	lua_Debug entry;
	int depth = 0; 

	while (lua_getstack(L, depth, &entry)) {
		int status = lua_getinfo(L, "Sln", &entry);
		//assert(status);
		std::string src = entry.short_src;
		std::string line = std::to_string(entry.currentline);
		std::string name = entry.name ? entry.name : "?";
		ExtoolsLuajit::lp.Warning(src+"("+line+"): "+name);
		depth++;
	}
}

/*void luajit_xpcall_handler(lua_State * L) {

}*/

int luajit_error(lua_State * L) {
	std::string s = "LVM PANIC: ";
	s+=lua_tostring(L, -1);
	Core::Alert(s);
	luajit_stacktrace(L);
	ExtoolsLuajit::Jmp();
	return 0;
}

void * realloc_debug(void * ud, void * ptr, size_t osize, size_t nsize) {
	if (nsize == 0) {
			free(ptr);
			return NULL;
	}
	void * nptr = realloc(ptr, nsize);
	fprintf(stderr, "Resize %p from %u to %u\n", ptr, osize, nsize);
	return nptr;
}


lua_State * ExtoolsLuajit::Init() {
	//freopen("debug.stdout.log", "w", stdout);
	//freopen("debug.stderr.log", "w", stderr);
	//lp.Log("ExtoolsLuajit: Registering opcodes...");
	//opcode = Core::register_opcode("LUAJIT_CALL", luajit_call);
	//lp.Log("ExtoolsLuajit: Got opcode: "+std::to_string(opcode)+", initializing Lua state");
	lp.Log("Initializing Lua state...");
	//GLOBAL_STATE = *this;
	//L = lua_newstate(realloc_debug, NULL);
	L = luaL_newstate();
	luaL_openlibs(L);
	lua_pushlightuserdata(L, (void*)&extools_luajit_key);
	lua_newtable(L);
	lua_newtable(L);
	//ExtoolsLuajit::stack_dump(L);
	/*lua_pushstring(L, "__mode");
	lua_pushstring(L, "v");
	lua_settable(L, -3);*/
	lua_setmetatable(L, -2);
	lua_settable(L, LUA_REGISTRYINDEX);
	if (ExtoolsLuajit::SetJmp()) {
		lp.Error("ExtoolsLuajit: Failed to load libraries...");
		return NULL;
	}

	//ExtoolsLuajit::stack_dump(L);
	LoadLibraries(L);
	//ExtoolsLuajit::stack_dump(L);
	lp.Log("ExtoolsLuajit: Lua init complete!");
	HookDM();
	//luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE|LUAJIT_MODE_OFF); // disable luajit because i dunno what's breaking
	lua_atpanic(L, luajit_error);

	// debug stuff
	auto glob = Value::World();
	List list = glob.get("vars");
	lp.Debug("Found "+std::to_string(list.list->length)+" world vars.");
	for (int i = 0; i < list.list->length; i++) {
		lp.Debug(list.at(i));
	}
	//

	return L;
}

void ExtoolsLuajit::LoadLibraries(lua_State * L) {
	LoadByondLibrary(L);
	LoadConsoleLibrary(L);
}

void ExtoolsLuajit::LoadScript(lua_State * L, std::string path) {
	//luaL_dofile(L, path.c_str());
	if (ExtoolsLuajit::SetJmp()) {
		return;
	}
	if (luaL_loadfile(L, path.c_str()) || lua_pcall(L, 0, 0, 0))
		Core::Alert(lua_tostring(L, -1));
	/*(luaL_loadfile(L, path.c_str()));
	(lua_call(L, 0, 0));*/
}

int ExtoolsLuajit::GetID(lua_State *L) {
	int top = lua_gettop(L);
	lua_pushlightuserdata(L, &extools_luajit_key);
	lua_gettable(L, LUA_REGISTRYINDEX);
	int len = lua_objlen(L, -1);
	lua_settop(L, top);
	return len+1;
}

void ExtoolsLuajit::RegisterFunction(lua_State * Lv, int id, int stackpos) {
	int top = lua_gettop(Lv);
	int func = stackpos;
	if (!lua_isfunction(L, func) || lua_iscfunction(L, func)) {
		std::string err = std::to_string(func) + " not a function (how?)\ntype: ";
		if (lua_iscfunction(L, func))
			err+="cfunction";
		else
			err+=luaL_typename(L, func);
		Core::Alert(err);
	}
	lua_pushlightuserdata(Lv, &extools_luajit_key);
	lua_gettable(Lv, LUA_REGISTRYINDEX);
	lua_pushinteger(Lv, id);
	//Core::Alert(luaL_typename(Lv, func));
	lua_pushvalue(Lv, func);
	lua_settable(Lv, -3);
	/*lua_pushvalue(Lv, func);
	printf("Lua pointer: %p\n", Lv);
	if (lua_pcall(Lv, 0, 0, 0)) {
		Core::Alert(lua_tostring(Lv, -1));
	}*/
	lua_settop(Lv, top);
}

trvh ExtoolsLuajit::LuaCall(lua_State * Lv, unsigned int id, unsigned int args_len, Value* args, Value src) {
	//Core::Alert("Calling ID "+std::to_string(id));
	lua_pushlightuserdata(Lv, &extools_luajit_key);
	lua_gettable(Lv, LUA_REGISTRYINDEX);
	lua_pushnumber(Lv, id);
	lua_gettable(Lv, -2);
	//ByondToLua(L, args_len, args, src);
	//lua_call(L, args_len, 1);
	//Core::Alert("precall "+std::to_string(id)+" "+luaL_typename(Lv, lua_gettop(L))+", stack top: "+std::to_string(lua_gettop(L)));
	//stack_dump(Lv);
	//std::vector<Value> vals;
	ByondToLua(Lv, src);
	for (size_t i = 0; i < args_len; i++) {
		//vals.push_back(args[i]);
		ByondToLua(Lv, args[i]);
	}
	if (SetJmp()) {
		Core::Alert("uh oh we had a fucky wucky");
		return Value::Null();
	}
	try {
		if (lua_pcall(Lv, args_len+1, 1, 0)) {
			Core::Alert(lua_tostring(Lv, -1));
			//luajit_stacktrace(L);
			return Value::Null();
		}
	} catch(const std::exception& e) {
		Core::Alert(std::string("C++ Exception: ")+e.what());
		//luajit_stacktrace(L);
		return Value::Null();
	}
	//Core::Alert("Called ID "+std::to_string(id));
	lp.Debug("Exited fuckfest");
	Value rv = LuaToByond(Lv, -1);
	lp.Debug("Got value");
	return rv;
}

extern "C" {
	void enable_luajit() {
		ExtoolsLuajit::Init();
	}
}
