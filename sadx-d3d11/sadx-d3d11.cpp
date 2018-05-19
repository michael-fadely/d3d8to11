#include "stdafx.h"
#include "globals.h"
#include <IniFile.hpp>

static void __cdecl njDrawSprite2D_DrawNow_r(NJS_SPRITE *sp, Int n, Float pri, NJD_SPRITE attr);
static Trampoline njDrawSprite2D_DrawNow_t(0x0077E050, 0x0077E058, njDrawSprite2D_DrawNow_r);
static void __cdecl njDrawSprite2D_DrawNow_r(NJS_SPRITE *sp, Int n, Float pri, NJD_SPRITE attr)
{
	auto original = reinterpret_cast<decltype(njDrawSprite2D_DrawNow_r)*>(njDrawSprite2D_DrawNow_t.Target());

	if (!(attr & NJD_SPRITE_ALPHA))
	{
		DWORD value;
		Direct3D_Device->GetRenderState(D3DRS_ZWRITEENABLE, &value);
		Direct3D_Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		original(sp, n, pri, attr);
		Direct3D_Device->SetRenderState(D3DRS_ZWRITEENABLE, value);
	}
	else
	{
		original(sp, n, pri, attr);
	}
}

extern "C"
{
	EXPORT ModInfo SADXModInfo = { ModLoaderVer, nullptr, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0 };
	EXPORT void __cdecl Init(const char* path, HelperFunctions& helpers)
	{
		globals::helper_functions = helpers;

		auto d3d9 = GetModuleHandle(L"d3d9.dll");
		if (d3d9)
		{
			FreeLibrary(d3d9);
		}

		auto d3d8 = GetModuleHandle(L"d3d8.dll");
		if (d3d8)
		{
			FreeLibrary(d3d8);
		}

		std::string inipath(path);
		inipath.append("\\config.ini");

		IniFile ini(inipath);

		globals::max_fragments = ini.getInt("ppll", "maxFragCount", MAX_FRAGMENTS_DEFAULT);

		if (globals::max_fragments != MAX_FRAGMENTS_DEFAULT)
		{
			PrintDebug("Maximum fragments: %u\n", globals::max_fragments);
		}

		globals::allow_d32 = ini.getBool("misc", "allowDepth32", false);

		if (globals::allow_d32)
		{
			PrintDebug("Allowing 32-bit depth buffer.\n");
		}

		WriteJump(reinterpret_cast<void*>(0x007C235E), Direct3DCreate8);
	}

	EXPORT void __cdecl OnFrame()
	{
		if (ControllerPointers[0])
		{
			if (ControllerPointers[0]->PressedButtons & Buttons_D)
			{
				Direct3D_Device->free_shaders();
				Direct3D_Device->oit_load_shaders();
			}

			if (ControllerPointers[0]->PressedButtons & Buttons_C)
			{
				Direct3D_Device->oit_enabled = !Direct3D_Device->oit_enabled;
			}
		}

		auto color = DebugFontColor;
		auto size = DebugFontSize;
		DebugFontSize = 12.0f * std::min(HorizontalStretch, VerticalStretch);

		SetDebugFontColor(Direct3D_Device->oit_enabled ? 0xFF00FF00 : 0xFFFF0000);
		DisplayDebugStringFormatted(NJM_LOCATION(1, 1), "OIT: %s", Direct3D_Device->oit_enabled ? "ON" : "OFF");

		SetDebugFontColor(color.color);
		DebugFontSize = size;
	}
}
