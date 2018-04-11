#include <Windows.h>
#include "fmath.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include "utils.hpp"
#include "Entities.hpp"
#include "lua.hpp"
#include "Net.hpp"
#include "Engine.hpp"
#include "Esp.hpp"
#include <fstream>

using color_t = uint8_t[4];
void(__cdecl* datapaths)(void) = nullptr;

void init(HINSTANCE dll)
{
	void(__cdecl* print)(const color_t&, const char*, ...) = nullptr;
	auto tier0 = GetModuleHandleW(L"tier0.dll");
	print = reinterpret_cast<decltype(print)>(GetProcAddress(tier0, "?ConColorMsg@@YAXABVColor@@PBDZZ"));

	auto handle = GetModuleHandleW(L"engine.dll");
	auto client = GetModuleHandleW(L"client.dll");
	
	globals::engine = interface::get<Engine>(handle, "VEngineClient");
	globals::entities = interface::get<Entities>(client, "VClientEntityList");

	// Credits to Copypaste
	using datapack_paths_t = void(__cdecl*)();
	datapaths = reinterpret_cast<datapack_paths_t>(signature::search(client, signature::detail::convert("55 8B EC 8B 0D ?? ?? ?? ?? 83 EC 7C")));
	// datapaths();

	
	globals::lua = interface::get<lua::Shared>(GetModuleHandleW(L"lua_shared.dll"), "LUASHARED");

	auto chl = interface::get<void>(client, "VClient");
	auto classes = method<get_all_classes_t>(8, chl)(chl);
	
	for (; classes; classes = classes->next)
		netvars::store(classes->table->table, classes->table);

	// Credits to Aixxe for the pattern and the render prototype
	auto overlay = GetModuleHandleW(L"gameoverlayrenderer.dll");

	void*& present = **reinterpret_cast<void***>(
		signature::search(overlay, signature::detail::convert("FF 15 ?? ?? ?? ?? 8B F8 85 DB 74 1F")) + 2
	);

	void*& reset   = **reinterpret_cast<void***>(
		signature::search(overlay, signature::detail::convert("FF 15 ?? ?? ?? ?? 8B F8 85 FF 78 18")) + 2
	);

	original::present = *reinterpret_cast<decltype(&original::present)>(&present);
	original::reset   = *reinterpret_cast<decltype(&original::reset)>(&reset);

	present = reinterpret_cast<void*>(&detours::present);
	reset   = reinterpret_cast<void*>(&detours::reset);

	for (; !(GetAsyncKeyState(VK_HOME) & 1); std::this_thread::sleep_for(std::chrono::milliseconds(25)));

	SetWindowLongPtrW(FindWindowW(L"Valve001", nullptr), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original::proc));

	present = *reinterpret_cast<void**>(&original::present);
	reset   = *reinterpret_cast<void**>(&original::reset);

	ImGui_ImplDX9_Shutdown();

	return FreeLibraryAndExitThread(dll, 0);
}

int __stdcall DllMain(HINSTANCE dll, DWORD reason, LPVOID)
{
	DisableThreadLibraryCalls(dll);
	if (reason == DLL_PROCESS_ATTACH)
	{
		std::thread(init, dll).detach();
	}

	return 1;
}
