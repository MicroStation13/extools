#include "luajit.h"

trvh load_script(unsigned int args_len, Value* args, Value src) {
	ExtoolsLuajit::lp.Debug(args[0]);
	ExtoolsLuajit::LoadScript(ExtoolsLuajit::L, args[0]);
	return Value::Null();
}

void try_hook(std::string proc, ProcHook hook) {
	auto p_ptr = Core::try_get_proc(proc);
	if (p_ptr != nullptr) {
		(*p_ptr).hook(hook);
	}
}

void ExtoolsLuajit::HookDM() {
	lp.Log("ExtoolsLuajit: Installing hooks.");
	try_hook("/proc/__luajit_real_loadscript", load_script);
	lp.Log("ExtoolsLuajit: Hooks installed.");
}