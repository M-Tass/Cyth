#pragma once
#define interface struct
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef interface
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include "utils.hpp"
#include "Ruda-Bold.hpp"
#pragma comment(lib, "user32.lib")

// Our drawing function, called in Present
extern inline void render();

namespace globals
{
	// Controls the visibility of the menu
	inline bool menu = true;
}

namespace original
{
	WNDPROC proc = nullptr;
	decltype(&IDirect3DDevice9::Reset)	 reset = nullptr;
	decltype(&IDirect3DDevice9::Present) present = nullptr;
}

namespace detours
{
	LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		ImGui_ImplDX9_WndProcHandler(hwnd, msg, wParam, lParam);
		return CallWindowProcW(original::proc, hwnd, msg, wParam, lParam);
	}

	HRESULT __stdcall present(IDirect3DDevice9* self, const RECT* src, const RECT* dst, HWND hwnd, const RGNDATA* rgn)
	{
		if (static bool initialised = false; !initialised)
		{
			auto gmod = FindWindowW(L"Valve001", nullptr);
			
			original::proc = reinterpret_cast<WNDPROC>(
				SetWindowLongPtrW(gmod, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(proc)) // reinterpret_cast for president
			);
			
			ImGui_ImplDX9_Init(gmod, self);

			auto& io = ImGui::GetIO();
			ImFontConfig config;
			config.FontDataOwnedByAtlas = false;
			
			io.FontDefault = io.Fonts->AddFontFromMemoryTTF(ruda, std::size(ruda), 14.f, &config);
			io.IniFilename = nullptr;
			
			initialised = true;
		}
		else
		{
			ImGui_ImplDX9_NewFrame();
			render();
			ImGui::Render();
		}

		return (self->*original::present)(src, dst, hwnd, rgn);
	}

	HRESULT __stdcall reset(IDirect3DDevice9* self, D3DPRESENT_PARAMETERS* params)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		ImGui_ImplDX9_CreateDeviceObjects();

		return (self->*original::reset)(params);
	}
}